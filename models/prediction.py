#! /usr/bin/python
import sys


import json
import importlib
import pkgutil
import os

import models
from models.evaluate_model import evaluate
import models.utils as utils

def import_models(base_pkg):
    base_path = os.path.dirname(base_pkg.__file__)
    modules = {}
    
    for finder, name, ispkg in pkgutil.walk_packages(base_pkg.__path__, base_pkg.__name__ + "."):
        if name.endswith(".energy_model") or name.endswith(".performance_model"):
            modules[name] = importlib.import_module(name)
    return modules

def prediction(hardware, bgo_dag, graph=None):
    graph_characteristics = None
    graph_benchmarks = None

    if graph:
        if isinstance(graph, dict):
            graph_characteristics = graph_data
        else:
            graph_benchmarks = graph_data
            
    # Loop over bgo dag and do energy and performance predictions for all hardware configurations.
    for i, bgo in enumerate(bgo_dag):
        bgo_dag[i]['performances'] = []

        if graph_benchmarks is not None:
            for host in hardware['hosts']:
                performance, energy = utils.get_benchmark_result(graph_benchmarks, bgo['name'], host['name'])
                bgo_dag[i]['performances'].append({'host': host['name'], 'runtime': performance, 'energy': energy})
        else:
            # Each of these dictionaries stores as keys the implementation name of the specific bgo implementation, and as values the model functions.
            CPU_perf_models = {}
            CPU_energy_models = {}
            GPU_perf_models = {}
            GPU_energy_models = {}
            for name, model in all_models.items():
                model_parts = name.split('.')
                model_bgo = model_parts[1]
                model_device = model_parts[2]
                model_implementation = model_parts[3]
                model_type = model_parts[-1]
                
                if model_bgo != bgo["name"]:
                    continue

                if model_device == "CPU" and model_type == "performance_model":
                    CPU_perf_models[model_implementation] = model
                elif model_device == "CPU" and model_type == "energy_model":
                    CPU_energy_models[model_implementation] = model
                elif model_device == "GPU" and model_type == "performance_model":
                    GPU_perf_models[model_implementation] = model
                elif model_device == "GPU" and model_type == "energy_model":
                    GPU_energy_models[model_implementation] = model

            # Error handling
            if not CPU_perf_models and not GPU_perf_models:
                utils.exit_with_error(f'No performance model found for BGO {bgo["name"]}')
            if not CPU_energy_models and not GPU_energy_models:
                utils.exit_with_error(f'No energy model found for BGO {bgo["name"]}')

            # Predict performance and energy, and annotate bgo_dag with the prediction values for each host.
            for host in hardware['hosts']:
                performance = {'CPU': {implementation: model.predict(host) for implementation, model in CPU_perf_models.items()},
                               'GPU': {implementation: model.predict(host) for implementation, model in GPU_perf_models.items()}}
                energy      = {'CPU': {implementation: model.predict(host) for implementation, model in CPU_energy_models.items()},
                               'GPU': {implementation: model.predict(host) for implementation, model in GPU_energy_models.items()}}
                if graph_characteristics is not None:
                    performance = {'CPU': {implementation: evaluate(model.predict(host), graph_characteristics) for implementation, model in CPU_perf_models.items()},
                                   'GPU': {implementation: evaluate(model.predict(host), graph_characteristics) for implementation, model in GPU_perf_models.items()}}
                    energy      = {'CPU': {implementation: evaluate(model.predict(host), graph_characteristics) for implementation, model in CPU_energy_models.items()},
                                   'GPU': {implementation: evaluate(model.predict(host), graph_characteristics) for implementation, model in GPU_energy_models.items()}}

                bgo_dag[i]['performances'].append({'host': host['name'], 'runtime': performance, 'energy': energy})
    return bgo_dag



if __name__ == '__main__':
    all_models = import_models(models)
    arguments = sys.argv[1:]

    # Error handling.
    if len(arguments) < 2:
        utils.exit_with_error('Usage: ./prediction.py <hardware_information_json> <bgo_dag> [graph_characteristics or graph_name]')

    # Read the hardware information from the first argument.
    hardware = utils.try_cast_json_dict(utils.check_file(arguments[0]), 'Invalid JSON format for hardware information')
    bgo_dag = utils.try_cast_json_list(utils.check_file(arguments[1]), 'Invalid JSON format for BGO DAG')

    # Evaluate the model if the last argument is 'evaluate_model'.
    graph_data = None
    if len(arguments) == 3:
        graph_data = utils.try_cast_graph_name_or_characteristics(arguments[2], 'Invalid graph characteristics or name format')

    bgo_dag = prediction(hardware, bgo_dag, graph_data)
    print(json.dumps(bgo_dag, indent=4))

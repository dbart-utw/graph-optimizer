from flask import Flask, request, jsonify
import json
import subprocess
import pathlib
from datetime import datetime

app = Flask(__name__)

def predict(request, evaluate, benchmark):
    hardware = request.form.get('hardware')
    bgo_dag = request.form.get('bgo_dag')
    command = ['python3', '-m', 'models.prediction', hardware, bgo_dag]

    if (evaluate):
        graph_props = request.form.get('graph_props')
        command.append(graph_props)

    if (benchmark):
        graph_benchmarks = request.form.get('graph_benchmarks')
        command.append(graph_benchmarks)

    try:
        output = subprocess.check_output(command)
    except subprocess.CalledProcessError as e:
        return jsonify({"command": ' '.join(command), "error": str(e)}), 500
    return output


def greenify_output(output):
    greenifier_output = {"tasks": json.loads(output)}

    for bgo in greenifier_output['tasks']:
        bgo['runTimes'] = {perf['host']: int(perf['runtime']) for perf in bgo['performances']}
        bgo['energyConsumption'] = {perf['host']: int(perf['energy']) for perf in bgo['performances']}
        bgo['submissionTime'] = datetime.today().strftime('%Y-%m-%d')

        del bgo['performances']

    return greenifier_output


@app.route('/models', methods=['POST'])
def models():
    return predict(request, False, False)


@app.route('/evaluate', methods=['POST'])
def evaluate():
    return predict(request, True, False)


@app.route('/benchmark', methods=['POST'])
def benchmark():
    output = predict(request, False, True)
    if request.form.get('greenify') == 'true':
        return greenify_output(output)
    return output


@app.route('/greenifier', methods=['POST'])
def greenifier():
    output = predict(request, True)
    return greenify_output(output)

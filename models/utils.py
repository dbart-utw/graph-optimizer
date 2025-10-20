import json
import pandas as pd

ERROR_404 = "No analytical model available..."

def exit_with_error(message):
    print("Error:", message)
    exit(1)


def check_file(string):
    try:
        with open(string, 'r') as f:
            return f.read()
    except:
        return string


def try_cast_csv(string, error):
    try:
        return pd.read_csv(string)
    except:
        exit_with_error(error)


def try_cast_json_dict(string, error):
    try:
        return dict(json.loads(string))
    except:
        exit_with_error(error)


def try_cast_graph_name_or_characteristics(string, error):
    try:
        return dict(json.loads(string))
    except:
        try:
            return pd.read_csv(f'models/benchmarks/{string}.csv')
        except:
            exit_with_error(error)

def try_cast_json_list(string, error):
    try:
        return list(json.loads(string))
    except:
        exit_with_error(error)


def try_cast_float(string, error):
    try:
        return float(string)
    except ValueError:
        exit_with_error(error)


def try_cast_float_list(string, error):
    try:
        l = string.strip('][').split(',')

        if len(l) == 0:
            exit_with_error(error)

        return list(map(lambda x: float(x.strip()), l))
    except:
        exit_with_error(error)


def avg_mem_access_time(miss_rates, mem_access_times):
    time = 0
    final_component = 1
    for i in range(len(miss_rates)):
        summand = 1
        for j in range(i):
            summand *= miss_rates[j]
        summand *= (1-miss_rates[i])*mem_access_times[i]
        time += summand
        final_component *= miss_rates[i]

    time += final_component*mem_access_times[-1]

    return time


def T_malloc(n):
    return 0


def get_benchmark_result(df, bgo, host):
    if df.empty:
        exit_with_error("Benchmark file is empty.")

    # Filter the DataFrame for the specific BGO
    df_bgo = df[df['bgo'] == bgo]
    if df_bgo.empty:
        exit_with_error(f"'bgo' {bgo} not found in benchmark file.")

    df_host = df_bgo[df_bgo['host_name'] == host]
    if df_host.empty:
        exit_with_error(f"'host_name' {host} not found for BGO {bgo} in benchmark file.")

    try:
        result_time = df_host['time'].mean() * 1000  # Convert to milliseconds
    except KeyError:
        exit_with_error("'time' column not found in benchmark results.")

    try:
        result_energy = df_host['total_energy'].mean()
    except KeyError:
        exit_with_error("'total_energy' column not found in benchmark results.")

    # Convert to a dictionary and return
    return result_time, result_energy
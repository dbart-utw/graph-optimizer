import os
import sys
import subprocess
import pandas as pd
import argparse

def run_benchmark(graph_file, num_runs):
    # Make "temp" directory if it doesn't exist
    if not os.path.exists('temp'):
        os.makedirs('temp')

    # Run the benchmark script
    try:
        subprocess.run(['sudo', './run_benchmark', graph_file, str(num_runs)], check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error running benchmark for {graph_file}: {e}")
        return False

def gather_results(graph_file, host_name):
    total_df = pd.DataFrame()
    for csv in os.listdir('temp'):
        if not csv.endswith('.csv'):
            continue

        df = pd.read_csv(os.path.join('temp', csv)).rename(columns={"Time": "time"})

        df_energy = df.drop(columns=['time'])
        df['total_energy'] = df_energy[list(df_energy.columns)].sum(axis=1)

        algo, source = csv.split('-')
        df['bgo'] = algo
        df['source'] = source
        df['graph'] = graph_file.split('/')[-1][:-4]
        df['host_name'] = host_name

        total_df = pd.concat([total_df, df], ignore_index=True)

    total_df.to_csv(f'results/{graph_file.split('/')[-1][:-4]}.csv', index=False)

    for csv in os.listdir('temp'):
        os.remove(os.path.join('temp', csv))
    os.rmdir('temp')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run benchmarks and gather results.')
    # if multiple graph files are provided, the script will run benchmarks for each
    parser.add_argument('--graph_file', type=str, help='Path to the graph file', nargs='+')
    parser.add_argument('--num_runs', type=int, default=10, help='Number of runs for each benchmark')
    parser.add_argument('--results_dir', type=str, default='results', help='Directory to save results')
    parser.add_argument('--temp_dir', type=str, default='temp', help='Temporary directory for benchmark runs')
    parser.add_argument('--host_name', type=str, default='localhost', help='Host name for the benchmark', required=True)

    args = parser.parse_args()

    for graph_file in args.graph_file:
        if not os.path.exists(graph_file):
            print(f"Graph file {graph_file} does not exist.")
            continue
        if run_benchmark(graph_file, args.num_runs):
            gather_results(graph_file, args.host_name)
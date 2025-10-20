import os
import pandas as pd

dirs = ['balanced', 'performance', 'power_save']
output_dfs = {}

for dir in dirs:
    for file in os.listdir(dir):
        if not file.endswith('.csv'):
            continue
        if file not in output_dfs:
            output_dfs[file] = []
        file_path = os.path.join(dir, file)
        df = pd.read_csv(file_path)
        output_dfs[file].append(df)

for file, dfs in output_dfs.items():
    if len(dfs) == 1:
        output_dfs[file] = dfs[0]
    else:
        output_dfs[file] = pd.concat(dfs, ignore_index=True)
    output_dfs[file].to_csv(file, index=False)
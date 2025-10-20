import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


df_mean = lambda df: df.groupby(['case']+list(df.columns[5:]), as_index=False, sort=False)['runtime_ns'].mean()
R_squared = lambda y, f: 1 - sum((y - f)**2) / sum((y - np.mean(y))**2)

df_bc = df_mean(pd.read_csv('data/simple/results/bc.csv'))
df_fm = df_mean(pd.read_csv('data/simple/results/find_max.csv'))
df_bfs = df_mean(pd.read_csv('data/simple/results/bfs.csv'))
df_combined = df_mean(pd.read_csv('data/simple/results/combined.csv'))

rt_bc = np.array(df_bc['runtime_ns'])
rt_fm = np.array(df_fm['runtime_ns'])
rt_bfs = np.array(df_bfs['runtime_ns'])

rt_expected = rt_bc + rt_fm + rt_bfs
rt_combined = np.array(df_combined['runtime_ns'])

print(f"R² = {R_squared(rt_combined, rt_expected):6.4f}")

V_size = np.array(df_combined['G.SIZE_VERTS'])
E_size = np.array(df_combined['G.SIZE_EDGES'])

plt.scatter(V_size, rt_expected, marker='.', label='Predicted')
plt.scatter(V_size, rt_combined, marker='.', label='Actual')
plt.xlabel('$|V|$')
plt.ylabel('runtime (in ns)')
plt.legend()
plt.close(plt.savefig('rt_simple_V.png'))

plt.scatter(E_size, rt_expected, marker='.', label='Predicted')
plt.scatter(E_size, rt_combined, marker='.', label='Expected')
plt.xlabel('$|E|$')
plt.ylabel('runtime (in ns)')
plt.legend()
plt.close(plt.savefig('rt_simple_E.png'))

print("Check the output graphs at 'rt_simple_V.png' and 'rt_simple_E.png'")

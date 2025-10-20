import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


df_mean = lambda df: df.groupby(['case']+list(df.columns[5:]), as_index=False, sort=False)['runtime_ns'].mean()
R_squared = lambda y, f: 1 - sum((y - f)**2) / sum((y - np.mean(y))**2)

df_bc = df_mean(pd.read_csv('data/simple/results/bc.csv'))
df_fm = df_mean(pd.read_csv('data/simple/results/find_max.csv'))
df_convert = df_mean(pd.read_csv('data/convert/results/convert.csv'))
df_bfs = df_mean(pd.read_csv('data/convert/results/bfs.csv'))
df_combined = df_mean(pd.read_csv('data/convert/results/combined.csv'))

rt_bc = np.array(df_bc['runtime_ns'])
rt_fm = np.array(df_fm['runtime_ns'])
rt_bfs = np.array(df_bfs['runtime_ns'])

V_size = np.array(df_combined['G.SIZE_VERTS'])
E_size = np.array(df_combined['G.SIZE_EDGES'])

# Linear model of the conversion runtime
coef = [0.2848204218822067, 411.54150917481775, -7574573.333558306]  # Obtained from experiment.
A = np.column_stack((V_size**2, E_size, np.ones(len(V_size))))
rt_convert = np.dot(A, coef)

rt_expected = rt_bc + rt_fm + rt_convert + rt_bfs
rt_combined = np.array(df_combined['runtime_ns'])

print(f"R² = {R_squared(rt_combined, rt_expected):6.4f}")

plt.scatter(V_size, rt_expected, marker='.', label='Predicted')
plt.scatter(V_size, rt_combined, marker='.', label='Actual')
plt.xlabel('$|V|$')
plt.ylabel('runtime (in ns)')
plt.legend()
plt.close(plt.savefig('rt_convert_V.png'))

plt.scatter(E_size, rt_expected, marker='.', label='Predicted')
plt.scatter(E_size, rt_combined, marker='.', label='Expected')
plt.xlabel('$|E|$')
plt.ylabel('runtime (in ns)')
plt.legend()
plt.close(plt.savefig('rt_convert_E.png'))

print("Check the output graphs at 'rt_convert_V.png' and 'rt_convert_E.png'")

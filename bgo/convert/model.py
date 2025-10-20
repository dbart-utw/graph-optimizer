# Script to calibrate and test the runtime performance model for the conversion.
# Model: a * |V|^2 + b * |E| + c

import sys
import random
import argparse

import pandas as pd
import numpy as np


parser = argparse.ArgumentParser()
parser.add_argument("trainfile", type=str)
parser.add_argument("testfile", nargs='?', type=str)
parser.add_argument("--seed", type=str)
parser.add_argument("--split", type=float, default=0.5)
args = parser.parse_args()


random.seed(args.seed)

if not args.testfile:
    # Just use the first file and split it up into a training and test data.
    df = pd.read_csv(args.trainfile)

    # Randomly select files for the training and test set.
    filenames = df['G.PATH'].unique()
    random.shuffle(filenames)
    filenames_t = filenames[:int(len(filenames)*args.split)]
    filenames_s = filenames[int(len(filenames)*args.split):]

    # Prepare the training and test sets without filtering out any data.
    data_ct = df[df['G.PATH'].isin(filenames_t)]
    data_cs = df[df['G.PATH'].isin(filenames_s)]
else:
    data_ct = pd.read_csv(args.trainfile)
    data_cs = pd.read_csv(args.testfile)

# Prepare filtered sets by removing the 3 highest and 3 lowest obtained runtimes from each file.
remove_extremes = lambda group : group.sort_values('runtime_ns').iloc[3:-3].sort_values('run')
data_ft = data_ct.groupby('G.PATH').apply(remove_extremes).reset_index(drop=True)
data_fs = data_cs.groupby('G.PATH').apply(remove_extremes).reset_index(drop=True)


def prepare(df):
    # Prepare the matrix A and vector b using the data from the Pandas dataframe.
    data = df[['G.SIZE_VERTS', 'G.SIZE_EDGES', 'runtime_ns']].to_numpy()
    A = np.column_stack((data[:, 0]**2, data[:, 1], np.ones(len(data))))
    b = data[:, 2]
    return A, b

# Prepare matrix A and vector b.
A_ct, b_ct = prepare(data_ct)
A_cs, b_cs = prepare(data_cs)
A_ft, b_ft = prepare(data_ft)
A_fs, b_fs = prepare(data_fs)

# Calibrate the model using least squares, AKA find the optimal x.
x_c, _, _, _ = np.linalg.lstsq(A_ct, b_ct, rcond=None)
x_f, _, _, _ = np.linalg.lstsq(A_ft, b_ft, rcond=None)

print(f"coef x (complete) : [{', '.join([f'{x}' for x in x_c])}]")
print(f"coef x (filtered) : [{', '.join([f'{x}' for x in x_f])}]")

# Calculate the predicted runtimes using the calibrated model.
b_ctp = np.dot(A_ct, x_c)
b_csp = np.dot(A_cs, x_c)
b_ftp = np.dot(A_ft, x_f)
b_fsp = np.dot(A_fs, x_f)

# Calculate the coefficient of determination
R_squared = lambda p, a: 1 - sum((a - p)**2) / sum((a - np.mean(a))**2)
Rs_ct = R_squared(b_ctp, b_ct)
Rs_cs = R_squared(b_csp, b_cs)
Rs_ft = R_squared(b_ftp, b_ft)
Rs_fs = R_squared(b_fsp, b_fs)
Rs_cb = R_squared(np.concatenate((b_ctp, b_csp)), np.concatenate((b_ct, b_cs)))
Rs_fb = R_squared(np.concatenate((b_ftp, b_fsp)), np.concatenate((b_ft, b_fs)))


print(f"     complete  filtered\n"
      f" T    {Rs_ct:5.4f}    {Rs_ft:5.4f}\n"
      f" S    {Rs_cs:5.4f}    {Rs_fs:5.4f}\n"
      f"T+S   {Rs_cb:5.4f}    {Rs_fb:5.4f}")

import os
import pandas as pd

mappings = {'symmetry': {'sym': 'symmetric', 'asym': 'general'},
            'dtype': {'unweighted': 'integer', 'positive': 'integer', 'posweighted': 'real', 'signed': 'real', 'multisigned': 'real', 'weighted': 'integer', 'multiweighted': 'integer', 'multiposweighted': 'real'}}

metadata = pd.read_csv('graph_urls.csv')


def check_file(filename, dir):
    if not os.path.exists(filename):
        return False
    if metadata[metadata['name'] == dir]['vertex-count'].all():
        return True

    return False


def get_header(line):
    terms = line.split()[1:]
    print(terms)
    dtype = next((mappings['dtype'][term] for term in terms if term in mappings['dtype']), None)
    sym = next((mappings['symmetry'][term] for term in terms if term in mappings['symmetry']), None)
    print(dtype, sym)

    if dtype is None or sym is None:
        return None, None

    return f'%%MatrixMarket matrix coordinate {dtype} {sym}\n', next((term for term in terms if term in mappings['dtype']), None)


def get_nm(dir):
    vertex_count = metadata[metadata['name'] == dir].iloc[0]['vertex-count']
    edge_count = metadata[metadata['name'] == dir].iloc[0]['edge-count']

    return f'{vertex_count} {vertex_count} {edge_count}\n'


def convert_konect(dir, outdir):
    if os.path.exists(f'{outdir}/{dir}.mtx'):
        return

    abspath = os.path.abspath(f'raw/{dir}')
    filename = os.path.join(abspath, dir, f'out.{dir}')
    if not check_file(filename, dir):
        return

    with open(filename, 'r') as f:
        lines = f.readlines()

    header, term = get_header(lines[0])
    if header is None:
        return

    with open(f'{outdir}/{dir}.mtx', 'w') as f:
        f.write(header)
        f.write(get_nm(dir))

        for line in lines[1:]:
            if line.startswith('%'):
                continue

            if line == '\n':
                continue

            val = 1
            if not (term == 'unweighted' or term == 'positive' or term == 'dynamic'):
                func = int if mappings['dtype'][term] == 'integer' else float
                val = func(line.split()[2])

            u, v = map(int, line.split()[:2])
            f.write(f'{u} {v} {val}\n')


if __name__ == '__main__':
    for dir in os.listdir('raw'):
        convert_konect(dir, 'converted')
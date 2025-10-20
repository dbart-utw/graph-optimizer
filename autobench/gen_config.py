"""
Generates the 'config.json' file.
"""

import sys

from os.path import abspath, basename, dirname, exists, expanduser, join, relpath
from pathlib import Path
import json

from scipy.io import mminfo
from clang.cindex import Index

from util.config import *

from gen_bench_code import parse_bgo_header


def update_data_config(config, base_dir, data_dir):
    mtx_files = [str(path) for path in Path(data_dir).glob('**/*.mtx')]

    if 'data' not in config:
        config['data'] = {}

    for mtx_file in mtx_files:
        with open(mtx_file, 'r') as f:
            rows, cols, entries, _, _, _ = mminfo(f)
            if rows != cols:
                print(f"Error in '{mtx_file}', number of rows and columns does not match!", file=sys.stderr)
                continue

            name = relpath(mtx_file, base_dir)
            if name not in config['data']:
                config['data'][name] = {}

            config['data'][name]['size_verts'] = rows
            config['data'][name]['size_edges'] = entries


def update_abs_bgo_config(config, base_dir, bgos_dir):
    config_files = [str(path) for path in Path(bgos_dir).glob('*/config.json')]

    if 'bgos' not in config:
        config['bgos'] = {}

    for config_file in config_files:
        abs_bgo_name = basename(dirname(config_file))
        if abs_bgo_name not in config['bgos']:
            config['bgos'][abs_bgo_name] = {}
        with open(config_file, 'r') as f:
            bgo = json.load(f)
            config['bgos'][abs_bgo_name] = {**config['bgos'][abs_bgo_name], **bgo}


def update_imp_bgo_config(config, base_dir, bgos_dir):
    header_files = [str(path) for path in Path(bgos_dir).glob('*/*/*.hpp')]

    if 'bgos' not in config:
        config['bgos'] = {}

    for header_file in header_files:
        if basename(header_file).split('.')[0] != basename(dirname(header_file)):
            continue

        signature = parse_bgo_header(header_file, INCLUDES)
        signature['header'] = relpath(signature['header'], BASE_DIR)

        imp_bgo_name = basename(dirname(header_file))
        abs_bgo_name = relpath(abspath(join(dirname(header_file), '../')), BASE_DIR)
        if abs_bgo_name not in config['bgos']:
            config['bgos'][abs_bgo_name] = {}
        if 'implementations' not in config['bgos'][abs_bgo_name]:
            config['bgos'][abs_bgo_name]['implementations'] = {}
        config['bgos'][abs_bgo_name]['implementations'][imp_bgo_name] = signature


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description="Populate config files with available data.")
    parser.add_argument('--parts', nargs='+', choices=['all', 'data', 'abs_bgo', 'imp_bgo'], default=['all'])
    args = parser.parse_args()

    if 'all' in args.parts and len(args.parts) > 1:
        parser.error('The \'all\' option cannot be combined with other options.')

    config = {}
    if exists(CONFIG_FILE):
        with open(CONFIG_FILE, 'r') as f:
            config = json.load(f)

    if 'all' in args.parts or 'data' in args.parts:
        update_data_config(config, BASE_DIR, DATA_DIR)
    if 'all' in args.parts or 'abs_bgo' in args.parts:
        update_abs_bgo_config(config, BASE_DIR, BGOS_DIR)
    if 'all' in args.parts or 'imp_bgo' in args.parts:
        update_imp_bgo_config(config, BASE_DIR, BGOS_DIR)

    with open(CONFIG_FILE, 'w') as f:
        json.dump(config, f, indent=4)

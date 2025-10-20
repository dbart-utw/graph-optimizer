import argparse
import os

from clang.cindex import Index
# for docker container:
from clang.cindex import Config
Config.set_library_file("/usr/lib/llvm-15/lib/libclang.so.1")

from jinja2 import Environment, FileSystemLoader

from util.config import TRANSLATIONS, REORDER_OPTIONS


DEFAULT_TEMPLATE_PATH = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'misc/template.cpp')


def parse_bgo_header(headerfile, includes=''):
    """Read the first (!) function definition of a header file and return the signature."""
    def traverse_ast(node):
        if str(node.location.file) == str(headerfile) and node.kind.name == 'FUNCTION_DECL':
            return {
                'header': headerfile,
                'method': node.spelling,
                'return': node.result_type.spelling.replace(' ', ''),
                'args': [arg.type.spelling.replace(' ', '') for arg in node.get_arguments()],
            }
        for child in node.get_children():
            result = traverse_ast(child)
            if result:
                return result
        return None

    index = Index.create()
    root = index.parse(headerfile, args=includes.split(' ')).cursor
    return traverse_ast(root)


def generate_bench_code(context, template_path):
    """Generate benchmarking code based on a function signature and template."""
    context['argc'] = len(context['args'])
    for phase in ['decl', 'init', 'save', 'free', 'name']:
        context[f'{phase}s'] = []
        for i, arg in enumerate(context['args'], start=1):
            if phase not in TRANSLATIONS[arg]:
                continue
            context[f'{phase}s'].append(TRANSLATIONS[arg][phase].format(i=i))

    template = Environment(loader=FileSystemLoader(os.path.dirname(template_path))).get_template(os.path.basename(template_path))
    return template.render(context)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('sourcefile', type=str)
    parser.add_argument('headerfile', type=str)
    parser.add_argument('includes', type=str, default='')
    parser.add_argument('--template', type=str, default=DEFAULT_TEMPLATE_PATH)
    args = parser.parse_args()

    context = parse_bgo_header(args.headerfile, args.includes)
    context['reorder_options'] = REORDER_OPTIONS
    code = generate_bench_code(context, args.template)

    with open(args.sourcefile, 'w') as f:
        f.write(code)

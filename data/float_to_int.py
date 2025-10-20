import os

for file in os.listdir('converted/small'):
    # Remove the .0 from the end of every number in the secod line of the file
    with open(f'converted/{file}', 'r') as f:
        lines = f.readlines()
        lines[1] = ' '.join([l.split('.')[0] for l in lines[1].split()]) + '\n'

    with open(f'converted/{file}', 'w') as f:
        f.writelines(lines)
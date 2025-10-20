# Read the lines in a file. If a line is a duplicate of a previous line, remove it.
import sys
def remove_duplicate_edges(input_file, output_file):
    seen = set()
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        for line in infile:
            if line not in seen:
                seen.add(line)
                outfile.write(line)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python remove_duplicate_edges.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    remove_duplicate_edges(input_file, output_file)
    print(f"Duplicate edges removed. Output written to {output_file}.")
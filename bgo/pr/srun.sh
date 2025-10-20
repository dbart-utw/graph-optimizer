#!/bin/sh

for file in "$1"/* ;do
    [ -f "$file" ] && echo "Process '$file'." && srun ./main "$file"
done
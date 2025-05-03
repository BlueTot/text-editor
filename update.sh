#!/bin/bash

cd build
cmake ..
if [ -f compile_commands.json ]; then
    mv compile_commands.json ../
else
    echo "compile_commands.json does not exist"
fi
cd ..
exit 0


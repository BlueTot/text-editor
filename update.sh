#!/bin/bash

cd build
cmake ..
if [ -f config_commands.json ]; then
    mv config_commands ../
else
    echo "config_commands.json does not exist"
fi
cd ..
exit 0


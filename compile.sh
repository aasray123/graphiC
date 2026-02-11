#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e

# Navigate to the build directory
cd build

# Generate the makefiles
./premake5 gmake

# Navigate back to the parent directory
cd ..

# Compile the project. If this fails, the script will stop.
make

# Navigate to the executable's directory and run it
# ./bin/Debug/graphiC src/script.txt
# ./bin/Debug/graphiC src/test.txt
./bin/Debug/graphiC $1
# cd bin/Debug
# ./graphiC src/script.txt
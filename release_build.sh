#! /bin/bash

rm -rf build
mkdir build
cd build
mkdir bin
cmake .. -DCMAKE_BUILD_TYPE:STRING=Release
make

exit 0

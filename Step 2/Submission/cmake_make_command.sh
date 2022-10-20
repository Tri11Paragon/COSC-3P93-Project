#!/bin/bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j 16
./Step_2

#!/bin/bash
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
./Step_2

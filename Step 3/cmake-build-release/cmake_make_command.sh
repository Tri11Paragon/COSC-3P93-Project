#!/bin/bash
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ../
ninja -j 16 && ./Step_2

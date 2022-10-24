#/bin/bash
cd Step\ 3
rm -fr build
mkdir build
cd build
cmake ../
make -j 4

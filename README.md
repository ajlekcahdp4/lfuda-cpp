# LFUDA cache implementation on C++
## How to build
Linux
```
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release
cd build/
make
```
## How to run tests
```
cd build/
ctest
```
## How to compare number of hits of lfuda and belady (ideal cache) 
```
cd test
./compare.sh
```
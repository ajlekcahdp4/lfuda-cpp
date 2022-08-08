# LFUDA cache implementation on C++
## table of contents
 1. [about](#about)
 2. [How to](#how-to-build)
 3. [details](#details)
## about
In this repository I implemented LFU-DA (least frequently used with dynamic aging) page replacement policy. And compare this policy with ideal cache replacement policy - Belady's (See the result of this comparision on random tests [there](test/compared.txt)).
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
## details
### review
LFU-DA policy similar to the classic LFU except that cache stores his age and the weight of new cache members is set to this age, not to 1 (as in LFU). As in the LFU when needs to insert new element in cache we evict the element with the lowest weight. In other words, LFU-DA combines LFU and LRU policies.

### Complexity of the lookup_update () function: 
ln(N) on average, worst case N * ln(N),
where N - cache capacity.
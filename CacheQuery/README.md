# CacheQuery

In this part of this repo, we implement an a convenient tool for manually interacting with translation caches.

## Usage

To compile and run this experiment you can simple run

```
make test
```
Afterwards, you can run

```
sudo ./trigger "ABCDA"
```

where ABCD corresponds to an access order. In this example it would first access cache line A, next cache line B, afterwards, cache line C, and so on...

## Configuration

We have not implemented support for command line arguments due to time constraints. Instead this tool can be configured by adapting values in `shared/include/definitions.h` and `kernel_space/src/query.c`.

We can define the translation cache level using LEVEL, the amount of set_bits and ways in SET_BITS and WAYS and the implemented (suspected) hash function in `hash`. 
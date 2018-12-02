FileKVStore: File Based Key-Value Store
d
-----------------------------------

### What is it?

FileKVStore is a file based KV Store that provides fast query service. Given a static big file, FileKVStore provides a random query service as you fast as you need.

* src/ : Main source code
* test/ : Unit tests
* tool/ : A tools to generate data file for test purpose. 
* DataFile format is : | key size(fixed64) | key(bytes) | value size (fixed64) | value(bytes) |
* IndexFile format is : | key size(fixed64) | key(bytes) | Location(fixed64) |

where typically key size < 1 KB,value size < 1 MB.

### Design
The key part is to build index on bootstrap and provide query service on demand.  No concurrent writed is allowed. Design target is main for fast concurrent read.
1. Hash is real good fit for fast query. So Hash Index is implemented. And 3-Level hashing is implemented to reduce hash collisions. An index file is for one or more keys.
The index meta directory will be LVL1_x/LVL2_y/IDX_z.  where x = hash1(key) % BUCKET_NUM, y = hash2(key) % BUCKET_NUM, z = hash3(key) % BUCKET_NUM.
For now we have 4096 BUCKET_NUM, that means we support 4096 * 4096 * 4096 or more concurrent indexing.
2. LRU Cache for Index.  Value size could big as 1 MB, so we will so cache Value. An concurrent Cache is implemented by sharding segments. So concurrent read requests may not block each other most time.
3. Basically it needs just two disk reads to load a value, one is for index and the other is for data value.  Idealy on cache hit, index loading is saved and only single IO on disk is needed. This will be good for read performance. Ofcourse, Caching hit rate really depends on IO patten and Cache memory. For sequential IO, it's better to preread to improve the caching performance.

### Build Notes
Just Run 'source BuildMe'
```
-> # source BuildMe
g++ -std=c++11 -I../include   -c -o Cache.o Cache.cpp
g++ -std=c++11 -I../include   -c -o HashIndexStore.o HashIndexStore.cpp
...
```

### Test
Under tests. Below is an example output.

```
-> # ./fileStoreTest
[==========] Running 2 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 2 tests from FileStoreTest
Generating 10000 entries of KV Pair with maxKeyLen 1024 Bytes maxValueLen 1024 Bytes
Saved in file data2
Index Root Path meta
Successfully build index
[ RUN      ] FileStoreTest.sequential
[       OK ] FileStoreTest.sequential (1394 ms)
[ RUN      ] FileStoreTest.random
[       OK ] FileStoreTest.random (956 ms)
 Total Cache Hit 10000 out of 20000, hitRate(50%)
[----------] 2 tests from FileStoreTest (2350 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test case ran. (3985 ms total)
[  PASSED  ] 2 tests.
```

### Dependencies

FileKVStore requires gcc 4.8+ with C++11 support.

googletest is required to build and run the tests under test.

### TODO

* BPlusTree store implementation. Make both use of benifits of Hash and BPlusTree.
* Block Read on datafile. this is used to improve IO efficiency.
* Memory Map on lowlevel file access.


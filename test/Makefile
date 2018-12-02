CXX     	= g++
CXXFLAGS	= -std=c++11 -I../include
FLAGS		= -o
LDFLAGS 	= -lgtest
SRCS 		= $(shell find . -name "*.cpp")
TARGETS		= cacheTest fileStoreTest
EXECUTABLE	= cacheTest fileStoreTest


all: cacheTest fileStoreTest

cacheTest: CacheTest.o
	$(CXX) $(CXXFLAGS) -o cacheTest CacheTest.o ../src/Cache.o ../src/JenkinsHash.o $(LDFLAGS)

fileStoreTest: FileStoreTest.o
	$(CXX) $(CXXFLAGS) -o fileStoreTest FileStoreTest.o ../src/FileStore.o ../src/Cache.o ../src/JenkinsHash.o ../src/IndexStore.o ../src/HashIndexStore.o ../src/Util.o ../src/Cleanable.o ../tool/DataGenerator.o $(LDFLAGS)

.PHONY: clean 

clean:
	rm -rf *.o $(EXECUTABLE) meta data2


cmake_minimum_required(VERSION 3.12)
project(FileKVStore)

set(CMAKE_CXX_STANDARD 11)
set(APP_HEADER ./include)
set(APP_SOURCE ./src)

include_directories(${APP_HEADER})
include_directories(${APP_SOURCE})

set(
        SOURCE_FILES
        tool/generateDataFile.cpp
        ${APP_SOURCE}/Main.cpp
        ${APP_SOURCE}/Cache.cpp
        ${APP_SOURCE}/JenkinsHash.cpp
        ${APP_SOURCE}/FileStore.cpp
        ${APP_SOURCE}/IndexStore.cpp
        ${APP_SOURCE}/HashIndexStore.cpp
        ${APP_SOURCE}/Util.cpp
        include/Status.h include/Cleanable.h src/Cleanable.cpp src/BPlusTreeIndexStore.cpp
        test/CacheTest.cpp test/FileStoreTest.cpp tool/DataGenerator.cpp include/DataGenerator.h)

add_executable(FileKVStore ${SOURCE_FILES})
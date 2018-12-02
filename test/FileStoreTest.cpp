#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "Cache.h"
#include "FileStore.h"
#include "DataGenerator.h"

constexpr long long KEY_TAG   = 0x5F5F5F5F59454B24; // reverse of '$KEY____'
constexpr long long VALUE_TAG = 0x5F5F45554C415624; // reverse of '$VALUE__'

class FileStoreTest : public ::testing::Test {
protected:
    constexpr static const char* dataFileName = (const char*)"data2";
    constexpr static long entryNumber = 10000;
    constexpr static long maxKeyLen = 1024;
    constexpr static long maxValueLen = 1024;
    static FileStore* pFileStore;
    static std::vector<std::string> keys;
    static std::vector<std::string> values;
    static void SetUpTestCase() {
        keys.reserve(entryNumber);
        values.reserve(entryNumber);
        ASSERT_EQ(0, keys.size());
        ASSERT_EQ(0, values.size());
        DataGenerator::GenerateData(dataFileName, entryNumber, maxKeyLen, KEY_TAG, maxValueLen, VALUE_TAG, &keys, &values);

        pFileStore = new FileStore(dataFileName, "meta");
        pFileStore->Open();

    }
    static void TearDownTestCase() {
        pFileStore->Close();
        delete pFileStore;
        pFileStore = nullptr;
    }

    static long getRandom(long range) {
        std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch());
        return us.count() % range;
    }

};
FileStore* FileStoreTest::pFileStore = nullptr;
std::vector<std::string> FileStoreTest::keys = std::vector<std::string>();
std::vector<std::string> FileStoreTest::values = std::vector<std::string>();

TEST_F(FileStoreTest, sequential) {
	// do something
	Value value;
	for (int i = 0; i < entryNumber; i++) {
	    Slice key(keys[i].data(), keys[i].size());
	    pFileStore->Get(key, &value);
	    ASSERT_TRUE(value.size() > 0);
	    std::string& expectedValueStr = values[i];
	    ASSERT_EQ(value.size(),  expectedValueStr.size());
	    ASSERT_EQ(0, strcmp(value.data(), expectedValueStr.c_str()));
	}
}

TEST_F(FileStoreTest, random) {
    Value value;
    int loop = 10;
    while (loop-- > 0) {
        int startId = getRandom(entryNumber);
        int endId = startId + 10;
        endId = endId < entryNumber ? endId : entryNumber;
        int repeatReadCount = 100;
        while (repeatReadCount-- > 0) {
            for (int i = startId; i < endId; i++) {
                Slice key(keys[i].data(), keys[i].size());
                pFileStore->Get(key, &value);
                ASSERT_TRUE(value.size() > 0);
                std::string& expectedValueStr = values[i];
                ASSERT_EQ(value.size(),  expectedValueStr.size());
                ASSERT_EQ(0, strcmp(value.data(), expectedValueStr.c_str()));
            }
        }
    }
}

int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

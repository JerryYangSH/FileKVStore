#include "DataGenerator.h"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdlib.h>
using namespace std;

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution;
/**
 * Helper utility to generate data File
 * File Format is : KEY_LEN<8B> KEY VALUE_LEN<8B> VALUE
 * The output keys will be saved in generatedKeys if NOT null.
 */
void DataGenerator::GenerateData(const char *dataFileName, const long entryNumber, const long maxKeyLen, const long long KEY_TAG,
        const long maxValueLen, const long long VALUE_TAG,
        std::vector<string> * generatedKeys,
        std::vector<string> * generatedValues) {
    cout << "Generating " << entryNumber << " entries of KV Pair with maxKeyLen " << maxKeyLen << " Bytes maxValueLen " << maxValueLen << " Bytes" << endl;

    ofstream ofs(dataFileName, ios::out | ios::binary | ios::ate);

    char* buffer = new char[maxKeyLen + maxValueLen + 16]();
    long long seqId = 0x3030303030303030; // increasing unique ID

    int actualKeyLen = 0;
    int actualValueLen = 0;
    for (int i = 0; i < entryNumber; i++) {
        actualKeyLen = generateRandom(buffer, maxKeyLen, KEY_TAG, seqId, false);
        actualValueLen = generateRandom(buffer + actualKeyLen + 8, maxValueLen, VALUE_TAG, seqId, false);
        ofs.write(buffer, actualKeyLen + actualValueLen + 16);
        seqId++;
        // cout << "[DEBUG] Generated key-"<< i << " len = "<< actualKeyLen << " value len = "<< actualValueLen  << endl;
        if (generatedKeys != nullptr) {
            generatedKeys->push_back(std::move(string(buffer + 8, actualKeyLen)));
        }
        if (generatedValues != nullptr) {
            generatedValues->push_back(std::move(string(buffer + actualKeyLen + 16, actualValueLen)));
        }
    }

    ofs.close();
    delete [] buffer;

    cout << "Saved in file " << dataFileName << endl;
}
/**
 * Generate random bytes and fill in buffer
 * Return actualy bytes number generated.
 */
int DataGenerator::generateRandom(char* buffer, const int maxLen, const long long tag, const long long id, bool hardRandom) {
    int64_t* pLong = reinterpret_cast<int64_t *> (buffer);
    int actualLen = distribution(generator) % maxLen;
    actualLen = actualLen < 16 ? 16 : actualLen;
    *pLong++ = actualLen;  /* actual data len */
    *pLong++ = tag;

    if (hardRandom) {
        for (int i = 1; i < actualLen / sizeof(int64_t); i++) {
            *pLong++ = distribution(generator); // truely random, slow
        }
    }
    else {
        for (int i = 1; i < actualLen / sizeof(int64_t); i++) {
            *pLong++ = tag;
        }
    }
    * reinterpret_cast<int64_t *>(buffer + actualLen) = id; // fill last 8 bytes with counter

    return actualLen;
}

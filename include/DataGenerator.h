#pragma once

#include <random>
#include <string>
#include <vector>

// Used for test to generate data file
class DataGenerator {
public:
    static int generateRandom(char* buffer, const int maxLen, const long long tag, const long long id, bool hardRandom);
    static void GenerateData(const char *dataFileName, const long entryNumber,
            const long maxKeyLen, const long long KEY_TAG, const long maxValueLen, const long long VALUE_TAG, std::vector<std::string> * generatedKeys, std::vector<std::string> * generatedValues);
};

#include <iostream>
#include "DataGenerator.h"

using namespace std;

constexpr int MAX_KEY_LEN   = 1024;
constexpr int MAX_VALUE_LEN = (1024 * 1024);
constexpr long long KEY_TAG   = 0x5F5F5F5F59454B24; // reverse of '$KEY____'
constexpr long long VALUE_TAG = 0x5F5F45554C415624; // reverse of '$VALUE__'

int main(int argc, char* argv[]) {
	if (argc < 3) {
			cout << "A tool used to generate data file with KV pairs." << endl;
			cout << "Usage : " << argv[0] <<" <dataFileName> <entryNumber> [maxKeyLen] [maxValueLen]" << endl;
			cout << "\t\tDefault maxKeyLen   = " << MAX_KEY_LEN << " (Bytes)" << endl;  
			cout << "\t\tDefault maxValueLen = " << MAX_VALUE_LEN << " (Bytes)" << endl;  
			return - 1;
	}
	char* dataFileName = argv[1];
	int entryNumber = atoi(argv[2]);
	int maxKeyLen = MAX_KEY_LEN;
	int maxValueLen = MAX_VALUE_LEN;
	if (argc >= 4) {
		maxKeyLen = atoi(argv[3]);
	}
	if (argc >= 5) {
		maxValueLen = atoi(argv[4]);
	}
	DataGenerator::GenerateData(dataFileName, entryNumber, maxKeyLen, KEY_TAG, maxValueLen, VALUE_TAG, nullptr, nullptr);

	return 0;
}


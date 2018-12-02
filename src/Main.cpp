// This file contains the 'main' function. Program execution begins and ends there.

#include <memory>
#include <iostream>
#include "FileStore.h"
using namespace std;

void simple_query_demo(FileStore* pStore);

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage : " << argv[0] <<" <datafile>" << std::endl;
		return - 1;
	}
	char* datafilename = argv[1];
	unique_ptr<FileStore> pStore(new FileStore(datafilename, "meta"));
	pStore->Open();

	// JUST TEST
    simple_query_demo(pStore.get());

    pStore->Close();

	return 0;
}

void simple_query_demo(FileStore* pStore) {
    Key keys [] = {"$KEY____10000000", "$KEY____10000000", "$KEY____20000000", "$KEY____30000000", "$KEY____40000000",
                   "$KEY____50000000", "$KEY____60000000", "$KEY____70000000", "$KEY____80000000", "$KEY____90000000"};
    for (int i = 0; i < 2; i++) {
        for (auto key : keys) {
            Value value;
            Status s = pStore->Get(key, &value);
            if (Status::Ok == s) {
                cout << "[Query] " << key.ToString() << " : " << value.ToString() << endl;
            } else {
                cout << "[Query] " << key.ToString() << " Error " << s << endl;
            }
        }
    }
}

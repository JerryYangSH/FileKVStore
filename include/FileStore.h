#pragma once
#include <fstream>
#include "IndexStore.h"
#include "Status.h"
#include "Types.h"

class FileStore {
private:
	string mDataFileName;
	string mIndexRoot;
	std::unique_ptr<IndexStore> mIndexStore;

	bool updateKeyIndex(const char* key_bytes, size_t key_len, const uint64_t offset);

	Status readKVPair(ifstream& ifs,
		Location loc,
		std::unique_ptr<char[]>& key_bytes, size_t* pkey_len,
		std::unique_ptr<char[]>& value_bytes, size_t* pvalue_len);

	Status readKeyOnly(ifstream& ifs,
		Location loc,
		std::unique_ptr<char[]>& key_bytes, size_t* pkey_len,
		size_t* pvalue_len);
	Status get(const Key& key, const Location loc, Value* value);
    Status buildIndex();

public:
	explicit FileStore(char* dataFileName, const char* indexRoot);
    explicit FileStore(string dataFileName, string indexRoot);
	~FileStore();
	Status Open();
	void Close();
	Status Get(const Key& key, Value* value);
};

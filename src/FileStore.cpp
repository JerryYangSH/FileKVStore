#include <memory>
#include <iostream>
#include "FileStore.h"
#include "Util.h"

using namespace std;

FileStore::FileStore(char* dataFileName, const char* indexRoot) {
	this->mDataFileName = dataFileName;
	this->mIndexRoot = indexRoot;
	this->mIndexStore.reset(new HashIndexStore(mIndexRoot.c_str()));
}

FileStore::FileStore(string dataFileName, string indexRoot) {
    this->mDataFileName = std::move(dataFileName);
    this->mIndexRoot = std::move(indexRoot);
    this->mIndexStore.reset(new HashIndexStore(mIndexRoot.c_str()));
}

FileStore::~FileStore() {
}

Status FileStore::Open() {
	return buildIndex();
}
void FileStore::Close() {
	return;
}
Status FileStore::Get(const Key& key, Value* value) {
	Location loc = mIndexStore->GetIndex(key);
	if (loc == INVALID_LOC) {
		cout <<"No value found for key " << key.ToString() << endl;
		return Status::NotFound;
	}

	return get(key, loc, value);
}

// *******************************************************************************
// private

Status FileStore::buildIndex() {
	unique_ptr<char[]> key_raw_bytes{ new char[Util::MAX_KEY_LEN + 8] };
	char key_len_bytes[8];
	uint64_t actual_key_len;
	uint64_t offset = 0;
	uint64_t total_len;
	ifstream ifs(mDataFileName, ios::in | ios::binary | ios::ate);
	if (!ifs.is_open()) {
		cout << "Unable to open file " << mDataFileName << endl;
		return Status::IOError;
	}
	total_len = static_cast<uint64_t>(ifs.tellg());
	
	while (offset < total_len) {
		ifs.seekg(offset, ios::beg);
		ifs.read(key_len_bytes, 8);
		actual_key_len = Util::Decode64Long(key_len_bytes);
		if (actual_key_len > Util::MAX_KEY_LEN) {
			cout << "Invalid key len "<< actual_key_len << " at offset " << offset << endl;
			ifs.close();
			return Status::Corruption;
		}
		ifs.read(key_raw_bytes.get(), actual_key_len + 8); // load key bytes and value_size together.

		if (!updateKeyIndex(key_raw_bytes.get(), actual_key_len, offset)) {
			cout << "Failed to build key index at offset " << offset << endl;
			ifs.close();
			return Status::IOError;
		}

		uint64_t value_len = Util::Decode64Long(key_raw_bytes.get() + actual_key_len);
		if (value_len > Util::MAX_VALUE_LEN) {
			cout << "Invalid value len " << value_len << " at offset " << offset << endl;
			ifs.close();
			return Status::Corruption;
		}
		offset += actual_key_len + value_len + 16;
	}
	ifs.close();
	cout << "Successfully build index" << endl;
	return Status::Ok;
}

bool FileStore::updateKeyIndex(const char* key_bytes, size_t key_len, const uint64_t offset) {
	if (mIndexStore == nullptr) {
		cout << "Index Store is NULL for dataFile " << mDataFileName << endl;
		return false;
	}
	const Key key(key_bytes, key_len);
	return mIndexStore->UpdateIndex(key, offset);
}

/**
 * readKVPair
 * @param ifs
 * @param loc
 * @param key_bytes
 * @param pkey_len
 * @param value_bytes
 * @param pvalue_len
 * @return
 * Note :
 * Call this with file open already.
 * Bytes buffer life cycle is taken care by unique_ptr.
 */
Status FileStore::readKVPair(ifstream& ifs,
	Location loc,
	std::unique_ptr<char[]>& key_bytes, size_t* pkey_len,
	std::unique_ptr<char[]>& value_bytes, size_t* pvalue_len) {

	char len_bytes[8] = {0};
	uint64_t actual_key_len;
	uint64_t actual_value_len;

	ifs.seekg(loc, ios::beg);
	ifs.read(len_bytes, 8);
	actual_key_len = Util::Decode64Long(len_bytes);
	*pkey_len = actual_key_len;
	if (actual_key_len > Util::MAX_KEY_LEN) {
		cout << "Invalid key len " << actual_key_len << " at offset " << loc << endl;
		return Status::Corruption;
	}

	char* ubuf = new char[actual_key_len + 8](); // load key bytes and value len in single IO
	if (ubuf == nullptr) {
		cout << "Failed to allocate memory for key len " << actual_key_len << endl;
		return Status::OutOfMemory;
	}
	key_bytes.reset(ubuf);
	ifs.read(key_bytes.get(), actual_key_len + 8);
	if (ifs.fail()) {
		return Status::IOError;
	}
	actual_value_len = Util::Decode64Long(key_bytes.get() + actual_key_len);
	*pvalue_len = actual_value_len;
	if (actual_value_len > Util::MAX_VALUE_LEN) {
		cout << "Invalid value len " << actual_value_len << " at offset " << loc << endl;
		return Status::Corruption;
	}
	ubuf = new char[actual_value_len]();
	if (ubuf == nullptr) {
		cout << "Failed to allocate memory for value len " << actual_value_len << endl;
		return Status::OutOfMemory;
	}
	value_bytes.reset(ubuf);
	ifs.read(value_bytes.get(), actual_value_len);

	return ifs.fail() ? Status::IOError : Status::Ok;
}

/**
 * readKeyOnly - read key only from ifs
 * @param ifs
 * @param loc
 * @param key_bytes
 * @param pkey_len
 * @param pvalue_len
 * @return
 * Note :
 * Call this with file open already.
 */
Status FileStore::readKeyOnly(ifstream& ifs,
	Location loc,
	std::unique_ptr<char[]>& key_bytes, size_t* pkey_len,
	size_t* pvalue_len) {

	char len_bytes[8] = { 0 };
	uint64_t actual_key_len;
	uint64_t actual_value_len;

	ifs.seekg(loc, ios::beg);
	ifs.read(len_bytes, 8);
	actual_key_len = Util::Decode64Long(len_bytes);
	*pkey_len = actual_key_len;
	if (actual_key_len > Util::MAX_KEY_LEN) {
		cout << "Invalid key len " << actual_key_len << " at offset " << loc << endl;
		return Status::Corruption;
	}

	key_bytes.reset(new char[actual_key_len + 8]); // load key bytes and value len in single IO
	ifs.read(key_bytes.get(), actual_key_len + 8);
	if (ifs.fail()) {
		return Status::IOError;
	}

	actual_value_len = Util::Decode64Long(key_bytes.get() + actual_key_len);
	*pvalue_len = (size_t)actual_value_len;
	if (actual_value_len > Util::MAX_VALUE_LEN) {
		cout << "Invalid value len " << actual_value_len << " at offset " << loc << endl;
		return Status::Corruption;
	}

	return ifs.fail() ? Status::IOError : Status::Ok;
}

// loc : offset where key is stored
Status FileStore::get(const Key& key, const Location loc, Value* value) {
    Status s;
	std::unique_ptr<char[]> key_bytes;
    std::unique_ptr<char[]> value_bytes;
	size_t key_len;
	size_t value_len;
	assert (loc != INVALID_LOC);

	ifstream ifs(this->mDataFileName, ios::in | ios::binary);
	if (!ifs.is_open()) {
		cout << "Unable to open file " << this->mDataFileName << endl;
		return Status::IOError;
	}

	if ((s = readKVPair(ifs, loc, key_bytes, &key_len, value_bytes, &value_len)) != Status::Ok) {
		ifs.close();
		return s;
	}
	if (key.compare(key_bytes.get(), key_len) != 0) {
		ifs.close();
		cout << "Corrupted key at location " << loc << " in file " << this->mDataFileName << endl;
		return Status::Corruption;
	}

	Slice slice(value_bytes.get(), value_len);
	value->PinSelf(slice);

	return Status::Ok;
}

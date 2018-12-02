#include <iostream>
#include "IndexStore.h"
#include "Util.h"

using namespace std;

// public
IndexStore::IndexStore(const char* indexRoot) {
	this->mIndexRootPath = indexRoot;
	this->mCache = new ConcurrentCache(1024 * 1024 * 1024); // 1 GB
	cout <<"Index Root Path " << mIndexRootPath << endl;

}
IndexStore::~IndexStore() {
	delete this->mCache;
}
Location IndexStore::GetIndex(const Key& key) {
    // check if hit in cache
    Location loc = mCache->Get(key);
    if (loc != INVALID_LOC) {
        // cout << "[Debug] Index cache hit for " << key.ToString() << endl;
        return loc;
    }
    //TODO : Use BloomFilter to check if index is not there

	string fileFullPathName = makePath(key);
	if (!Util::DoesFileExist(fileFullPathName)) {
		cout << "Index File "<< fileFullPathName << " for key " << key.ToString() << " does not exist" << endl;
		return INVALID_LOC;
	}
	loc = getIndex(fileFullPathName, key);
	// update cache
	if (loc != INVALID_LOC) {
		mCache->Put(key, loc);
	}
	return loc;
}
bool IndexStore::UpdateIndex(const Key& key, const Location loc) {
    assert (loc != INVALID_LOC);
	string filePathName = makePath(key);

	size_t pos = filePathName.find_last_of('/');
	if (!Util::MakePathIfNotExist(filePathName.substr(0, pos))) {
		cout << "Unable to create meta file " << filePathName << endl;
		return false;
	}
	putIndex(filePathName, key, loc);

	// cout<<"[DEBUG] Update index for key " << key.ToString() << ", key size " << key.size() << ", with loc " << loc << " @"<< filePathName << endl;
	return true;
}


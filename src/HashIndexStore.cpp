#include <iostream>
#include <fstream>
#include <sstream>
#include <stdarg.h>
#include <memory>
#include <cstdio>
#include "../include/IndexStore.h"
#include "../include/JenkinsHash.h"
#include "../include/Util.h"

const string HashIndexStore::PREFIX1 = "LVL1_";
const string HashIndexStore::PREFIX2 = "LVL2_";
const string HashIndexStore::PREFIX3 = "IDX__";

HashIndexStore::HashIndexStore(const char* indexRoot) : IndexStore(indexRoot) {}
HashIndexStore::~HashIndexStore() {}

// **************************************************************************************
// private
// TODO : Use memory map to read file
// TODO : Iterator on KV pairs.
Location HashIndexStore::getIndex(const string& filename, const Key& key) {
    unique_ptr<char[]> key_raw_bytes(new char[Util::MAX_KEY_LEN + 8]); // plus Location
    char key_len_bytes[8];
    uint64_t actual_key_len;
    uint64_t offset = 0;
    uint64_t total_len;
    ifstream ifs(filename, ios::in | ios::binary | ios::ate);
    if (!ifs.is_open()) {
        cout << "Unable to open index file " << filename << endl;
        return INVALID_LOC;
    }
    total_len = static_cast<uint64_t>(ifs.tellg());

    //assert (total_len < 1 * 1024 * 1024 * 1024);

    while (offset < total_len) {
        ifs.seekg(offset, ios::beg);
        ifs.read(key_len_bytes, 8);
        actual_key_len = Util::Decode64Long(key_len_bytes);
        if (actual_key_len > Util::MAX_KEY_LEN) {
            cout << "Invalid key len " << actual_key_len << " at offset " << offset << endl;
            ifs.close();
            return INVALID_LOC;
        }
        ifs.read(key_raw_bytes.get(), actual_key_len + 8); // load key bytes and location_size together.
        // cout << "[DEBUG] " << "actual key len = " << actual_key_len  << " @" << filename << endl;
        if (key.compare(key_raw_bytes.get(), actual_key_len) == 0) {
            ifs.close();
            return Util::Decode64Long(key_raw_bytes.get() + actual_key_len);
        }

        offset += actual_key_len + 16;
    }
    ifs.close();
    return INVALID_LOC;
}

void HashIndexStore::putIndex(const string& filename, const Key& key, const Location loc) {
    char len_bytes[8];
    ofstream ofs(filename, ios::out | ios::binary | ios::ate);
    ofs.write(Util::Encode64Long((unsigned long long)(key.size()), len_bytes), 8);
    ofs.write(key.data(), key.size());
    ofs.write(Util::Encode64Long((unsigned long long)loc, len_bytes), 8);
    ofs.close();
    // cout <<"[DEBUG] " << "Append key " << key.ToString() << " size = " << key.size() << " loc = " << loc << " @" << filename << endl;
}

string HashIndexStore::makePath(const Key& key) {
    long long hash1 = hash(key, 0);
    long long hash2 = hash(key, 1);
    long long hash3 = hash(key, 2);
    ostringstream stringStream;
    stringStream << mIndexRootPath << "/" << PREFIX1 << hash1 % BucketSize << "/"
                 << PREFIX2 << hash2 % BucketSize << "/"
                 << PREFIX3 << hash3 % BucketSize << ".meta";
    return stringStream.str();
    //return string_format("%s/%08d/%08d/%08d.meta", indexRootPath,
    //	hash1 % BucketSize, hash2 % BucketSize, hash3 % BucketSize);
}


long long HashIndexStore::hash(const Key& k, char hashType) {
    if (hashType == 0) {
        return JenkinsHash::hash(k.data(), k.size(), 0);
    }
    else if (hashType == 1) {
        // TODO : use different hash function
        return JenkinsHash::hash(k.data(), k.size(), 119);
    }
    else {
        // TODO : use different hash function
        return JenkinsHash::hash(k.data(), k.size(), 100000);
    }
}

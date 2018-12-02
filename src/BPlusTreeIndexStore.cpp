#include "IndexStore.h"

// Just a B++Tree PLACEHOLDER, NOT IMPLEMENTED YET.
// HashIndexStore is actively used instead of this.

BPlusTreeIndexStore::BPlusTreeIndexStore(const char* indexRoot) : IndexStore(indexRoot) {}
BPlusTreeIndexStore::~BPlusTreeIndexStore() {}

Location BPlusTreeIndexStore::getIndex(const string& filename, const Key& key) {
    return INVALID_LOC;
}

void BPlusTreeIndexStore::putIndex(const string& filename, const Key& key, const Location loc) {

}
string BPlusTreeIndexStore::makePath(const Key& key) {
    return "";
}

#pragma once
#include <string>
#include "Cache.h"

using namespace std;
class IndexStore {
protected:
	string mIndexRootPath;
	ConcurrentCache* mCache;

public:
	explicit IndexStore(const char* indexRoot);
	virtual ~IndexStore();
	Location GetIndex(const Key& key);
	bool UpdateIndex(const Key& key, const Location loc);

private:
	virtual Location getIndex(const string& filename, const Key& key) = 0;
	virtual void putIndex(const string& filename, const Key& key, const Location loc) = 0;
    virtual string makePath(const Key& key) = 0;

};

class HashIndexStore : public IndexStore {
public:
    explicit HashIndexStore(const char* indexRoot);
    virtual ~HashIndexStore();
private:
    static constexpr int BucketSize = 4096;
    static const string PREFIX1;
    static const string PREFIX2;
    static const string PREFIX3;

    static long long hash(const Key& key, char hashType);

    Location getIndex(const string& filename, const Key& key) override;
    void putIndex(const string& filename, const Key& key, const Location loc) override;
    string makePath(const Key& key) override;
};

class BPlusTreeIndexStore : public IndexStore {
public:
	explicit BPlusTreeIndexStore(const char* indexRoot);
	virtual ~BPlusTreeIndexStore();
private:
	Location getIndex(const string& filename, const Key& key) override;
	void putIndex(const string& filename, const Key& key, const Location loc) override;
	string makePath(const Key& key) override;
};

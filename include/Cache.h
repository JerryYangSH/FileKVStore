#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "Types.h"

using list_item_t = std::pair<std::weak_ptr<std::string>, Location>; // Use string pointer to save string copy.
using list_t = std::list<list_item_t>;
using ckey_t = std::shared_ptr<std::string>;

struct KeyHasher
{
    std::size_t operator()(const ckey_t& k) const {
        size_t hash = 0;
        size_t totalLen = k->size();
        size_t remainByteLen = totalLen % sizeof(int64_t);
        size_t remainLongLen = totalLen - remainByteLen;
        const int64_t* pLongs = reinterpret_cast<const int64_t *> (k->data());
        for (int i = 0; i < remainLongLen; i+= sizeof(int64_t)) {
            hash += *pLongs++;
            hash *= 31;
        }

        const char* pBytes = reinterpret_cast<const char*>(pLongs);
        while (remainByteLen-- > 0) {
            hash += *pBytes++;
        }
        return hash;
    }
};
struct KeyEqualer
{
    bool operator()(const ckey_t& k1, const ckey_t& k2) const {
        if (k1->size() != k2->size()) {
            return false;
        }
        for (auto i = 0; i < k1->size(); i++) {
            if ((*k1)[i] != (*k2)[i]) {
                return false;
            }
        }
        return true;
    }
};

using map_t = std::unordered_map<ckey_t, list_t::iterator, KeyHasher, KeyEqualer>;

// LRU Cache
class Cache {
private:
	long capacity; // memory limit in bytes
	long used;
	list_t list; // sorted list by access order, oldest in front.
	map_t map;
    std::mutex mtx;

	volatile bool scan_ongoing;
	// sync request to evict stale cache entries when capacity limit is hit.
	void scanEvict();

	struct Statistic {
	    long hit = 0;
	    long total = 0;
	};
	Statistic statistic;

public:
	explicit Cache(int _capacity);
	~Cache();
	void Put(const Key& key, Location loc);
	Location Get(const Key& key);
	bool Contains(const Key& key) const;
	long Usage();
    long Size();
	inline int Hit() { return statistic.hit; }
    inline int Total() { return statistic.total; }
};

/**
 * Class ConcurrentCache
 * Support Concurrent access on Cache by sharding segment
 * The request is sharded into different bucket for concurrent access.
 */
class ConcurrentCache {
private:
    constexpr static int CONCURRENT_SEGMENT = 64;
    constexpr static int MIN_CAPACITY = 1024 * 1024;
    long total_capacity;
    std::unique_ptr<Cache> cachesPtr[CONCURRENT_SEGMENT];
    int locateSegment(const Key& key) const;
public:
    ConcurrentCache(long _capacity);
    ~ConcurrentCache();
    void Put(const Key& key, Location loc);
    Location Get(const Key& key);
    bool Contains(const Key& key) const;
    long Usage() const; // how much memory is consumed in bytes
    long Size() const;  // how many entries of key.
    int Hit() const;   // cache hit
    int Total() const; // cache hit + cache miss
};
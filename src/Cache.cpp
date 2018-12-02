#include <iostream>
#include <unordered_set>
#include "Cache.h"
#include "JenkinsHash.h"

Cache::Cache(int _capacity) : capacity(_capacity), used(0), scan_ongoing(false) {
}
Cache::~Cache() {
    /*
    if (statistic.total != 0) {
        std::cout << " Cache hit " << statistic.hit << " out of " << statistic.total
        << ", hitRate("<< statistic.hit * 100 / statistic.total << "%)"<< std::endl;
    }
    */
}
/** Put call is exclusive call.
 * a) If cache hit, and then move it to tail (latest). (delete old and append new)
 * b) else if not hit, a new item is added at tail. And check if over capacity, so schedule a request to evict the old keys.
 */
void Cache::Put(const Key& key, Location loc) {
    std::lock_guard<std::mutex> l(mtx);
    std::string skey(key.data(), key.size());
    ckey_t sp_key = std::make_shared<std::string>(std::move(skey));
    auto it_m = map.find(sp_key);
    if (it_m != map.end()) {
        auto it_l = map[sp_key];
        list.erase(it_l);
        map.erase(it_m);
    }
    else {
        used += key.size() + sizeof(Location);
        if (used > capacity) {
            scanEvict();
        }
    }
    list.push_back(std::make_pair(sp_key, loc));
    map[sp_key] = --list.end();

    // std::cout << "[Cache] Put [" << *sp_key << "," << loc  << "] in cache " << std::endl;
}

/** Get call is thread-safe.
 * Since GET will be likely to change the map and list, so exclusive lock is required.
 * [TODO]: Use fine grained lock to make read-read friendly.
 */
 Location Cache::Get(const Key& key) {
    std::lock_guard<std::mutex> l(mtx);
    Location loc = INVALID_LOC;
    std::string skey(key.data(), key.size());
    ckey_t sp_key = std::make_shared<std::string>(std::move(skey));

    auto it_m = map.find(sp_key);
	if (it_m != map.end()) {
	    auto it_l = map[sp_key];
	    loc = it_l->second;
	    list.splice(list.end(), list, it_l);
        map[sp_key] = --list.end();

        statistic.hit++;
        //std::cout << "[Cache] Hit cache [" << *sp_key << "," << loc  << "] from cache , cache size =" << map.size() << " used=" << used << std::endl;
	}
	statistic.total++;

	return loc;
}

/** scanEvict - Scan and Evict stale items.
 * The evciting scanner could be async that scanner and clean could be done in background.
 * But here we use sync for simplicity.
 */
void Cache::scanEvict() {
    if (scan_ongoing) {
        return;
    }
    if (used <= capacity) {
        return;
    }
    //std::cout << "[Cache] Start evicting stale items from cache. Memory consumed " << used << " over " << capacity << std::endl;
    scan_ongoing = true;
    long targetSize = used;
    std::unordered_set<ckey_t, KeyHasher, KeyEqualer> removalKeys;
    for (auto it_l = list.begin(); it_l != list.end(); it_l++) {
        if (targetSize < capacity * 0.9) {
            break;
        }
        targetSize -= it_l->first.lock()->size() + sizeof(Location);
        removalKeys.insert(it_l->first.lock());
    }

    for (auto it_m = removalKeys.begin(); it_m != removalKeys.end(); it_m++) {
        auto it_l = map[*it_m];
        map.erase(*it_m);
        list.erase(it_l);
        used -= it_l->first.lock()->size() + sizeof(Location);
    }

    scan_ongoing = false;
}

bool Cache::Contains(const Key& key) const {
    std::string skey(key.data(), key.size());
    ckey_t sp_key = std::make_shared<std::string>(std::move(skey));
    return (map.find(sp_key) != map.end());
}

long Cache::Usage() {
    return used;
}

long Cache::Size() {
    return map.size();
}

/**
 * class ConcurrentCache
 */
ConcurrentCache::ConcurrentCache(long _capacity) : total_capacity(_capacity) {
    long capacity = _capacity / CONCURRENT_SEGMENT;
    capacity = capacity < MIN_CAPACITY ? MIN_CAPACITY : capacity;
    for (int i = 0; i < CONCURRENT_SEGMENT; i++) {
        cachesPtr[i].reset(new Cache(capacity));
    }
}
ConcurrentCache::~ConcurrentCache(){
    // Uncomment below for debug purpose
    /*
    long hits = 0;
    long totals = 0;
    for (int i = 0; i < CONCURRENT_SEGMENT; i++) {
        hits += cachesPtr[i]->Hit();
        totals += cachesPtr[i]->Total();
    }
    if (totals != 0) {
        std::cout << " Total Cache Hit " << hits << " out of " << totals
                  << ", hitRate("<< hits * 100 / totals << "%)"<< std::endl;
    }
    */
}

int ConcurrentCache::locateSegment(const Key& key) const {
    return static_cast<int>(JenkinsHash::hash(key.data(), static_cast<int>(key.size()), 71) % CONCURRENT_SEGMENT);
}

// public
void ConcurrentCache::Put(const Key& key, Location loc) {
    int id = locateSegment(key);
    cachesPtr[id]->Put(key, loc);
}
Location ConcurrentCache::Get(const Key& key) {
    int id = locateSegment(key);
    return cachesPtr[id]->Get(key);
}
bool ConcurrentCache::Contains(const Key& key) const {
    int id = locateSegment(key);
    return cachesPtr[id]->Contains(key);
}
long ConcurrentCache::Usage() const {
    long sum = 0;
    for (int i = 0; i < CONCURRENT_SEGMENT; i++) {
        sum += cachesPtr[i]->Usage();
    }
    return sum;
}
long ConcurrentCache::Size() const {
    long sum = 0;
    for (int i = 0; i < CONCURRENT_SEGMENT; i++) {
        sum += cachesPtr[i]->Size();
    }
    return sum;
}
int ConcurrentCache::Hit() const {
    long sum = 0;
    for (int i = 0; i < CONCURRENT_SEGMENT; i++) {
        sum += cachesPtr[i]->Hit();
    }
    return sum;
}
int ConcurrentCache::Total() const {
    long sum = 0;
    for (int i = 0; i < CONCURRENT_SEGMENT; i++) {
        sum += cachesPtr[i]->Total();
    }
    return sum;
}

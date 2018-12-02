#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "Cache.h"

TEST(cacheTest, simple) {
	std::unique_ptr<Cache> pCache(new Cache(128));
	Key key("k1234567890");
	pCache->Put(key, -1);
	pCache->Put(key, 0);
	ASSERT_TRUE(pCache->Contains(key));
	ASSERT_EQ(1, pCache->Size());
	ASSERT_EQ(key.size() + sizeof(Location), pCache->Usage());
	ASSERT_TRUE(pCache->Contains(key));
	ASSERT_EQ(0, pCache->Get(key));
}

TEST(cacheTest, evict) {
	constexpr long kNum = 5;
    std::unique_ptr<Cache> pCache(new Cache((16 + 8) * kNum)); // cache only recent kNum entries
    std::vector<Key> keys;
    for (int i = 0; i < kNum; i++) {
    	Key key("0123456789ABCDE" + std::to_string(i)); // 16 bytes
    	keys.push_back(key);
    	pCache->Put(key, i);

    	if (i > 0) {
    		ASSERT_TRUE(pCache->Contains(keys[i-1]));
    	}
    }

    ASSERT_EQ(kNum, keys.size());
    // all keys are in the cache.
    ASSERT_TRUE(pCache->Contains(keys[0]));
    ASSERT_TRUE(pCache->Contains(keys[kNum-1]));

    // put one more, and trigger evict
    pCache->Put(Key("0123456789ABCDE" + std::to_string(kNum)), kNum);

    ASSERT_FALSE(pCache->Contains(keys[0])); // the oldest key is evicted out.
    ASSERT_FALSE(pCache->Contains(keys[1]));
    ASSERT_TRUE(pCache->Contains(keys[2]));
}


TEST(cacheTest, concurrency) {
    std::unique_ptr<ConcurrentCache> pCache(new ConcurrentCache(10 * 1024 * 1024));
    std::vector<Key> keys;
    for (int i = 0; i < 10; i++) {
        Key key("0123456789ABCDE" + std::to_string(i)); // 16 bytes
        keys.push_back(key);
        pCache->Put(key, i);
    }
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(pCache->Contains(keys[i]));
        ASSERT_EQ(i, pCache->Get(keys[i]));
    }

}

int main(int argc, char *argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

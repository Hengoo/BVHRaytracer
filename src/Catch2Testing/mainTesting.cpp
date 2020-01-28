#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch2/catch.hpp"

#include   "../cacheSimulator.h"
#include <algorithm>
#include <random>
#include <omp.h>//openMp

//smallest thing to add to cache is  32*64, otherwise it hits the default initialized 0

int cacheTestNoHitSequential(CacheSimulator cache, int loadCount)
{
	for (uint32_t i = 32; i < loadCount; i++)
	{
		if (i % 2 == 0)
			cache.loadHeap(i * 64);
		else
			cache.loadStack(i * 64);
	}
	return cache.getThisThreadHits();
}

int cacheFixedHits(CacheSimulator cache, int totalLoads, int dupLoads)
{
	int counter = 0;
	for (uint32_t i = 32; i < totalLoads; i++)
	{
		cache.loadStack(i * 64);
		if (i > 10)
		{
			if (counter < dupLoads)
			{
				cache.loadHeap((i - 8) * 64);
				counter++;
			}
		}
	}
	return cache.getThisThreadHits();
}

bool cacheTestStackHeapCounters(CacheSimulator cache, int totalLoads, int dupLoads)
{
	int counter = 0;
	for (uint32_t i = 32; i < totalLoads; i++)
	{
		cache.loadStack(i * 64);
		if (i > 10)
		{
			if (counter < dupLoads)
			{
				cache.loadStack((i - 8) * 64);
				counter++;
			}
		}
	}
	int tId = omp_get_thread_num();
	int zeroSum = cache.heapCacheHits[tId];
	int sum = cache.stackCacheHits[tId];
	cache.resetThisThread();

	counter = 0;
	for (uint32_t i = 32; i < totalLoads; i++)
	{
		cache.loadHeap(i * 64);
		if (i > 10)
		{
			if (counter < dupLoads)
			{
				cache.loadHeap((i - 8) * 64);
				counter++;
			}
		}
	}
	zeroSum += cache.stackCacheHits[tId];
	sum += cache.heapCacheHits[tId];
	return zeroSum == 0 && sum == dupLoads * 2;
}

int cacheTestNoHitRandomOrder(CacheSimulator cache, int loadCount)
{
	//shuffle vector:
	std::vector<int> input(loadCount);
	std::iota(input.begin(), input.end(), 32);
	auto rng = std::default_random_engine{ 0 };
	std::shuffle(input.begin(), input.end(), rng);

	for (auto& i : input)
	{
		cache.loadHeap(i * 64);
		if (cache.getAllHeapHits() != 0)
		{
			std::cout << i << std::endl;
		}
	}
	return cache.getThisThreadHits();
}

int cacheCheckCachelinesZero(CacheSimulator cache)
{
	int sum = 0;
	for (auto& c : cache.cache)
	{
		for (auto& s : c.storage)
		{
			sum = std::accumulate(s.cacheLines.begin(), s.cacheLines.end(), 0);
		}
	}
	return sum;
}

int cacheResetTest(int cacheSize, int loadCount)
{
	CacheSimulator cache(cacheSize);
	int hit = 0;
	hit += cacheTestNoHitRandomOrder(cache, loadCount);
	cache.resetEverything();
	hit += cacheTestNoHitRandomOrder(cache, loadCount);

	cache.resetThisThread();
	hit += cacheTestNoHitRandomOrder(cache, loadCount);
	return hit;
}

int parallelCacheTest(CacheSimulator cache, int loadCount)
{
	int threadCount = omp_get_max_threads();
	std::vector<int> hits(threadCount, 0);
#pragma omp parallel
	for (int i = 0; i < threadCount; i++)
	{
		int tId = omp_get_thread_num();
		hits[tId] = cacheTestNoHitRandomOrder(cache, loadCount);
	}
	return std::accumulate(hits.begin(), hits.end(), 0);
}

int parallelCacheClearTest1(CacheSimulator cache, int loadCount)
{
	int threadCount = omp_get_max_threads();
	std::vector<int> hits(threadCount, 0);
#pragma omp parallel
	for (int i = 0; i < threadCount; i++)
	{
		int tId = omp_get_thread_num();
		hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
		cache.resetThisThread();
		hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
		cache.resetThisThread();
		hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
		cache.resetThisThread();
		hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
		cache.resetThisThread();
		hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
	}
	return std::accumulate(hits.begin(), hits.end(), 0);
}

int parallelCacheClearTest2(CacheSimulator cache, int loadCount, int loadHits)
{
	int threadCount = omp_get_max_threads();
	std::vector<int> hits(threadCount, 0);
#pragma omp parallel
	for (int i = 0; i < threadCount; i++)
	{
		if (i % 2 == 0)
		{
			int tId = omp_get_thread_num();
			hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
			cache.resetThisThread();
			hits[tId] += cacheFixedHits(cache, loadCount, loadHits);
			cache.resetThisThread();
			hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
			cache.resetThisThread();
			hits[tId] += cacheFixedHits(cache, loadCount, loadHits);
			cache.resetThisThread();
			hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
			cache.resetThisThread();
			hits[tId] += cacheFixedHits(cache, loadCount, loadHits);
		}
		else
		{
			int tId = omp_get_thread_num();

			hits[tId] += cacheFixedHits(cache, loadCount, loadHits);
			cache.resetThisThread();
			hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
			cache.resetThisThread();
			hits[tId] += cacheFixedHits(cache, loadCount, loadHits);
			cache.resetThisThread();
			hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);
			cache.resetThisThread();
			hits[tId] += cacheFixedHits(cache, loadCount, loadHits);
			cache.resetThisThread();
			hits[tId] += cacheTestNoHitRandomOrder(cache, loadCount);

		}
	}
	return std::accumulate(hits.begin(), hits.end(), 0);
}

int parallelStackHeapTest(CacheSimulator cache, int loadCount, int loadHits)
{
	int threadCount = omp_get_max_threads();
	std::vector<bool> resultArray(threadCount, true);
#pragma omp parallel
	for (int i = 0; i < threadCount; i++)
	{
		int tId = omp_get_thread_num();
		resultArray[tId] = resultArray[tId] && cacheTestStackHeapCounters(cache, loadCount, loadHits);
		cache.resetThisThread();
	}

	bool result = true;
	for (auto b : resultArray)
	{
		result = result && b;
	}
	return result;
}


TEST_CASE("cache test 1", "[ForWhatIsThis?]")
{
	int cacheSize = 256;
	int loadCount = 1111;
	SECTION("testing cache initialization")
	{
		CacheSimulator cache(cacheSize);
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);
		//random fills:
		cacheTestNoHitSequential(cache, loadCount);
		cacheTestNoHitRandomOrder(cache, loadCount);

		cache = CacheSimulator(cacheSize);
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);
	}

	SECTION("testing cache no hit fills")
	{
		CacheSimulator cache(cacheSize);
		REQUIRE(cacheTestNoHitSequential(cache, loadCount) == 0);
		cache.resetEverything();
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);
		REQUIRE(cacheTestNoHitRandomOrder(cache, loadCount) == 0);
		cache.resetEverything();
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);
	}

	SECTION("testing cache fixed hits")
	{
		CacheSimulator cache(cacheSize);
		REQUIRE(cacheFixedHits(cache, 150, 15) == 15);
		cache.resetEverything();
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);

		REQUIRE(cacheFixedHits(cache, 1500, 150) == 150);
		cache.resetEverything();
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);

		REQUIRE(cacheTestStackHeapCounters(cache, 1000, 900) == true);
	}

	SECTION("testing cache reset")
	{
		REQUIRE(cacheResetTest(cacheSize, loadCount) == 0);
	}

	SECTION("testing multi threading fills no hits")
	{
		CacheSimulator cache(cacheSize);
		REQUIRE(parallelCacheTest(cache, loadCount) == 0);
		cache.resetEverything();
		REQUIRE(cacheCheckCachelinesZero(cache) == 0);
	}

	SECTION("testing multi threading clears")
	{
		CacheSimulator cache(cacheSize);
		REQUIRE(parallelCacheClearTest1(cache, loadCount * 20) == 0);

		int threadCount = omp_get_max_threads();
		REQUIRE(parallelCacheClearTest2(cache, 3220, 3022) == 3022 * 3 * threadCount);

		REQUIRE(parallelStackHeapTest(cache, 3220, 2900) == true);
	}



}
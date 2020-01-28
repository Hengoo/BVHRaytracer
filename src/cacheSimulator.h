#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <bitset>
#include <array>
#include <omp.h>
#include <numeric>

//This shouldnt change but not 100% sure
#define pointerType uint32_t

//number of cachelines: for the ryzen threadripper 1950x this is 524,288B what corresponds to 
//256 cachelines with each 64B for each of the 32 threads
//(in theory it would have 512 cachelines that is shared between 2 threads each)


//Hardware cache structure:

//cacheline content: tag | data block
//- tag is basically that part of the address we dont know from the location of this cacheline.
//- datablock is the data of the cacheline (64 byte)

//cacheline location calculation:  tag | index | block offset
//-block offset is 6 -> log2(64). Original address Bit shift of 6 gives us id to the cacheline
//		bitshift by 6 is equivalent to division by 64
//-index is the index from the 8 way associative set
//-tag is what is left.

//since we have 512 cachelines per core we get 64 8 big sets

//to determine the location inside the set: plru. For 8 elements we need a 7 bit large tree


//This implementation is just to calculate hits and misses, so we only store the tag
// and also inlcude the index part in the tag to keep everything simple

class Plru
{
	//contains 8 cachelines with a plru replacement strategy.
	//the tree needs 7 bit

	//in order to have it easier i just do it like this:
	bool level1;
	std::array<bool, 2> level2;
	std::array<bool, 4> level3;

	void updateTree(int cacheId)
	{
		level1 = !((cacheId & 4) >> 2);
		level2[((cacheId & 4) >> 2)] = !((cacheId & 2) >> 1);
		level3[((cacheId & 4) >> 1) | ((cacheId & 2) >> 1)] = !(cacheId & 1);
	}
	int getEvictionId()
	{
		int result = level1 << 2;
		result |= level2[level1] << 1;
		result |= level3[level1 << 1 | level2[level1]];
		return result;
	}

public:
	std::array<uint32_t, 8> cacheLines;
	Plru()
	{
		//initialize the tree with false
		level1 = false;
		level2[0] = false;
		level2[1] = false;
		std::fill(level3.begin(), level3.end(), false);
		cacheLines.fill(0);
	}

	//Updates cache and returns true if hit
	bool load(pointerType address)
	{
		//check hit / miss
		auto res = std::find(cacheLines.begin(), cacheLines.end(), address);
		if (res != cacheLines.end())
		{
			//hit: update bits
			int id = std::distance(cacheLines.begin(), res);
			updateTree(id);
			return true;
		}
		else
		{
			//miss: update cacheline and bits
			int evictId = getEvictionId();
			cacheLines[evictId] = address;
			updateTree(evictId);
			return false;
		}
	}
};

class Cache8WaySet
{
	//contains #cacheline / 8 PLRU s
	int bitsForIndex;
	int cacheSize;
	pointerType mask;

public:
	std::vector<Plru> storage;
	Cache8WaySet(int cacheSize)
		:cacheSize(cacheSize)
	{
		storage.resize(cacheSize / 8);
		//From the log2(#Plru) bits put in correct Plru
		bitsForIndex = log2(cacheSize / 8);
		mask = ((1 << bitsForIndex) - 1);
	}

	bool load(pointerType address)
	{
		///std::cerr << "load" << address;
		return storage[address & mask].load(address >> bitsForIndex);
	}

	void reset()
	{
		std::fill(storage.begin(), storage.end(), Plru());
	}
};



class CacheSimulator
{
	int threadCount;


	//for each thread: storage for cacheSize many cachelines
	//std::vector<std::stack<pointerType>> cache;
	//one thread per cache, but would be nice if it also supports two threads per cache
public:
	int cacheSize;
	std::vector<Cache8WaySet>cache;
	std::vector<uint64_t> stackCacheLoads;
	std::vector<uint64_t> stackCacheHits;
	std::vector<uint64_t> heapCacheLoads;
	std::vector<uint64_t> heapCacheHits;


	CacheSimulator(int cacheSize = 256)
		:cacheSize(cacheSize)
	{
		if (cacheSize != 0)
		{
			threadCount = omp_get_max_threads();
			cache.resize(threadCount, Cache8WaySet(cacheSize));
			stackCacheLoads.resize(threadCount);
			stackCacheHits.resize(threadCount);
			heapCacheLoads.resize(threadCount);
			heapCacheHits.resize(threadCount);
		}
	}

	inline uint64_t getAllStackHits()
	{
		return std::accumulate(stackCacheHits.begin(), stackCacheHits.end(), 0ULL);
	}

	inline uint64_t getAllHeapHits()
	{
		return std::accumulate(heapCacheHits.begin(), heapCacheHits.end(), 0ULL);
	}

	inline uint64_t getAllStackLoads()
	{
		return std::accumulate(stackCacheLoads.begin(), stackCacheLoads.end(), 0ULL);
	}

	inline uint64_t getAllHeapLoads()
	{
		return std::accumulate(heapCacheLoads.begin(), heapCacheLoads.end(), 0ULL);
	}

	inline uint64_t getThisThreadHits()
	{
		int tId = omp_get_thread_num();
		return stackCacheHits[tId] + heapCacheHits[tId];
	}

	inline uint64_t getThisThreadStackLoads()
	{
		int tId = omp_get_thread_num();
		return stackCacheLoads[tId] + heapCacheLoads[tId];
	}

	//simultes cache load.
	inline void loadStack(void* pointer)
	{
		//the method should be optimized away when doDacheSimulator is not defined.
		//cast pointer into type we can do math with
		pointerType p = (pointerType)pointer;
		//ignore the first 6 bits to get cache line Id
		pointerType cacheId = p >> 6;

		int tId = omp_get_thread_num();
		stackCacheLoads[tId]++;
		stackCacheHits[tId] += cache[tId].load(cacheId);
	}
	inline void loadHeap(void* pointer)
	{
		//the method should be optimized away when doDacheSimulator is not defined.
		//cast pointer into type we can do math with
		pointerType p = (pointerType)pointer;
		//ignore the first 6 bits to get cache line Id
		pointerType cacheId = p >> 6;

		int tId = omp_get_thread_num();
		heapCacheLoads[tId]++;
		heapCacheHits[tId] += cache[tId].load(cacheId);
	}

	//simultes cache load.
	inline void loadStack(pointerType p)
	{
		//ignore the first 6 bits to get cache line Id
		pointerType cacheId = p >> 6;

		int tId = omp_get_thread_num();
		stackCacheLoads[tId]++;
		stackCacheHits[tId] += cache[tId].load(cacheId);
	}
	inline void loadHeap(pointerType p)
	{
		//std::cout << p % 64 << std::endl;
		//ignore the first 6 bits to get cache line Id
		pointerType cacheId = p >> 6;

		int tId = omp_get_thread_num();
		heapCacheLoads[tId]++;
		heapCacheHits[tId] += cache[tId].load(cacheId);
	}

	inline void resetAllCounter()
	{
		std::fill(stackCacheLoads.begin(), stackCacheLoads.end(), 0);
		std::fill(stackCacheHits.begin(), stackCacheHits.end(), 0);
		std::fill(heapCacheLoads.begin(), heapCacheLoads.end(), 0);
		std::fill(heapCacheHits.begin(), heapCacheHits.end(), 0);
	}

	inline void resetCounter(int tId)
	{
		stackCacheLoads[tId] = 0;
		stackCacheHits[tId] = 0;
		heapCacheLoads[tId] = 0;
		heapCacheHits[tId] = 0;
	}

	inline void resetThisThreadCounter()
	{
		int tId = omp_get_thread_num();
		resetCounter(tId);
	}

	//resets counters and cache
	inline void resetThisThread()
	{
		int tId = omp_get_thread_num();
		cache[tId].reset();
		resetCounter(tId);
	}

	//resets counters and cache
	inline void resetEverything()
	{
		std::fill(cache.begin(), cache.end(), Cache8WaySet(cacheSize));
		resetAllCounter();
	}
};
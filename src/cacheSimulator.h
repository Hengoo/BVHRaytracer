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

	std::array<uint32_t, 8> cacheLines;

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
	Plru()
	{
		//initialize the tree with false
		level1 = false;
		level2[0] = false;
		level2[1] = false;
		std::fill(level3.begin(), level3.end(), false);
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

	std::vector<Plru> storage;
	int bitsForIndex;
	int cacheSize;
	pointerType mask;

public:
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

	std::vector<Cache8WaySet>cache;

	//for each thread: storage for cacheSize many cachelines
	//std::vector<std::stack<pointerType>> cache;
	//one thread per cache, but would be nice if it also supports two threads per cache
public:
	int cacheSize;
	std::vector<uint64_t> cacheLoads;
	std::vector<uint64_t> cacheHits;

	CacheSimulator(int cacheSize = 256)
		:cacheSize(cacheSize)
	{

		threadCount = omp_get_max_threads();
		cache.reserve(threadCount);
		cache.resize(threadCount, Cache8WaySet(cacheSize));
		cacheLoads.resize(threadCount);
		cacheHits.resize(threadCount);
	}

	//simultes cache load.
	inline void load(void* pointer)
	{
		//the method should be optimized away when doDacheSimulator is not defined.
		//cast pointer into type we can do math with
		pointerType p = (pointerType)pointer;
		//ignore the first 6 bits to get cache line Id
		pointerType cacheId = p >> 6;

		int tId = omp_get_thread_num();
		cacheLoads[tId]++;
		cacheHits[tId] += cache[tId].load(cacheId);
	}

	//simultes cache load.
	inline void load(pointerType pointer)
	{
		//the method should be optimized away when doDacheSimulator is not defined.
		//ignore the first 6 bits to get cache line Id
		pointerType cacheId = pointer >> 6;

		int tId = omp_get_thread_num();
		cacheLoads[tId]++;
		cacheHits[tId] += cache[tId].load(cacheId);
	}

	//writes cache results of THIS THREAD in a file
	inline void writeThreadResult()
	{
		int tId = omp_get_thread_num();

		std::cerr << "Thread: " << tId << ": Cache hitrate: " << cacheHits[tId] / (float)cacheLoads[tId]
			<< "total sum of all loads : " << cacheLoads[tId]
			<< ", Total Hits: " << cacheHits[tId]
			<< ", Total Cache miss: " << cacheLoads[tId] - cacheHits[tId] << std::endl;
		cacheLoads[tId] = 0;
		cacheHits[tId] = 0;

		//write to what file?

		//reset counters
		cacheLoads[tId] = 0;
		cacheHits[tId] = 0;
	}

	//writes all cache results
	inline void writeAllResult()
	{
		uint64_t sumLoad = std::accumulate(cacheLoads.begin(), cacheLoads.end(), 0ULL);
		uint64_t sumHit = std::accumulate(cacheHits.begin(), cacheHits.end(), 0ULL);

		std::cerr << "total sum of all loads: " << sumLoad
			<< ", Total Hits: " << sumHit
			<< ", Total Cache miss: " << sumLoad - sumHit << std::endl;
		for (int tId = 0; tId < threadCount; tId++)
		{
			std::cerr << "Thread: " << tId << ": Cache hitrate: " << cacheHits[tId] / (float)cacheLoads[tId] << std::endl;
			cacheLoads[tId] = 0;
			cacheHits[tId] = 0;
		}


		//write to what file?

		//reset counters
		resetEverything();
	}

	inline void resetCounter()
	{
		for (int i = 0; i < threadCount; i++)
		{
			cacheLoads[i] = 0;
			cacheHits[i] = 0;
		}
	}

	inline void resetCounter(int tId)
	{
		cacheLoads[tId] = 0;
		cacheHits[tId] = 0;
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
		resetCounter();
	}

	void clearAllCounter()
	{
		std::fill(cacheLoads.begin(), cacheLoads.end(), 0);
		std::fill(cacheHits.begin(), cacheHits.end(), 0);
	}

	float getHitRate()
	{
		int tId = omp_get_thread_num();
		return cacheHits[tId] / (float)cacheLoads[tId];
	}
};
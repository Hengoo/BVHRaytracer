#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <bitset>
#include <array>
#include <omp.h>

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
	int getEvictId()
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
			int evictId = getEvictId();
			if (3452816845 != cacheLines[evictId])
			{
				int abc = 0;
			}
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
	pointerType mask;

public:
	Cache8WaySet(int cacheSize)
	{
		storage.resize(cacheSize / 8);
		//From the log2(#Plru) bits put in correct Plru
		bitsForIndex = log2(cacheSize / 8);
		mask = ((1 << bitsForIndex) - 1);
	}

	bool load(pointerType address)
	{
		return storage[address & mask].load(address);
	}
};



class CacheSimulator
{
	int threadCount;

	Cache8WaySet cache;

	//for each thread: storage for cacheSize many cachelines
	//std::vector<std::stack<pointerType>> cache;
	//one thread per cache, but would be nice if it also supports two threads per cache
public:
	std::vector<uint64_t> loads;
	std::vector<uint64_t> cacheHits;

	CacheSimulator(int cacheSize = 256)
		:cache(Cache8WaySet(cacheSize)), threadCount()
	{
		int threadCount = omp_get_max_threads();
		loads.resize(threadCount);
		cacheHits.resize(threadCount);
	}

	void load(void* pointer)
	{
		//cast pointer into type we can do math with
		pointerType p = (pointerType)pointer;
		//divide by 64 (cacheLine size) to get the cacheLineId
		pointerType cacheId = p / 64;

		int tId = omp_get_thread_num();
		loads[tId]++;
		cacheHits[tId] += cache.load(cacheId);
	}


	void clearCounter()
	{
		std::fill(loads.begin(), loads.end(), 0);
		std::fill(cacheHits.begin(), cacheHits.end(), 0);
	}
};
#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>

//This shouldnt change but not 100% sure
#define pointerType uint32_t

//number of cachelines: for the ryzen threadripper 1950x this is 524,288B what corresponds to 
//256 cachelines with each 64B for each of the 32 threads
//(in theory it would have 512 cachelines that is shared between 2 threads each)
#define cacheSize 256

class cacheSimulator
{
	int threadCount;

	std::vector<uint64_t> loadedCachelines;

	//for each thread: storage for cacheSize many cachelines
	std::vector<std::stack<pointerType>> cache;
	//one thread per cache, but would be nice if it also supports two threads per cache


	void load(void* pointer)
	{
		//cast pointer into type we can do math with
		pointerType p = (pointerType)pointer;
		//divide by 64 (cacheLine size) to get the cacheLineId
		pointerType cacheId = p / 64;
	}

};
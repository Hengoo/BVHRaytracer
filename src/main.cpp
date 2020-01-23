#pragma once

#include"rayTracer.h"

//currently for fastnode sizeof. remove later when im finished with fastNode
#include "accelerationStructure/fastNodeManager.h"

#include "cacheSimulator.h"

/*
	general performance things / todo:
		check inline (there is an optimization option to inline when possible?) --> also learn whats needed for inline to work
		check constexpr and where it can be usefull
		use more const (for safety)
*/



/*
static void testIspc()
{
	std::cout << "starting Ispc test" << std::endl;
	int count = 11;

	std::vector<float> input(count);
	std::iota(input.begin(), input.end(), 0);

	// call the ispc function:
	varyingTest(input.data(), input.size());

	// Print results
	//for (int i = 0; i < 16; ++i)
	//	printf("%d, %f\n", i, input[i]);

}*/

static void testCacheSequence()
{
	//std::vector<int> seq = { 0, 1, 2, 3, 4, 2, 5, 6, 7, 6, 8, 9, 6, 2, 0, 6, 10, 2, 11, 12, 1, 13, 6, 12, 9, 14, 15, 13, 13, 16, 17, 0, 12, 18, 3, 19, 20, 4, 21, 13, 9, 22, 23, 24, 25, 26, 27, 2, 13, 20, 28 };
	std::vector<int> seq = { 0, 1, 1, 1, 2, 3, 3, 4, 5, 5, 1, 6, 3, 6, 0, 1, 7, 1, 1, 0, 8, 4, 2, 1, 9, 10, 1, 11, 12, 13, 1, 14, 15, 16, 17, 18, 19, 3, 20, 21, 3, 1, 1, 22, 23, 24, 8, 13, 1, 1, 9 };
	CacheSimulator cache = CacheSimulator(8);
	for (int i = 0; i < seq.size(); i++)
	{
		cache.load(seq[i] * 64);
	}
	cache.writeAllResult();
}

void cacheTests()
{
	CacheSimulator cache = CacheSimulator(256);

	int extraCount = 500000;
	for (int i = 0; i < 256 * 100; i++)
	{
		if (i % 100 == 0)
		{
			std::cout << "step: " << i << ", hitRatio: " << (float)cache.cacheHits[0] / cache.cacheLoads[0] << std::endl;
			cache.clearAllCounter();
		}

		extraCount++;
		int loadId = i % 100;
		cache.load((void*)(64 * loadId));
		int randId = rand() % 512;
		cache.load((void*)(64 * randId));
		//if (i % 2 == 0)
		//{
		//	cache.load((void*)((extraCount) * 64));
		//}
	}
}

int main()
{
	/*
	testIspc();
	return EXIT_SUCCESS;
	*/

	//cacheTests();
	//testCacheSequence();

	std::cout << "sizes of the nodes:." << std::endl;
	std::cout << "Node " << 4 << ": " << sizeof(FastNode<4>) << std::endl;
	std::cout << "Node " << 8 << ": " << sizeof(FastNode<8>) << std::endl;
	std::cout << "Node " << 12 << ": " << sizeof(FastNode<12>) << std::endl;
	std::cout << "Node " << 16 << ": " << sizeof(FastNode<16>) << std::endl;

	std::cout << "Extra leaf data: LeafSize * 9 * 4 * padding " << std::endl;
	/*
	std::cout << sizeof(FastNode<24>) << std::endl;
	std::cout << sizeof(FastNode<32>) << std::endl;
	std::cout << sizeof(FastNode<40>) << std::endl;
	std::cout << sizeof(FastNode<48>) << std::endl;
	std::cout << sizeof(FastNode<56>) << std::endl;
	std::cout << sizeof(FastNode<64>) << std::endl;
	*/

	RayTracer rayTracer;

	//input from cinfig file
	try
	{
		rayTracer.readConfig();
	}
	catch (const std::exception & e)
	{
		std::cerr << "failed to read config?" << std::endl;
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	//need to check performance impact of this: (seems to be super low if any)
	try
	{
		rayTracer.run();
		std::cout << "end of program" << std::endl;
		system("pause");
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
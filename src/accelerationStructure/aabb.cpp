#include "aabb.h"

//compute best split for every node
primPointVector::iterator Aabb::PrimIntervall::computerBestSplit(float invSurfaceArea, int leafTarget)
{
	//method should be usable for all nodes?

	size_t size = getPrimCount();

	//test all combos with heuristic:
	//save results in array and take the one with best result

	// Split by metric (currently SAH)
	std::vector<float> metric(size - 1);

	Aabb node(0.f, primitiveBegin, primitiveBegin);
	std::for_each(std::execution::seq, metric.begin(), metric.end(),
		[&](auto& met)
		{
			node.sweepRight();
			met += node.sah(invSurfaceArea, leafTarget);
		});
	node = Aabb(0, primitiveEnd, primitiveEnd);
	std::for_each(std::execution::seq, metric.rbegin(), metric.rend(),
		[&](auto& met)
		{
			node.sweepLeft();
			met += node.sah(invSurfaceArea, leafTarget);
		});
	//make the split with the best metric:
	auto bestElement = std::min_element(metric.begin(), metric.end());
	return primitiveBegin + std::distance(metric.begin(), bestElement) + 1;
}

//compute best split for every node with sorting
primPointVector::iterator Aabb::PrimIntervall::computerBestSplitSort(float invSurfaceArea, int leafTarget, int8_t& sortAxis)
{
	//method should be usable for all nodes?

	sortAxis = chooseAxisAndSort(primitiveBegin, primitiveEnd);

	size_t size = getPrimCount();

	//test all combos with heuristic:
	//save results in array and take the one with best result

	// Split by metric (currently SAH)
	std::vector<float> metric(size - 1);

	Aabb node(0.f, primitiveBegin, primitiveBegin);
	std::for_each(std::execution::seq, metric.begin(), metric.end(),
		[&](auto& met)
		{
			node.sweepRight();
			met += node.sah(invSurfaceArea, leafTarget);
		});
	node = Aabb(0, primitiveEnd, primitiveEnd);
	std::for_each(std::execution::seq, metric.rbegin(), metric.rend(),
		[&](auto& met)
		{
			node.sweepLeft();
			met += node.sah(invSurfaceArea, leafTarget);
		});
	//make the split with the best metric:
	auto bestElement = std::min_element(metric.begin(), metric.end());
	return primitiveBegin + std::distance(metric.begin(), bestElement) + 1;
}

//compute best split with buckets
primPointVector::iterator Aabb::PrimIntervall::computerBestSplit(float invSurfaceArea, int leafTarget,
	int bucketCount, glm::vec3& centerMin, glm::vec3& centerMax, uint16_t sortAxis)
{
	//method should be usable for all nodes?

	size_t size = getPrimCount();

	//greedy approach with buckets:

	//split primitives into buckets:
	std::vector<std::unique_ptr<Aabb>> buckets;

	//fill the buckets:


	//easy version:
	for (size_t i = 0; i < bucketCount - 1; i++)
	{
		buckets.push_back(std::make_unique<Aabb>(0.f, primitiveBegin + (size / bucketCount) * i, primitiveBegin + (size / bucketCount) * (i + 1)));
	}
	buckets.push_back(std::make_unique<Aabb>(0.f, primitiveBegin + (size / bucketCount) * (bucketCount - 1), primitiveEnd));

	/*
	//"better" spacial version: (it likes to produce extremly uneven splits because the primitives often have clusters)
	//cut at sortAxis
	int lastCutIndex = 0;
	int bestCutIndex = 0;
	for (size_t i = 1; i < bucketCount; i++)
	{
		auto cut = centerMin[sortAxis] + ((centerMax[sortAxis] - centerMin[sortAxis]) / bucketCount) * i;
		//lazy version: faster would be some kind of binary search
		for (int j = lastCutIndex; j < size; j++)
		{
			//check if prim is right from cut:
			if (cut >= (*(primitiveBegin + j))->getCenter()[sortAxis])
			{
				bestCutIndex = j;
			}
		}
		if (bestCutIndex != lastCutIndex)
		{
			buckets.push_back(std::make_unique<Aabb>(0.f, primitiveBegin + lastCutIndex, primitiveBegin + bestCutIndex));
			lastCutIndex = bestCutIndex;
		}

	}
	if (size != bestCutIndex)
	{
		buckets.push_back(std::make_unique<Aabb>(0.f, primitiveBegin + bestCutIndex, primitiveEnd));
	}
	bucketCount = buckets.size();
	*/

	std::vector<float> metric(bucketCount - 1);

	//iterate trough buckets
	Aabb node(0.f, primitiveBegin, primitiveBegin);
	for (size_t i = 0; i < bucketCount - 1; i++)
	{
		//use all but the first bucket
		node.sweepRight((buckets[i + 1]).get());
		metric[i] = node.sah(invSurfaceArea, leafTarget);
	}
	node = Aabb(0.f, primitiveEnd, primitiveEnd);
	for (int i = bucketCount - 2; i >= 0; i--)
	{
		//use all but the last bucket
		node.sweepLeft((buckets[i]).get());
		metric[i] += node.sah(invSurfaceArea, leafTarget);
	}

	//make the split with the best metric:
	auto bestElement = std::min_element(metric.begin(), metric.end());

	//return best cut position:
	return buckets[std::distance(metric.begin(), bestElement)]->primitiveEnd;

}
void Aabb::recursiveBvh(const unsigned branchingFactor, const unsigned leafTarget, const int bucketCount, const bool sortEachSplit)
{
	std::vector<std::array<int8_t, 3>> sortAxisEachSplit;
	allPrimitiveBegin = primitiveBegin;
	allPrimitiveEnd = primitiveEnd;
	//check primitive count. if less than x primitives, this node is finished. (pbrt would continue of leafcost is larger than split cost !!!)
	if (getPrimCount() <= leafTarget)
	{
		return;
	}

	//TODO: if i want to keep primitives in this node i have to spawn them to primitiveBegin BEFORE the sort
	//also dont forget to set primitiveBegin and primitiveEnd correctly before end of method

	sortAxis = chooseAxisAndSort(primitiveBegin, primitiveEnd);

	std::vector<PrimIntervall> workIntervall;
	workIntervall.push_back(PrimIntervall(primitiveBegin, primitiveEnd));
	int bestI = 0;
	int primCounter = 0;
	float invSurfaceArea = 1 / getSurfaceArea();

	for (size_t b = 0; b < branchingFactor - 1; b++)
	{
		primCounter = 0;
		for (size_t i = 0; i < workIntervall.size(); i++)
		{
			//choose the PrimIntervall with most primitives

			if (primCounter < workIntervall[i].getPrimCount())
			{
				primCounter = workIntervall[i].getPrimCount();
				bestI = i;
			}
		}
		if (primCounter <= leafTarget)
		{
			break;
		}
		primPointVector::iterator bestSplit;
		if (bucketCount <= 0 || workIntervall[bestI].getPrimCount() < bucketCount * 5)
		{
			//compute all possible splits
			if (sortEachSplit)
			{
				int8_t newSortAxis;
				bestSplit = workIntervall[bestI].computerBestSplitSort(invSurfaceArea, leafTarget, newSortAxis);
				//fill sort axis vector correctly: bestI is the id
				int8_t left = 0;
				int8_t right = 0;

				//find the id for left (right = left -1)
				if (!sortAxisEachSplit.empty())
				{
					//need to seach for the entry in sortAxisEachSplit where it points to -(bestI+1)
					//this value is left, right is left -1)
					//and replace it with sortAxisEachSPlit.size()
					int8_t tmpId = 0;
					int8_t tmpSide = 0;
					for (size_t i = 0; i < sortAxisEachSplit.size(); i++)
					{
						if (sortAxisEachSplit[i][1] == -(bestI + 1))
						{
							tmpId = i;
							tmpSide = 1;
							left = sortAxisEachSplit[i][1];
							break;
						}
						if (sortAxisEachSplit[i][2] == -(bestI + 1))
						{
							tmpId = i;
							tmpSide = 2;
							left = sortAxisEachSplit[i][2];
							break;
						}
					}
					if (tmpSide == 0)
					{
						std::cerr << "unexpected per axis sorting error?" << std::endl;
					}
					sortAxisEachSplit[tmpId][tmpSide] = sortAxisEachSplit.size();
					right = left - 1;
					//then increment every id that is smaller than left, so it is in the right order
					for (auto& i : sortAxisEachSplit)
					{
						if (i[1] < 0 && i[1] <= left)
						{
							i[1] = i[1] - 1;
						}
						if (i[2] < 0 && i[2] <= left)
						{
							i[2] = i[2] - 1;
						}
					}

				}
				else
				{
					left = -1;
					right = -2;
				}
				//add new split axis
				sortAxisEachSplit.push_back({ newSortAxis, left, right });
			}
			else
			{
				//all splits slong the same common axis
				bestSplit = workIntervall[bestI].computerBestSplit(invSurfaceArea, leafTarget);
			}

		}
		else
		{
			if (sortEachSplit)
			{
				std::cerr << "Not implemented bucket version of sorting for each split" << std::endl;
				throw 322;
			}
			//compute bucketed split
			//This part is for spacial version: compute bounds of centers
			glm::vec3 min = glm::vec3(222222.0f);
			glm::vec3 max = glm::vec3(-222222.0f);
			glm::vec3 centerDistance;
			std::for_each(std::execution::seq, workIntervall[bestI].primitiveBegin, workIntervall[bestI].primitiveEnd,
				[&](auto& p)
				{
					centerDistance = p->getCenter();
					min = glm::min(min, centerDistance);
					max = glm::max(max, centerDistance);
				});
			bestSplit = workIntervall[bestI].computerBestSplit(invSurfaceArea, leafTarget, bucketCount,
				min, max, sortAxis);
		}


		//split at bestSplit of best intervall:
		PrimIntervall p1(workIntervall[bestI].primitiveBegin, bestSplit);
		PrimIntervall p2(bestSplit, workIntervall[bestI].primitiveEnd);
		workIntervall[bestI] = p1;
		workIntervall.insert(workIntervall.begin() + bestI + 1, p2);
	}
	//create childNodes of every workIntervall;
	//Order has to be sorted!
	for (auto& i : workIntervall)
	{
		addNode(std::make_shared<Aabb>(depth + 1, i.primitiveBegin, i.primitiveEnd));
	}

	//calculate the traversal order for each axis:
	//first get order we have to traverse the childs:
	if (sortEachSplit)
	{
		calculateTraverseOrderEachAxis(branchingFactor, sortAxisEachSplit);
	}

	//just a debug output to get warned abount eventual loop
	if (depth >= 35)
	{
		std::cerr << "bvh is suspiciously deep: " << depth << std::endl;
	}
	primitiveBegin = primitiveEnd;

	//constructs bvh of all children:
	Node::recursiveBvh(branchingFactor, leafTarget, bucketCount, sortEachSplit);
}
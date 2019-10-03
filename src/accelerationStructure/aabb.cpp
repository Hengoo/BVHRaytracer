#include "aabb.h"

primPointVector::iterator Aabb::PrimIntervall::computerBestSplit(float invSurfaceArea, int leafTarget)
{
	//method should be usable for all nodes?


	//version to sort each intervall itself -> mixed result, (without implementing correct traversal)
	/*
	glm::vec3 min = glm::vec3(222222.0f);
	glm::vec3 max = glm::vec3(-222222.0f);
	glm::vec3 centerDistance;
	std::for_each(std::execution::seq, primitiveBegin, primitiveEnd,
		[&](auto& p)
		{
			centerDistance = p->getCenter();
			min = glm::min(min, centerDistance);
			max = glm::max(max, centerDistance);
		});
	centerDistance = max - min;

	//choose axis to split:
	//TODO: possible  version i want to try: take sum of aabb boxes and split the one with the SMALLEST sum (-> least overlapping?)
	int sortAxis = maxDimension(centerDistance);

	//sort primitive array along axis:
	//its faster to first check if its sorted
	if (!std::is_sorted(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis)))
	{
		//TODO test what parallel stuff like std::execution::seq or unseqpar does here
		std::sort(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis));
	}
	*/

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

primPointVector::iterator Aabb::PrimIntervall::computerBestSplit(float invSurfaceArea, int leafTarget,
	int bucketCount, glm::vec3 centerMin, glm::vec3 centerMax, uint16_t sortAxis)
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
	//better spacial version:
	// cut at sortAxis 
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
		node.sweepRight(&*buckets[i + 1]);
		metric[i] = node.sah(invSurfaceArea, leafTarget);
	}
	node = Aabb(0.f, primitiveEnd, primitiveEnd);
	for (int i = bucketCount - 2; i >= 0; i--)
	{
		//use all but the last bucket
		node.sweepLeft(&*buckets[i]);
		metric[i] += node.sah(invSurfaceArea, leafTarget);
	}

	//make the split with the best metric:
	auto bestElement = std::min_element(metric.begin(), metric.end());

	//return best cut position:
	return buckets[std::distance(metric.begin(), bestElement)]->primitiveEnd;

}
void Aabb::recursiveBvh(const unsigned int branchingFactor, const unsigned int leafTarget, int bucketCount)
{
	//check primitive count. if less than x primitives, this node is finished. (pbrt would continue of leafcost is larger than split cost !!!)
	if (getPrimCount() <= leafTarget)
	{
		return;
	}

	//TODO: if i want to keep primitives in this node i have to spawn them to primitiveBegin BEFORE the sort
	//also dont forget to set primitiveBegin and primitiveEnd correctly before end of method


	//calculate distance of centers along each axis -> largest distance is the axis we want to split
	glm::vec3 min = glm::vec3(222222.0f);
	glm::vec3 max = glm::vec3(-222222.0f);
	glm::vec3 centerDistance;
	std::for_each(std::execution::seq, primitiveBegin, primitiveEnd,
		[&](auto& p)
		{
			centerDistance = p->getCenter();
			min = glm::min(min, centerDistance);
			max = glm::max(max, centerDistance);
		});
	centerDistance = max - min;

	//choose axis to split:
	//TODO: possible  version i want to try: take sum of aabb boxes and split the one with the SMALLEST sum (-> least overlapping?)
	sortAxis = maxDimension(centerDistance);

	//stop when triangle centers are at the same position (when this happens to often i might try the real center instead of the aabb center????
	//if (centerDistance[axis] <= 0.00002)
	//{
	//	return;
	//}


	//sort primitive array along axis:
	//its faster to first check if its sorted
	if (!std::is_sorted(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis)))
	{
		//TODO test what parallel stuff like std::execution::seq or unseqpar does here
		std::sort(primitiveBegin, primitiveEnd, std::bind(sortPrimitive, std::placeholders::_1, std::placeholders::_2, sortAxis));
	}


	std::vector<PrimIntervall> workIntervall;
	workIntervall.push_back(PrimIntervall(primitiveBegin, primitiveEnd));
	int bestI = 0;
	float primCounter = 0;

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
			bestSplit = workIntervall[bestI].computerBestSplit(1 / getSurfaceArea(), leafTarget);
		}
		else
		{
			min = glm::vec3(222222.0f);
			max = glm::vec3(-222222.0f);
			std::for_each(std::execution::seq, workIntervall[bestI].primitiveBegin, workIntervall[bestI].primitiveEnd,
				[&](auto& p)
				{
					centerDistance = p->getCenter();
					min = glm::min(min, centerDistance);
					max = glm::max(max, centerDistance);
				});
			bestSplit = workIntervall[bestI].computerBestSplit(1 / getSurfaceArea(), leafTarget, bucketCount,
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

	//just a debug output to get warned abount eventual loop
	if (depth >= 35)
	{
		std::cout << depth << std::endl;
	}

	primitiveBegin = primitiveEnd;

	//constructs bvh of all children:
	Node::recursiveBvh(branchingFactor, leafTarget, bucketCount);
}



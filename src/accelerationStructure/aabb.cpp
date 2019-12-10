#include "aabb.h"

//compute best split for every node
primPointVector::iterator Aabb::PrimIntervall::computerBestSplit(float invSurfaceArea, int leafTarget, float& sahSplitCost)
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
	sahSplitCost = *bestElement;
	return primitiveBegin + std::distance(metric.begin(), bestElement) + 1;
}

//compute best split for every node with sorting
primPointVector::iterator Aabb::PrimIntervall::computerBestSplitSort(float invSurfaceArea, int leafTarget, int8_t& sortAxis, float& sahSplitCost)
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
	node = Aabb(0.f, primitiveEnd, primitiveEnd);
	std::for_each(std::execution::seq, metric.rbegin(), metric.rend(),
		[&](auto& met)
		{
			node.sweepLeft();
			met += node.sah(invSurfaceArea, leafTarget);
		});
	//make the split with the best metric:
	auto bestElement = std::min_element(metric.begin(), metric.end());
	sahSplitCost = *bestElement;
	return primitiveBegin + std::distance(metric.begin(), bestElement) + 1;
}

void Aabb::recursiveBvh(const unsigned branchingFactor, const unsigned leafTarget, const bool sortEachSplit, const bool smallLeafs)
{
	std::vector<std::array<int8_t, 3>> sortAxisEachSplit;
	allPrimitiveBegin = primitiveBegin;
	allPrimitiveEnd = primitiveEnd;
	//check primitive count. if less than x primitives, this node is finished. (pbrt would continue of leafcost is larger than split cost !!!)

	size_t nodePrimCount = getPrimCount();

	if (nodePrimCount <= leafTarget && !smallLeafs)
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
		float splitCost;
		primPointVector::iterator bestSplit = getSplitIntervall(workIntervall, bestI, sortEachSplit, invSurfaceArea, leafTarget, sortAxisEachSplit, splitCost, true);

		//split at bestSplit of best intervall:
		PrimIntervall p1(workIntervall[bestI].primitiveBegin, bestSplit);
		PrimIntervall p2(bestSplit, workIntervall[bestI].primitiveEnd);

		//apply split to workintervall
		workIntervall[bestI] = p1;
		workIntervall.insert(workIntervall.begin() + bestI + 1, p2);
	}

	//if node count not full: do second loop over all current childs and look which of those nodes could be split to smaller leafs
	for (size_t b = workIntervall.size() - 1; b < branchingFactor - 1; b++)
	{
		//try all nodes and split them. Choose the one with the best sah improvement
		float bestSahImprovement = 0;
		int bestSahImprovementId = -1;
		primPointVector::iterator bestSahImprovementSplit;

		for (size_t i = 0; i < workIntervall.size(); i++)
		{
			//Leaf cost is always 1 and node intersection is the factor
			int intervallNodeCount = workIntervall[i].getPrimCount();
			if (intervallNodeCount == 1)
			{
				continue;
			}
			float leafCost = intervallNodeCount;
			float splitCost = 0.f;
			//try split:
			primPointVector::iterator bestSplit = getSplitIntervall(workIntervall, i, sortEachSplit, invSurfaceArea, leafTarget, sortAxisEachSplit, splitCost, false);

			//the cost factor for nodes. (according to pbgt book its +
			splitCost += 0.6f;
			if (leafCost - splitCost > bestSahImprovement)
			{
				bestSahImprovement = leafCost - splitCost;
				bestSahImprovementId = i;
				bestSahImprovementSplit = bestSplit;
			}
		}

		if (bestSahImprovement == 0)
		{
			//no improvement possible
			break;
		}
		else
		{
			//apply split and continue with loop

			//do split again to apply sort axis change
			float splitCost = 0.f;
			primPointVector::iterator bestSplit = getSplitIntervall( workIntervall, bestSahImprovementId, sortEachSplit, invSurfaceArea, leafTarget, sortAxisEachSplit, splitCost, true);

			//split at bestSplit of best intervall:
			PrimIntervall p1(workIntervall[bestSahImprovementId].primitiveBegin, bestSahImprovementSplit);
			PrimIntervall p2(bestSahImprovementSplit, workIntervall[bestSahImprovementId].primitiveEnd);

			//apply split to workintervall
			workIntervall[bestSahImprovementId] = p1;
			workIntervall.insert(workIntervall.begin() + bestSahImprovementId + 1, p2);
		}
	}

	if (workIntervall.size() == 1)
	{
		return;
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
	if (depth >= 40)
	{
		std::cerr << "bvh is suspiciously deep: " << depth << std::endl;
	}
	primitiveBegin = primitiveEnd;

	//constructs bvh of all children:
	Node::recursiveBvh(branchingFactor, leafTarget, sortEachSplit, smallLeafs);
}

primPointVector::iterator Aabb::getSplitIntervall(std::vector<Aabb::PrimIntervall>& workIntervall,	int bestI,
	const bool& sortEachSplit, float invSurfaceArea, const unsigned int& leafTarget, std::vector<std::array<int8_t, 3Ui64>>& sortAxisEachSplit,
	float& sahSplitCost, bool applySortAxisChange)
{
	primPointVector::iterator resultValue;
	//compute all possible splits
	if (sortEachSplit)
	{
		int8_t newSortAxis;
		resultValue = workIntervall[bestI].computerBestSplitSort(invSurfaceArea, leafTarget, newSortAxis, sahSplitCost);

		if (applySortAxisChange)
		{
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
					throw(322);
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
	}
	else
	{
		//all splits along the same common axis
		resultValue = workIntervall[bestI].computerBestSplit(invSurfaceArea, leafTarget, sahSplitCost);
	}
	return resultValue;
}

#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
//for the parallel for
#include <execution>

#include "node.h"
#include "../primitives/primitive.h"
#include "../ray.h"
#include "../glmUtil.h"

void Node::addNode(std::shared_ptr<Node> n)
{
	children.push_back(n);
}

//void Node::addPrimitive(std::shared_ptr<Primitive> p)
//{
//	primitives.push_back(p);
//}

void Node::recursiveBvh(const unsigned branchingFactor, const unsigned leafSize, const bool sortEachSplit, const int leafSplitOption, float sahFactor)
{
	std::for_each(std::execution::par_unseq, children.begin(), children.end(),
		[&](auto& c)
		{
			c->recursiveBvh(branchingFactor, leafSize, sortEachSplit, leafSplitOption, sahFactor);
		});
}

void Node::recursiveOctree(const unsigned leafSize)
{
	std::for_each(std::execution::par_unseq, children.begin(), children.end(),
		[&](auto& c)
		{
			c->recursiveOctree(leafSize);
		});
}

//recursive node/ray intersect
bool Node::intersect(Ray& ray)
{
	bool result = false;

	if (getPrimCount() != 0)
	{
		//save primitivecount
		//so we know how much we space we waste (and how efficiently we use cachelines)
		ray.primitiveFullness[getPrimCount()] ++;

		ray.leafIntersectionCount[depth]++;

		//std::all_of stops loop when false is returned
		if (!ray.shadowRay)
		{
			std::for_each(primitiveBegin, primitiveEnd,
				[&](auto& p)
				{
					if (p->intersect(ray))
					{
						result = true;
					}
				});
		}
		else
		{
			return std::any_of(primitiveBegin, primitiveEnd,
				[&](auto& p)
				{
					if (p->intersect(ray))
					{
						if (ray.shadowRay)
						{
							return true;
						}
					}
					return false;
				});
		}

	}

	if (getChildCount() != 0)
	{
		//save childcount of this intersection
		ray.childFullness[getChildCount()] ++;
		//increment node intersection counter
		ray.nodeIntersectionCount[depth]++;

		if (getPrimCount() > 0)
		{
			std::cerr << "TODO: implement correct counter for primitives in non leaf nodes" << std::endl;
		}

		std::vector<std::shared_ptr<Node>>::iterator begin = children.begin();
		std::vector<std::shared_ptr<Node>>::iterator end = children.end();

		if (traverseOrderEachAxis[0].empty())
		{
			//code duplication because it is about 10% faster to do it this way instead of calling a method in lambda or using std::bind
			if (ray.direction[sortAxis] > 0)
			{
				if (!ray.shadowRay)
				{
					std::for_each(children.begin(), children.end(),
						[&](auto& c)
						{
							ray.aabbIntersectionCount++;
							float dist;
							if (c->intersectNode(ray, dist))
							{
								ray.successfulAabbIntersectionCount++;

								//node intersection successful: rekursion continues
								if (c->intersect(ray))
								{
									result = true;
								}
							}
						});
				}
				else
				{
					return std::any_of(children.begin(), children.end(),
						[&](auto& c)
						{
							ray.aabbIntersectionCount++;
							float dist;
							if (c->intersectNode(ray, dist))
							{
								ray.successfulAabbIntersectionCount++;

								//node intersection successful: rekursion continues
								if (c->intersect(ray))
								{
									return true;
								}
							}
							return false;
						});
				}
			}
			else
			{
				if (!ray.shadowRay)
				{
					std::for_each(children.rbegin(), children.rend(),
						[&](auto& c)
						{
							ray.aabbIntersectionCount++;
							float dist;
							if (c->intersectNode(ray, dist))
							{
								ray.successfulAabbIntersectionCount++;

								//node intersection successful: rekursion continues
								if (c->intersect(ray))
								{
									result = true;
								}
							}
						});
				}
				else
				{
					return std::any_of(children.rbegin(), children.rend(),
						[&](auto& c)
						{
							ray.aabbIntersectionCount++;
							float dist;
							if (c->intersectNode(ray, dist))
							{
								ray.successfulAabbIntersectionCount++;

								//node intersection successful: rekursion continues
								if (c->intersect(ray))
								{
									return true;
								}
							}
							return false;
						});
				}
			}

		}
		else
		{
			//traverse nodes with children that can have arbitrary sorting
			int code = maxAbsDimension(ray.direction);
			bool reverse = ray.direction[code] <= 0;
			if (reverse)
			{
				for (int i = traverseOrderEachAxis[code].size() - 1; i >= 0; i--)
				{
					uint8_t cid = traverseOrderEachAxis[code][i];
					ray.aabbIntersectionCount++;

					float dist;
					if (children[cid]->intersectNode(ray, dist))
					{
						ray.successfulAabbIntersectionCount++;
						//node intersection successful: rekursion continues
						if (children[cid]->intersect(ray))
						{
							result = true;
							if (ray.shadowRay)
							{
								return true;
							}
						}
					}
				}
			}
			else
			{
				for (auto& cid : traverseOrderEachAxis[code])
				{
					ray.aabbIntersectionCount++;

					float dist;
					if (children[cid]->intersectNode(ray, dist))
					{
						ray.successfulAabbIntersectionCount++;
						//node intersection successful: rekursion continues
						if (children[cid]->intersect(ray))
						{
							result = true;
							if (ray.shadowRay)
							{
								return true;
							}
						}
					}

				}
			}
		}
	}
	return result;
}

size_t Node::getChildCount()
{
	return children.size();
}

size_t Node::getPrimCount()
{
	return std::distance(primitiveBegin, primitiveEnd);
}

void Node::calculateTraverseOrderEachAxis(unsigned int branchingFactor)
{
	std::array < std::vector < std::tuple<float, int>>, 3> distanceIdTuple;
	//second version that goes by faces (major ray axis). Ordering is done by box center

	for (int cId = 0; cId < getChildCount(); cId++)
	{
		auto center = children[cId]->getCenter();
		for (int face = 0; face < 3; face++)
		{
			distanceIdTuple[face].push_back(std::make_tuple(center[face], cId));
		}
	}
	for (int face = 0; face < 3; face++)
	{
		std::sort(distanceIdTuple[face].begin(), distanceIdTuple[face].end());
		for (auto& tup : distanceIdTuple[face])
		{
			traverseOrderEachAxis[face].push_back(std::get<1>(tup));
		}
	}
}
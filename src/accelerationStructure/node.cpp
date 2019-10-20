#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
//for the parallel for
#include <execution>

#include "node.h"
#include "../primitives/primitive.h"
#include "../ray.h"

void Node::addNode(std::shared_ptr<Node> n)
{
	children.push_back(n);
}

//void Node::addPrimitive(std::shared_ptr<Primitive> p)
//{
//	primitives.push_back(p);
//}

void Node::recursiveBvh(const unsigned int branchingFactor, const unsigned int leafSize, int bucketCount, bool sortEachSplit)
{
	std::for_each(std::execution::par_unseq, children.begin(), children.end(),
		[&](auto& c)
		{
			c->recursiveBvh(branchingFactor, leafSize, bucketCount, sortEachSplit);
		});
}

void Node::recursiveOctree(const unsigned int leafSize)
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
					ray.primitiveIntersectionCount++;
					if (p->intersect(ray))
					{
						ray.successfulPrimitiveIntersectionCount++;
						result = true;
					}
				});
		}
		else
		{
			return std::any_of(primitiveBegin, primitiveEnd,
				[&](auto& p)
				{
					ray.primitiveIntersectionCount++;
					if (p->intersect(ray))
					{
						ray.successfulPrimitiveIntersectionCount++;
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


		if (sortAxisEachSplit.empty())
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

			//queue saves the id of elelemnts of sortAxisEachSplit where we still need to explore the 2. option
			//true = visited, false = not visited
			std::vector<std::pair<int8_t, bool>>queue;
			queue.reserve(children.size());

			queue.push_back({ 0, false });
			while (!queue.empty())
			{
				auto current = queue.back();
				queue.pop_back();

				//faster to copy than to do reference or pointer
				auto& a = sortAxisEachSplit[current.first];
				//side. true = left, false = right
				bool side = ray.direction[a[0]] > 0;
				if (current.second)
				{
					side = !side;
				}

				int8_t id = 0;
				if (side)
				{
					id = a[1];
				}
				else
				{
					id = a[2];
				}
				//add second part to queue
				if (!current.second)
				{
					queue.push_back({ current.first,true });
				}
				if (id < 0)
				{
					//traverse aabb
					uint8_t c = abs(id) - 1;
					ray.aabbIntersectionCount++;

					float dist;
					if (children[c]->intersectNode(ray, dist))
					{
						ray.successfulAabbIntersectionCount++;

						//node intersection successful: rekursion continues
						if (children[c]->intersect(ray))
						{
							result = true;
							if (ray.shadowRay)
							{
								return true;
							}
						}
					}
				}
				else
				{
					queue.push_back({ id, false });
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
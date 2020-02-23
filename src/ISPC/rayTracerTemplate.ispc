//strongly inspired by ISPC raytracer example https://github.com/ispc/ispc/blob/master/examples/rt/rt.ispc

export uniform int getGangSize()
{
	return programCount;
}

//ISPC Equivalent to glm::vec3 :
typedef float<3> float3;

static inline float dot(const float3 a, const float3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float3 cross(float3 v0, float3 v1) {
	float3 ret;
	ret.x = v0.y * v1.z - v0.z * v1.y;
	ret.y = v0.z * v1.x - v0.x * v1.z;
	ret.z = v0.x * v1.y - v0.y * v1.x;
	return ret;
}

//input: triangle array (uniform pointer, and since its a pointer also uniform data). Uniform ray. (all access the same ray, but dont write on it). tMax -> is our output
//storage is needed for varing leafsize -> seems like i have to copy paste it once for 4, 8, and 12 to not have it
{% for leafMemory in leafNumbers%}
export uniform int triIntersect{{leafMemory}}(const uniform float tri[], uniform unsigned int triId, uniform float* uniform rayInfo)
{
	//large value so reduce_min never gets masked variables
	varying float tStore = 1000000;
	//orig index is needed because second loop is over gangsize and not the full leaf
	varying int origIndex = -1;
	uniform bool result = false;

	foreach(index = 0 ... {{leafMemory}})
	{
		//fast read:
		float3 p0 = { tri[(triId + index                      ) * {{padding}}], tri[(triId + index + {{leafMemory}}     ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 2) * {{padding}}] };
		float3 p1 = { tri[(triId + index + {{leafMemory}} * 3 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 4 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 5) * {{padding}}] };
		float3 p2 = { tri[(triId + index + {{leafMemory}} * 6 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 7 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 8) * {{padding}}] };
		
		//print positions:
		//print("p0 = %, %, % \n", p0.x, p0.y, p0.z);
		//print("p1 = %, %, % \n", p1.x, p1.y, p1.z);
		//print("p2 = %, %, % \n", p2.x, p2.y, p2.z);

		uniform float3 origin = {rayInfo[1],rayInfo[2],rayInfo[3] };
		uniform float3 dir = {rayInfo[7],rayInfo[8],rayInfo[9] };

		float3 e1 = p1 - p0;
		float3 e2 = p2 - p0;

		float3 s1 = cross(dir, e2);
		float divisor = dot(s1, e1);

		float hit = true;
		if (divisor == 0.)
			hit = false;
		float invDivisor = 1.f / divisor;

		// Compute first barycentric coordinate
		float3 d = origin - p0;
		float b1 = dot(d, s1) * invDivisor;
		if (b1 < 0. || b1 > 1.)
			hit = false;

		// Compute second barycentric coordinate
		float3 s2 = cross(d, e1);
		float b2 = dot(dir, s2) * invDivisor;
		if (b2 < 0. || b1 + b2 > 1.)
			hit = false;

		// Compute _t_ to intersection point
		float t = dot(e2, s2) * invDivisor;
		if (t < 0 || t > rayInfo[0])
			hit = false;

		if (hit)
		{
			//only overwrite result if we have a closer hit. (only needed if count > lane number)
			if(tStore > t)
			{
				result = true;
				tStore = t;
				origIndex = index;
			}
		}
	}
	if(result)
	{
		uniform float bestT = reduce_min(tStore);
		varying int index = -1;
		rayInfo[0]= bestT;
		//Since most of the entries should be 100000 we can use foreach unique to save some time
		foreach_unique(val in tStore)
		{
			if(bestT == val)
			{
				index = origIndex;
			}
		}
		return reduce_max(index);
	}
	else
	{
		return -1;
	}
}
{% endfor %}

//Same as triIntersect but for shadow rays. (no output besides bool)
{% for leafMemory in leafNumbers%}
export uniform bool triAnyHit{{leafMemory}}(const uniform float tri[], uniform unsigned int triId, uniform float* uniform rayInfo)
{
	//large value so reduce_min never gets masked variables
	uniform bool result = false;

	foreach(index = 0 ... {{leafMemory}} )
	{
		//fast read:
		float3 p0 = { tri[(triId + index                      ) * {{padding}}], tri[(triId + index + {{leafMemory}}     ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 2) * {{padding}}] };
		float3 p1 = { tri[(triId + index + {{leafMemory}} * 3 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 4 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 5) * {{padding}}] };
		float3 p2 = { tri[(triId + index + {{leafMemory}} * 6 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 7 ) * {{padding}}], tri[(triId + index + {{leafMemory}} * 8) * {{padding}}] };
		
		//print positions:
		//print("p0 = %, %, % \n", p0.x, p0.y, p0.z);
		//print("p1 = %, %, % \n", p1.x, p1.y, p1.z);
		//print("p2 = %, %, % \n", p2.x, p2.y, p2.z);

		uniform float3 origin = {rayInfo[1],rayInfo[2],rayInfo[3] };
		uniform float3 dir = {rayInfo[7],rayInfo[8],rayInfo[9] };

		float3 e1 = p1 - p0;
		float3 e2 = p2 - p0;

		float3 s1 = cross(dir, e2);
		float divisor = dot(s1, e1);

		float hit = true;
		if (divisor == 0.)
			hit = false;
		float invDivisor = 1.f / divisor;

		// Compute first barycentric coordinate
		float3 d = origin - p0;
		float b1 = dot(d, s1) * invDivisor;
		if (b1 < 0. || b1 > 1.)
			hit = false;

		// Compute second barycentric coordinate
		float3 s2 = cross(d, e1);
		float b2 = dot(dir, s2) * invDivisor;
		if (b2 < 0. || b1 + b2 > 1.)
			hit = false;

		// Compute _t_ to intersection point
		float t = dot(e2, s2) * invDivisor;
		if (t < 0 || t > rayInfo[0])
			hit = false;

		if (hit)
		{
			//only overwrite result if we have a closer hit. (only needed if count > lane number)
			result = true;
		}
	}
	return result;
}
{% endfor %}

{% for nodeMemory in nodeNumbers%}
export uniform bool aabbIntersect{{nodeMemory}}(const uniform float bounds[], uniform float distances[], uniform float* uniform rayInfo)
{
	//its faster to return if we had one positive hit
	uniform bool result = false;

	foreach(index = 0 ... {{nodeMemory}})
	{
		//load bounds:
		float3 boundMin = { bounds[(index                     ) * {{padding}}], bounds[(index + {{nodeMemory}}    ) * {{padding}}], bounds[(index + {{nodeMemory}} * 2) * {{padding}}] };
		float3 boundMax = { bounds[(index + {{nodeMemory}} * 3) * {{padding}}], bounds[(index + {{nodeMemory}} * 4) * {{padding}}], bounds[(index + {{nodeMemory}} * 5) * {{padding}}] };

		//its faster to just compute trough it with 0,0,0 and 0,0,0 ...
		//if(index >= nodeSize)
		//{
		//	//boundMax += boundMin;
		//	//boundMin += boundMax;
		//	continue;
		//}

		uniform float dist = rayInfo[0];
		uniform float3 origin = { rayInfo[1], rayInfo[2], rayInfo[3]};
		uniform float3 invDir = { rayInfo[4], rayInfo[5], rayInfo[6]};
		//get ray data:
		float t0 = 0, t1 = dist;
	
		float3 tNear = (boundMin - origin) * invDir;
		float3 tFar = (boundMax - origin) * invDir;

		if (tNear.x > tFar.x) {
			float tmp = tNear.x;
			tNear.x = tFar.x;
			tFar.x = tmp;
		}
		t0 = max(tNear.x, t0);
		t1 = min(tFar.x, t1);

		if (tNear.y > tFar.y) {
			float tmp = tNear.y;
			tNear.y = tFar.y;
			tFar.y = tmp;
		}
		t0 = max(tNear.y, t0);
		t1 = min(tFar.y, t1);

		if (tNear.z > tFar.z) {
			float tmp = tNear.z;
			tNear.z = tFar.z;
			tFar.z = tmp;
		}
		t0 = max(tNear.z, t0);
		t1 = min(tFar.z, t1);

		//print("% \n", t0);
		if(t0 <= t1)
		{
			//print("deb");
			//print("% \n", t0);
			result = true;
			distances[index] = t0;
		}
	}
	return result;
}
{% endfor %}
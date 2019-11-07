//strongly inspired / taken from ISPC raytracer example https://github.com/ispc/ispc/blob/master/examples/rt/rt.ispc

//#define bool int

//my glm::vec3 are just 3 floats after each other so this fits:
typedef float<3> float3;

struct Triangle
{
	float p[3][4];
	//float3 p0;
	//float3 p1;
	//float3 p2;
	int pad[4];
};

struct Ray
{
	float origin[3];
	float direction[3];
	float invDirection[3];
	float surfaceNormal[3];
	float surfacePosition[3];
	float tMax;
	bool shadowRay;
};

static inline float dot(const float3 a, const float3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*
static inline float3 cross(const float3 v1, const float3 v2) {
	float v1x = v1.x, v1y = v1.y, v1z = v1.z;
	float v2x = v2.x, v2y = v2.y, v2z = v2.z;
	float3 ret;
	ret.x = (v1y * v2z) - (v1z * v2y);
	ret.y = (v1z * v2x) - (v1x * v2z);
	ret.z = (v1x * v2y) - (v1y * v2x);
	return ret;
}*/

//both seem the be the same speed
static inline float3 cross(float3 v0, float3 v1) {
	float3 ret;
	ret.x = v0.y * v1.z - v0.z * v1.y;
	ret.y = v0.z * v1.x - v0.x * v1.z;
	ret.z = v0.x * v1.y - v0.y * v1.x;
	return ret;
}

static inline void normalize(float3 &v) {
	float len2 = dot(v, v);
	float invlen = rsqrt(len2);
	v *= invlen;
}

static inline uniform float3 uniformCross(const uniform float3 v1, const uniform float3 v2) {
	uniform float v1x = v1.x, v1y = v1.y, v1z = v1.z;
	uniform float v2x = v2.x, v2y = v2.y, v2z = v2.z;
	uniform float3 ret;
	ret.x = (v1y * v2z) - (v1z * v2y);
	ret.y = (v1z * v2x) - (v1x * v2z);
	ret.z = (v1x * v2y) - (v1y * v2x);
	return ret;
}

static inline uniform float3 computeNormal2(const uniform float3& p0, const uniform float3& p1,const uniform float3& p2)
{
	return uniformCross(p1 - p0, p2 - p0);
}

static inline float3 computeNormal(const float3& p0,const float3& p1,const float3& p2)
{
	return cross(p1 - p0, p2 - p0);
}

//input: triangle array (uniform pointer, and since its a pointer also uniform data). Uniform ray. (all access the same ray, but dont write on it). tMax -> is our output
//storage is needed for varing leafsize -> seems like i have to copy paste it once for 4, 8, and 12 to not have it
export uniform bool triIntersect(const uniform float tri[], uniform unsigned int triId, uniform Ray* uniform ray,
uniform float surfaceNormals[], uniform float surfacePositions[], uniform unsigned int8 count)
{
	//need to make result vector and later gather the smallest hit and itsurface normal
	uniform bool result = false;

	//large value so reduce_min never gets masked variables
	varying float tStore = 1000000;
	varying float3 surfaceNormal;
	varying float3 surfacePosition;
	foreach(index = 0 ... count)
	{

		//fast read:
		float3 p0 = { tri[triId + index            ], tri[triId + index + count    ], tri[triId + index + count * 2] };
		float3 p1 = { tri[triId + index + count * 3], tri[triId + index + count * 4], tri[triId + index + count * 5] };
		float3 p2 = { tri[triId + index + count * 6], tri[triId + index + count * 7], tri[triId + index + count * 8] };
		
		//const uniform pad = 8;
		//float3 p0 = { tri[triId + index            ], tri[triId + index + pad    ], tri[triId + index + pad * 2] };
		//float3 p1 = { tri[triId + index + pad * 3], tri[triId + index + pad * 4], tri[triId + index + pad * 5] };
		//float3 p2 = { tri[triId + index + pad * 6], tri[triId + index + pad * 7], tri[triId + index + pad * 8] };
		
		/*
		//print positions:
		print("p0 = %, %, % \n", p0.x, p0.y, p0.z);
		print("p1 = %, %, % \n", p1.x, p1.y, p1.z);
		print("p2 = %, %, % \n", p2.x, p2.y, p2.z);
		*/

		/*
		float3 p0 = { tri[triId + index].p[0][0], tri[triId + index].p[0][1], tri[triId + index].p[0][2] };
		float3 p1 = { tri[triId + index].p[1][0], tri[triId + index].p[1][1], tri[triId + index].p[1][2] };
		float3 p2 = { tri[triId + index].p[2][0], tri[triId + index].p[2][1], tri[triId + index].p[2][2] };
		*/

		uniform float3 origin = { ray->origin[0], ray->origin[1], ray->origin[2] };
		uniform float3 dir = { ray->direction[0], ray->direction[1], ray->direction[2] };

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
		if (t < 0 || t > ray->tMax)
			hit = false;

		if (hit)
		{
			//only overwrite result if we have a closer hit. (only needed if count > lane number)
			if(tStore > t)
			{
				tStore = t;
				//its faster to normalize it later.
				surfaceNormal = computeNormal(p0,p1,p2);
				surfacePosition = p0 + e1 * b1 + e2 * b2;
				result = true;
			}
		}
	}
	if(result)
	{	
		uniform float tMax = reduce_min(tStore);
		ray->tMax = tMax;

		//looping over gang size
		//foreach(index = 0 ... programCount)

		//Since most of the entries should be 100000 we can use foreach unique to save some time
		foreach_unique(val in tStore)
		{
			if(tMax == val)
			{
				normalize(surfaceNormal);
				surfaceNormals[programIndex*3 ] = surfaceNormal.x;
				surfaceNormals[programIndex*3 + 1] = surfaceNormal.y;
				surfaceNormals[programIndex*3 + 2] = surfaceNormal.z;

				surfacePositions[programIndex *3 ] = surfacePosition.x;
				surfacePositions[programIndex *3 +1] = surfacePosition.y;
				surfacePositions[programIndex *3 +2] = surfacePosition.z;
			}
		}
	}
	return result;
}


export uniform bool aabbIntersect(const uniform float bounds[], uniform float distances[], uniform Ray* uniform ray, uniform int boundId, uniform unsigned int8 count)
{
	//its faster to return if we hat one positive hit
	uniform bool result = false;
	foreach(index = 0 ... count)
	{
		//load bounds:
		
		float3 boundMin = { bounds[boundId + index            ], bounds[boundId + index + count    ], bounds[boundId + index + count * 2] };
		float3 boundMax = { bounds[boundId + index + count * 3], bounds[boundId + index + count * 4], bounds[boundId + index + count * 5] };
		//const uniform int pad = 8;
		//float3 boundMin = { bounds[boundId + index            ], bounds[boundId + index + pad    ], bounds[boundId + index + pad * 2] };
		//float3 boundMax = { bounds[boundId + index + pad * 3], bounds[boundId + index + pad * 4], bounds[boundId + index + pad * 5] };

		//get ray data:
		uniform float3 origin = { ray->origin[0], ray->origin[1], ray->origin[2] };
		uniform float3 invDir = { ray->invDirection[0], ray->invDirection[1], ray->invDirection[2] };
		float t0 = 0, t1 = ray->tMax;

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
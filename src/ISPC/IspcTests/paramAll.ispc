
typedef float<3> float3;
export uniform float testConst (const uniform float bounds[], uniform int loop)
{
	uniform float result = 0;
	foreach(index = 0 ... loop)
	{
		//load bounds:

		float3 boundMin = { bounds[index           ], bounds[index + loop    ], bounds[index + loop * 2] };
		float3 boundMax = { bounds[index + loop * 3], bounds[index + loop * 4], bounds[index + loop * 5] };
		float sum = boundMin.x + boundMin.y + boundMin.z + boundMax.x + boundMax.y + boundMax.z;
		result = reduce_max(sum);
	}
	return result;
}

typedef float<3> float3;
export uniform float testConst (const uniform float bounds[], uniform int loop)
{
	uniform float result = 0;
	foreach(index = 0 ... loop * 4)
	{
		//load bounds:

		float3 boundMin = { bounds[index        ], bounds[index + 4    ], bounds[index + 4 * 2] };
		float3 boundMax = { bounds[index + 4 * 3], bounds[index + 4 * 4], bounds[index + 4 * 5] };
		float sum = boundMin.x + boundMin.y + boundMin.z + boundMax.x + boundMax.y + boundMax.z;
		result = reduce_max(sum);
	}
	return result;
}
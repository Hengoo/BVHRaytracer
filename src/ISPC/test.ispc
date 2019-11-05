#define bool int

//my glm::vec3 are just 3 floats after each other so this fits:
typedef float<3> float3;

export void firstTest(uniform float vector[], uniform int count)
{
	foreach (i = 0 ... count) 
	{
		float value = vector[i];
		//just some random roots
		value = sqrt(value);
		vector[i] = value;
	}
}

export void varyingTest(const uniform float test[], uniform int count)
{
	varying float small = 0;
	varying float minimum = 0;
	foreach (i = 0 ... count) 
	{
		print("% \n", small);
		small += test[i];
		minimum = min(minimum, test[i]);
	}
	float sum = reduce_add(small);
	float min = reduce_min(minimum);
	print("%, %", sum, minimum);
}

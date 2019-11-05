#define bool int

//my glm::vec3 are just 3 floats after each other so this fits:
typedef float<3> float3;

export void firstTest(uniform float vector[], uniform int count)
{
	foreach (i = 0 ... count) 
	{
		float value = vector[i];
		//just some random roots
		value  = sqrt(value);
		vector[i] = value;
	}
}

export void glmTest(const uniform float test[], uniform int count)
{
	foreach (i = 0 ... count) 
	{
		print("test % \n" , test[i] );
	}
}

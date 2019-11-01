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

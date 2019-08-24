#pragma once

#include <algorithm>

//random int
int rint(int min, int max)
{
	int r = rand() % (max - min) + min;
	return r;
}

//random unsigned char (for colors)
unsigned char ruchar(unsigned char min, unsigned char max)
{
	unsigned char r = rand() % (max - min) + min;
	return r;
}

//random float
float rfloat(float min, float max)
{
	//https://stackoverflow.com/questions/686353/random-float-number-generation
	float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	return r3;
}



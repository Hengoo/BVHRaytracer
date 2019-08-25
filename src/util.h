#pragma once

#include <algorithm>
#include <vector>
#include <array>

//for gamma
#ifdef _MSC_VER
#define MachineEpsilon (std::numeric_limits<float>::epsilon() * 0.5)
#else
static PBRT_CONSTEXPR float MachineEpsilon =
std::numeric_limits<float>::epsilon() * 0.5;
#endif

//random int
static int rint(int min, int max)
{
	int r = rand() % (max - min) + min;
	return r;
}

//random unsigned char (for colors)
static unsigned char ruchar(unsigned char min, unsigned char max)
{
	unsigned char r = rand() % (max - min) + min;
	return r;
}

//random float
static float rfloat(float min, float max)
{
	//https://stackoverflow.com/questions/686353/random-float-number-generation
	float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	return r3;
}

static  inline float gamma(int n)
{
https://github.com/mmp/pbrt-v3/blob/master/src/core/pbrt.h
	return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

static  inline void scaleColor(std::array<unsigned char, 4> & c1, std::array<unsigned char, 4> & c2)
{
	c1[0] *= ((float)c2[0]) * (1.f / 255.f);
	c1[1] *= ((float)c2[1]) * (1.f / 255.f);
	c1[2] *= ((float)c2[2]) * (1.f / 255.f);
	c1[3] *= ((float)c2[3]) * (1.f / 255.f);
}

static inline void scaleColor(std::array<unsigned char, 4> & c1, float& c2)
{
	c1[0] *= c2;
	c1[1] *= c2;
	c1[2] *= c2;
}
#pragma once

#include <algorithm>
#include <vector>
#include <array>
#include "lodepng/lodepng.h"

#include <iostream>

#include <iomanip>
#include <sstream>
//for gamma
#ifdef _MSC_VER
#define MachineEpsilon (std::numeric_limits<float>::epsilon() * 0.5)
#else
static PBRT_CONSTEXPR float MachineEpsilon =
std::numeric_limits<float>::epsilon() * 0.5;
#endif

//static is needed to prevent double declaration (global function ... c++ ...)

//random int
static int rint(int min, int max)
{
	int r = rand() % (max - min) + min;
	return r;
}

//random unsigned uint8 (for colors)
static uint8_t rUint8(uint8_t min, uint8_t max)
{
	uint8_t r = rand() % (max - min) + min;
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
	//https://github.com/mmp/pbrt-v3/blob/master/src/core/pbrt.h
	return (float)((n * MachineEpsilon) / (1 - n * MachineEpsilon));
}

//Encode from raw pixels to an in - memory PNG file first, then write it to disk
//The image argument has width * height RGBA pixels or width * height * 4 bytes
static void encodeTwoSteps(std::string encodeFilename, std::vector<uint8_t>& encodeImage, unsigned encodeWidth, unsigned encodeHeight)
{
	std::vector<uint8_t> png;

	unsigned error = lodepng::encode(png, encodeImage, encodeWidth, encodeHeight);
	if (!error) lodepng::save_file(png, encodeFilename);

	//if there's an error, display it
	if (error) std::cerr << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

static std::string customToString(double d, int digits)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(digits) << d;
	return ss.str();
}

static std::string customToString(float f, int digits)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(digits) << f;
	return ss.str();
}

//some config managing stuff:
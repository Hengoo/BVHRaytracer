#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <array>

#include "glmInclude.h"

class Texture
{
private:
	int height, width;
	std::vector<unsigned char> colors;

public:
	std::array<unsigned char, 4> getColor(glm::vec2 uv)
	{
		//for now super ugly without interpolation:
		int u = (int)(uv.x * width);
		int v = (int)(uv.y * height);
		int index = (u + v * width) * 4;
		return { colors[index], colors[index + 1], colors[index + 2], colors[index + 3] };
	}

	//the texture is copied ONCE
	Texture(int width, int height, std::vector<unsigned char> colors)
		: width(width), height(height), colors(colors)
	{

	}

};
#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <array>

#include "glmInclude.h"
#include "color.h"

class Texture
{
private:
	const size_t height, width;
	const std::vector<uint8_t> colors;

public:
	Color getColor(const glm::vec2& uv)
	{
		//for now super ugly without interpolation:
		size_t u = (int)(uv.x * width) % width;
		size_t v = (int)(uv.y * height) % height;
		size_t index = (u + v * width) * 4;
		//dex = index % (colors.size()-4);
		return Color(colors[index], colors[index + 1], colors[index + 2], colors[index + 3]);
	}

	//the texture is copied once at creation
	Texture(size_t width, size_t height, std::vector<uint8_t> colors)
		: width(width), height(height), colors(colors)
	{

	}

};
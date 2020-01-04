#pragma once
#include <string>
#include <iostream>


static inline void readInt(const std::string& line, const std::string& name, int& result)
{
	auto res = line.find(name, 0);
	if (res != std::string::npos)
	{
		result = std::stoi(line.substr(line.find("=") + 1));
		return;
	}
};
static inline void readInt(const std::string& line, const std::string& name, unsigned& result)
{
	auto res = line.find(name, 0);
	if (res != std::string::npos)
	{
		result = std::stoi(line.substr(line.find("=") + 1));
		return;
	}
};
static inline void readFloat(const std::string& line, const std::string& name, float& result)
{
	auto res = line.find(name, 0);
	if (res != std::string::npos)
	{
		result = std::stof(line.substr(line.find("=") + 1));
		return;
	}
};
static inline void readBool(const std::string& line, const std::string& name, bool& result)
{
	auto res = line.find(name, 0);
	if (res != std::string::npos)
	{
		res = line.find("true", 0);
		auto res2 = line.find("false", 0);
		if (res != std::string::npos)
		{
			result = true;
			return;
		}
		else if (res2 != std::string::npos)
		{
			result = false;
			return;
		}
		else
		{
			std::cerr << name << "value written wrong -> default = false" << std::endl;
			result = false;
			return;
		}
	}
};
//reads two ints (branching factor and leafsize) and then multiple floats separated by ", "
static inline std::vector<float> readConfigLine(const std::string& line, int& nodeSize, int& leafSize)
{
	//first read the two ints:
	std::vector<float> result;
	int length = line.length();
	nodeSize = std::stoi(line.substr(0, length));
	int j = line.find(",") + 1;
	leafSize = std::stoi(line.substr(j + 1, length));
	j = line.find(",", j) + 1;

	while (j > 0) {
		result.push_back(std::stof(line.substr(j + 1, length)));
		j = line.find(",", j) + 1;
	}
	if (result.empty())
	{
		std::cerr << "readConfigLine was not able to read any values after branchingfactor and leafsize" << std::endl;
	}
	return result;
}
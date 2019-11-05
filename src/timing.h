#pragma once

//includes for the timer
#include <ctime>
#include <ratio>
#include <chrono>

static std::chrono::high_resolution_clock::time_point getTime()
{
	return std::chrono::high_resolution_clock::now();
}

static auto getTimeSpan(std::chrono::high_resolution_clock::time_point begin, std::chrono::high_resolution_clock::time_point end)
{
	std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
	return timeSpan.count();
}

static auto getTimeSpan(std::chrono::high_resolution_clock::time_point begin)
{
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
	return timeSpan.count();
}

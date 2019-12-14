#pragma once

//includes for the timer
#include <ctime>
#include <ratio>
#include <chrono>

typedef std::chrono::nanoseconds nanoSec;

inline static std::chrono::high_resolution_clock::time_point getTime()
{
	return std::chrono::high_resolution_clock::now();
}

inline static nanoSec getTimeSpan(const std::chrono::high_resolution_clock::time_point& begin)
{
	auto end = std::chrono::high_resolution_clock::now();
	return end - begin;
}

inline static auto getTimeDouble(nanoSec& time)
{
	std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time);
	return timeSpan.count();
}

inline static auto getTimeFloat(nanoSec& time)
{
	std::chrono::duration<float> timeSpan = std::chrono::duration_cast<std::chrono::duration<float>>(time);
	return timeSpan.count();
}

inline static auto getTimeFloat(const std::chrono::high_resolution_clock::time_point& begin)
{
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> timeSpan = std::chrono::duration_cast<std::chrono::duration<float>>(end - begin);
	return timeSpan.count();
}
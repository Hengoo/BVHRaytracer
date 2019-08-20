#pragma once
#include <iostream>
#include <vector>

#include "../ray.h"

class Primitive
{
public:

	Primitive()
	{
	}

	~Primitive()
	{
	}

	bool virtual intersect(std::shared_ptr<Ray>)
	{
		std::cout << "normal primitive intersect -> should not be called" <<std::endl;
		return false;
	}

protected:



};
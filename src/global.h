#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

#include "light.h"
#include "accelerationStructure/bvh.h"

//file for global variables lie bvh and lights
//Those are the same for all rays and objects for the current rendered image

//needed to spawn shadowrays
extern std::vector<Light> lights;

//needed for secondary rays and shadow rays
extern Bvh bvh;
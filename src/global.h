#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

//class Light;
#include "lights/light.h"
#include "accelerationStructure/bvh.h"

//file for global variables lie bvh and lights
//Those are the same for all rays and objects for the current rendered image

//needed to spawn shadowrays (it kinda feels wrong to have unique ptr with global access?)
extern std::vector < std::unique_ptr<Light>> lights;

//needed for secondary rays and shadow rays
extern Bvh bvh;
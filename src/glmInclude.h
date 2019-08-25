#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_XYZW_ONLY


//#define GLM_FORCE_PURE

#define GLM_FORCE_INTRINSICS


//#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_SWIZZLE#

//fore inline seems slightly faster ?
//#define GLM_FORCE_INLINE 
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //https://glm.g-truc.net/0.9.4/api/a00158.html  for make quat from a pointer to a array
#include <glm/gtx/hash.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/component_wise.hpp>

//not sure if i want to keept his?
#include <glm/gtx/intersect.hpp>
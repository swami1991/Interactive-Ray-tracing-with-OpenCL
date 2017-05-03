//
//  struct.hpp
//  Real time path tracer
//
//  Created by Swaminathan Ramachandran on 3/20/17.
//
//

#ifndef struct_hpp
#define struct_hpp

#include <stdio.h>
#include <iostream>

#include "glew.h"
#include "glfw3.h"
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>

#include "glm/glm.hpp"

#include "scene.hpp"
#include "grids.hpp"
#include "bvh.hpp"

#define PIXELS_Y 512
#define PIXELS_X 512

#define USE_GRID 0
#define USE_BVH 1

using namespace std;

#endif /* struct_hpp */

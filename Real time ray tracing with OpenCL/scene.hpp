//
//  scene.hpp
//  Real time ray tracing with OpenCL
//
//  Created by Swaminathan Ramachandran on 4/5/17.
//  Copyright © 2017 Swaminathan Ramachandran. All rights reserved.
//

#ifndef scene_hpp
#define scene_hpp

#include <stdio.h>
#include <iostream>
#include <OpenCL/OpenCL.h>
#include "glm/glm.hpp"
#include <vector>
#include "objloader.hpp"
#include <math.h>

using namespace std;

void addCornellBox(vector<glm::vec3> *verts, vector<glm::vec3> *norms, vector<glm::vec3> *color);
void loadScene(vector<glm::vec3> &verts, vector<glm::vec3> &norms, vector<glm::vec3> &colors, cl_float3 **v, cl_float3 **n, cl_float3 **c);

#endif /* scene_hpp */

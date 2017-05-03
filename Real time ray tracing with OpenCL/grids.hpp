//
//  grids.hpp
//  Real time ray tracing with OpenCL
//
//  Created by Swaminathan Ramachandran on 3/28/17.
//  Copyright © 2017 Swaminathan Ramachandran. All rights reserved.
//

#ifndef grids_hpp
#define grids_hpp

#include <iostream>
#include <stdlib.h>
#include "glm/glm.hpp"
#include <vector>
#include <math.h>
#include <OpenCL/OpenCL.h>

typedef struct BoundingVolume{
    cl_float xl, yl, zl;
    cl_float xh, yh, zh;
    
    BoundingVolume(){
        xl = yl = zl = 1000;
        xh = yh = zh = -1000;
    }
}BV;

typedef struct Grid{
    public:
    cl_uint3 *ids;
    cl_uint *size;
    
    cl_uint nx,ny,nz;
    cl_float wx,wy,wz;
    
    cl_uint arr_size;
    cl_uint3 grid_dim;
    cl_float3 voxel_w, grid_origin;
    
    struct BoundingVolume full_bv;
    std::vector<struct BoundingVolume> obj_bv;
    
    void createGrid(std::vector<glm::vec3> verts_in);
    
    void printGrid();
}Grid;

#endif /* grids_hpp */

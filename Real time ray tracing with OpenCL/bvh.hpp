//
//  bvh.hpp
//  Real time ray tracing with OpenCL
//
//  Created by Swaminathan Ramachandran on 4/11/17.
//  Copyright © 2017 Swaminathan Ramachandran. All rights reserved.
//

#ifndef bvh_hpp
#define bvh_hpp

#include <stdio.h>
#include <iostream>
#include <OpenCL/opencl.h>
#include <vector>
#include "glm/glm.hpp"

using namespace std;

struct BBox{
    unsigned int tri_idx;
    glm::vec3 high;
    glm::vec3 low;
    glm::vec3 center;
};

struct BVHNode{
    glm::vec3 low;
    glm::vec3 high;
    
    virtual bool isLeafNode() = 0;
};

struct BVHInnerNode: BVHNode{
    BVHNode *left;
    BVHNode *right;
    
    virtual bool isLeafNode(){return false;}
};

struct BVHLeafNode: BVHNode{
    vector<uint> tri_idx;
    
    virtual bool isLeafNode(){return true;}
};

struct GPU_BVHNode{
    cl_float3 low;
    cl_float3 high;
    
    union{
        struct {
            uint idxLeft;
            uint idxRight;
        }inner;
        struct {
            uint count;
            uint triStartIdx;
        }leaf;
    }u;
};

void CreateBVH_Texture(struct GPU_BVHNode *BVH,cl_float4 *bvh_lower,cl_float4 *bvh_higher, cl_uint4 *bvh_nodes, uint size);
void PrintBVH(struct GPU_BVHNode *BVH, int size, cl_float3 *verts);
void RecursiveCreateBVH_GPU(struct GPU_BVHNode *OutputArr, struct BVHNode *root, cl_float3 *verts, cl_float3 *norms, cl_float3 *colors, vector<glm::vec3> &verts_in, vector<glm::vec3> &norms_in, vector<glm::vec3> &colors_in, unsigned int &node_id, unsigned int &tri_id);
struct GPU_BVHNode* CreateBVH_GPU(struct BVHNode *root, cl_float3 *verts, cl_float3 *norms, cl_float3 *colors, vector<glm::vec3> &verts_in, vector<glm::vec3> &norms_in, vector<glm::vec3> &colors_in, uint &size);
void findBounds(BVHNode *Node, vector<struct BBox*> &BBoxList);
float findCost(vector<struct BBox*> &BBoxList);
struct BVHNode* RecursiveCreateBVH(vector<struct BBox*> &BBoxList, int iter);
struct BVHNode* CreateBVH(vector<glm::vec3> &verts);

#endif /* bvh_hpp */

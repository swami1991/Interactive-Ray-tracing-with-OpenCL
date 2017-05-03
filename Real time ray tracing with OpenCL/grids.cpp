//
//  grids.cpp
//  Real time ray tracing with OpenCL
//
//  Created by Swaminathan Ramachandran on 3/28/17.
//  Copyright © 2017 Swaminathan Ramachandran. All rights reserved.
//

#include "grids.hpp"

using namespace std;

void Grid::createGrid(std::vector<glm::vec3> verts_in){
    obj_bv.resize(verts_in.size()/3);
    for(int i=0; i<verts_in.size();i=i+3){
        //calculate lower,upper bounds
        if(verts_in[i].x < full_bv.xl) full_bv.xl = verts_in[i].x;
        if(verts_in[i].y < full_bv.yl) full_bv.yl = verts_in[i].y;
        if(verts_in[i].z < full_bv.zl) full_bv.zl = verts_in[i].z;
        if(verts_in[i].x > full_bv.xh) full_bv.xh = verts_in[i].x;
        if(verts_in[i].y > full_bv.yh) full_bv.yh = verts_in[i].y;
        if(verts_in[i].z > full_bv.zh) full_bv.zh = verts_in[i].z;
        
        if(verts_in[i+1].x < full_bv.xl) full_bv.xl = verts_in[i+1].x;
        if(verts_in[i+1].y < full_bv.yl) full_bv.yl = verts_in[i+1].y;
        if(verts_in[i+1].z < full_bv.zl) full_bv.zl = verts_in[i+1].z;
        if(verts_in[i+1].x > full_bv.xh) full_bv.xh = verts_in[i+1].x;
        if(verts_in[i+1].y > full_bv.yh) full_bv.yh = verts_in[i+1].y;
        if(verts_in[i+1].z > full_bv.zh) full_bv.zh = verts_in[i+1].z;
        
        if(verts_in[i+2].x < full_bv.xl) full_bv.xl = verts_in[i+2].x;
        if(verts_in[i+2].y < full_bv.yl) full_bv.yl = verts_in[i+2].y;
        if(verts_in[i+2].z < full_bv.zl) full_bv.zl = verts_in[i+2].z;
        if(verts_in[i+2].x > full_bv.xh) full_bv.xh = verts_in[i+2].x;
        if(verts_in[i+2].y > full_bv.yh) full_bv.yh = verts_in[i+2].y;
        if(verts_in[i+2].z > full_bv.zh) full_bv.zh = verts_in[i+2].z;
        
        if(verts_in[i].x < obj_bv[i/3].xl) obj_bv[i/3].xl = verts_in[i].x;
        if(verts_in[i].y < obj_bv[i/3].yl) obj_bv[i/3].yl = verts_in[i].y;
        if(verts_in[i].z < obj_bv[i/3].zl) obj_bv[i/3].zl = verts_in[i].z;
        if(verts_in[i].x > obj_bv[i/3].xh) obj_bv[i/3].xh = verts_in[i].x;
        if(verts_in[i].y > obj_bv[i/3].yh) obj_bv[i/3].yh = verts_in[i].y;
        if(verts_in[i].z > obj_bv[i/3].zh) obj_bv[i/3].zh = verts_in[i].z;
        
        if(verts_in[i+1].x < obj_bv[i/3].xl) obj_bv[i/3].xl = verts_in[i+1].x;
        if(verts_in[i+1].y < obj_bv[i/3].yl) obj_bv[i/3].yl = verts_in[i+1].y;
        if(verts_in[i+1].z < obj_bv[i/3].zl) obj_bv[i/3].zl = verts_in[i+1].z;
        if(verts_in[i+1].x > obj_bv[i/3].xh) obj_bv[i/3].xh = verts_in[i+1].x;
        if(verts_in[i+1].y > obj_bv[i/3].yh) obj_bv[i/3].yh = verts_in[i+1].y;
        if(verts_in[i+1].z > obj_bv[i/3].zh) obj_bv[i/3].zh = verts_in[i+1].z;
        
        if(verts_in[i+2].x < obj_bv[i/3].xl) obj_bv[i/3].xl = verts_in[i+2].x;
        if(verts_in[i+2].y < obj_bv[i/3].yl) obj_bv[i/3].yl = verts_in[i+2].y;
        if(verts_in[i+2].z < obj_bv[i/3].zl) obj_bv[i/3].zl = verts_in[i+2].z;
        if(verts_in[i+2].x > obj_bv[i/3].xh) obj_bv[i/3].xh = verts_in[i+2].x;
        if(verts_in[i+2].y > obj_bv[i/3].yh) obj_bv[i/3].yh = verts_in[i+2].y;
        if(verts_in[i+2].z > obj_bv[i/3].zh) obj_bv[i/3].zh = verts_in[i+2].z;
    }
    
    wx = full_bv.xh-full_bv.xl; wy = full_bv.yh-full_bv.yl; wz = full_bv.zh-full_bv.zl;
    
    float_t s = wx*wy*wz*3/verts_in.size();
    s = cbrt(s);
    nx = ceil(wx/s);
    ny = ceil(wy/s);
    nz = ceil(wz/s);
    
    wx = wx/nx; wy = wy/ny; wz = wz/nz;
    
    //create grid
    vector<cl_uint3> *temp_ids = new vector<cl_uint3>[nx*ny*nz];
    //allocate each object to a grid
    for(uint i=0;i<verts_in.size()/3;i++){
        glm::vec3 idx_l, idx_h;
        
        idx_l.x = (uint)((obj_bv[i].xl-full_bv.xl)/wx);
        idx_l.y = (uint)((obj_bv[i].yl-full_bv.yl)/wy);
        idx_l.z = (uint)((obj_bv[i].zl-full_bv.zl)/wz);
        if(idx_l.x == nx) idx_l.x--;
        if(idx_l.y == ny) idx_l.y--;
        if(idx_l.z == nz) idx_l.z--;
        
        idx_h.x = (uint)((obj_bv[i].xh-full_bv.xl)/wx);
        idx_h.y = (uint)((obj_bv[i].yh-full_bv.yl)/wy);
        idx_h.z = (uint)((obj_bv[i].zh-full_bv.zl)/wz);
        if(idx_h.x == nx) idx_h.x--;
        if(idx_h.y == ny) idx_h.y--;
        if(idx_h.z == nz) idx_h.z--;
        
        for(uint k=idx_l.x;k<=idx_h.x;k++){
            for(uint l=idx_l.y;l<=idx_h.y;l++){
                for(uint m=idx_l.z;m<=idx_h.z;m++){
                    cl_uint3 id;
                    id.x=3*i; id.y=3*i+1; id.z=3*i+2;
                    temp_ids[k*ny*nz+l*nz+m].push_back(id);
                }
            }
        }
    }
    
    int temp_size = 0;
    for(int i=0;i<nx*ny*nz;i++){
        temp_size += (int)temp_ids[i].size();
    }
    
    size = new cl_uint[nx*ny*nz];
    ids = new cl_uint3[temp_size];
    int k = 0;
    for(int i=0;i<nx*ny*nz;i++){
        size[i] = k;
        for(int j=0;j<temp_ids[i].size();j++){
            ids[k++] = temp_ids[i][j];
        }
    }
    assert(k == temp_size);
    
    arr_size = k;
    grid_dim.x = nx; grid_dim.y = ny; grid_dim.z = nz;
    grid_origin.x = full_bv.xl; grid_origin.y = full_bv.yl; grid_origin.z = full_bv.zl;
    voxel_w.x = wx; voxel_w.y = wy; voxel_w.z = wz;
}

void Grid::printGrid(){
    cout<<"nx,ny,nz = "<<nx<<" "<<ny<<" "<<nz<<endl;
    cout<<"wx,wy,wz = "<<wx<<" "<<wy<<" "<<wz<<endl;
    for(int i=0;i<arr_size;i++)
        cout<<ids[i].x<<" "<<ids[i].y<<" "<<ids[i].z<<" ";
    cout<<" "<<endl;
}

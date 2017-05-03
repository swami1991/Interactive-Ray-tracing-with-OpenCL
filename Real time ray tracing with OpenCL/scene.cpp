//
//  scene.cpp
//  Real time ray tracing with OpenCL
//
//  Created by Swaminathan Ramachandran on 4/5/17.
//  Copyright © 2017 Swaminathan Ramachandran. All rights reserved.
//

#include "scene.hpp"

void addCornellBox(vector<glm::vec3> *verts, vector<glm::vec3> *norms, vector<glm::vec3> *color){
    verts->push_back(glm::vec3(0.0,0.0,0.0)); norms->push_back(glm::vec3(1.0,0.0,0.0)); color->push_back(glm::vec3(1.0,0.0,0.0));
    verts->push_back(glm::vec3(0.0,1.0,0.0)); norms->push_back(glm::vec3(1.0,0.0,0.0)); color->push_back(glm::vec3(1.0,0.0,0.0));
    verts->push_back(glm::vec3(0.0,0.0,1.0)); norms->push_back(glm::vec3(1.0,0.0,0.0)); color->push_back(glm::vec3(1.0,0.0,0.0));
    
    
    verts->push_back(glm::vec3(0.0,1.0,0.0)); norms->push_back(glm::vec3(1.0,0.0,0.0)); color->push_back(glm::vec3(1.0,0.0,0.0));
    verts->push_back(glm::vec3(0.0,1.0,1.0)); norms->push_back(glm::vec3(1.0,0.0,0.0)); color->push_back(glm::vec3(1.0,0.0,0.0));
    verts->push_back(glm::vec3(0.0,0.0,1.0)); norms->push_back(glm::vec3(1.0,0.0,0.0)); color->push_back(glm::vec3(1.0,0.0,0.0));
    
    verts->push_back(glm::vec3(1.0,0.0,0.0)); norms->push_back(glm::vec3(-1.0,0.0,0.0)); color->push_back(glm::vec3(0.0,1.0,0.0));
    verts->push_back(glm::vec3(1.0,1.0,0.0)); norms->push_back(glm::vec3(-1.0,0.0,0.0)); color->push_back(glm::vec3(0.0,1.0,0.0));
    verts->push_back(glm::vec3(1.0,0.0,1.0)); norms->push_back(glm::vec3(-1.0,0.0,0.0)); color->push_back(glm::vec3(0.0,1.0,0.0));
    
    verts->push_back(glm::vec3(1.0,1.0,0.0)); norms->push_back(glm::vec3(-1.0,0.0,0.0)); color->push_back(glm::vec3(0.0,1.0,0.0));
    verts->push_back(glm::vec3(1.0,1.0,1.0)); norms->push_back(glm::vec3(-1.0,0.0,0.0)); color->push_back(glm::vec3(0.0,1.0,0.0));
    verts->push_back(glm::vec3(1.0,0.0,1.0)); norms->push_back(glm::vec3(-1.0,0.0,0.0)); color->push_back(glm::vec3(0.0,1.0,0.0));
    
    verts->push_back(glm::vec3(0.0,0.0,1.0)); norms->push_back(glm::vec3(0.0,0.0,-1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,0.0,1.0)); norms->push_back(glm::vec3(0.0,0.0,-1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(0.0,1.0,1.0)); norms->push_back(glm::vec3(0.0,0.0,-1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    
    verts->push_back(glm::vec3(0.0,1.0,1.0)); norms->push_back(glm::vec3(0.0,0.0,-1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,0.0,1.0)); norms->push_back(glm::vec3(0.0,0.0,-1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,1.0,1.0)); norms->push_back(glm::vec3(0.0,0.0,-1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    
    verts->push_back(glm::vec3(0.0,1.0,1.0)); norms->push_back(glm::vec3(0.0,-1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,1.0,1.0)); norms->push_back(glm::vec3(0.0,-1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,1.0,0.0)); norms->push_back(glm::vec3(0.0,-1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    
    verts->push_back(glm::vec3(0.0,1.0,1.0)); norms->push_back(glm::vec3(0.0,-1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,1.0,0.0)); norms->push_back(glm::vec3(0.0,-1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(0.0,1.0,0.0)); norms->push_back(glm::vec3(0.0,-1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    
    verts->push_back(glm::vec3(1.0,0.0,1.0)); norms->push_back(glm::vec3(0.0,1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(0.0,0.0,0.0)); norms->push_back(glm::vec3(0.0,1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,0.0,0.0)); norms->push_back(glm::vec3(0.0,1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    
    verts->push_back(glm::vec3(0.0,0.0,1.0)); norms->push_back(glm::vec3(0.0,1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(0.0,0.0,0.0)); norms->push_back(glm::vec3(0.0,1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    verts->push_back(glm::vec3(1.0,0.0,1.0)); norms->push_back(glm::vec3(0.0,1.0,0.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
    
    /* verts->push_back(glm::vec3(0.0,1.0,0.0)); norms->push_back(glm::vec3(0.0,0.0,1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
     verts->push_back(glm::vec3(0.0,0.0,0.0)); norms->push_back(glm::vec3(0.0,0.0,1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
     verts->push_back(glm::vec3(1.0,0.0,0.0)); norms->push_back(glm::vec3(0.0,0.0,1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
     
     verts->push_back(glm::vec3(1.0,1.0,0.0)); norms->push_back(glm::vec3(0.0,0.0,1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
     verts->push_back(glm::vec3(0.0,1.0,0.0)); norms->push_back(glm::vec3(0.0,0.0,1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));
     verts->push_back(glm::vec3(1.0,0.0,0.0)); norms->push_back(glm::vec3(0.0,0.0,1.0)); color->push_back(glm::vec3(0.5,0.5,0.5));*/
}

void createSphere(vector<glm::vec3> &verts, vector<glm::vec3> &norms, vector<glm::vec3> &colors, glm::vec3 center, float radius, glm::vec3 color){
    vector<glm::vec3> vertices, normals;
    float xc = center.x;
    float yc = center.y;
    float zc = center.z;
    float r = radius;
    float numLat = 50, numLon = 50;
    for(int lat=0;lat<=numLat;lat++){
        float theta = lat*3.14/numLat;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        
        for(int lon=0;lon<=numLon;lon++){
            float phi = lon*2*3.14/numLon;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            
            float x = cosPhi*sinTheta;
            float y = cosTheta;
            float z = sinPhi*sinTheta;
            
            vertices.push_back(glm::vec3(xc+r*x,yc+r*y,zc+r*z));
            normals.push_back(glm::vec3(x,y,z));
        }
    }
    
    for(int lat=0;lat<numLat;lat++){
        for(int lon=0;lon<numLon;lon++){
            unsigned int first = lat*(numLon+1)+lon;
            unsigned int second = first + numLon + 1;
            
            verts.push_back(vertices[first]);
            verts.push_back(vertices[second]);
            verts.push_back(vertices[first+1]);
            verts.push_back(vertices[second]);
            verts.push_back(vertices[second+1]);
            verts.push_back(vertices[first+1]);
            
            norms.push_back(normals[first]);
            norms.push_back(normals[second]);
            norms.push_back(normals[first+1]);
            norms.push_back(normals[second]);
            norms.push_back(normals[second+1]);
            norms.push_back(normals[first+1]);
            
            colors.push_back(color);
            colors.push_back(color);
            colors.push_back(color);
            colors.push_back(color);
            colors.push_back(color);
            colors.push_back(color);
            
        }
    }
}

void loadScene(vector<glm::vec3> &verts, vector<glm::vec3> &norms, vector<glm::vec3> &colors, cl_float3 **v, cl_float3 **n, cl_float3 **c){
    addCornellBox(&verts,&norms,&colors);
    uint size1 = (uint)verts.size();
    
    for(int i=0;i<size1;i++){
        verts[i].y -= 0.5;
        verts[i].x -= 0.5;
    }
    
    createSphere(verts,norms,colors,glm::vec3(-0.2,0,0.5),0.2,glm::vec3(0,0,1));
    createSphere(verts,norms,colors,glm::vec3(0.2,0.3,0.5),0.1,glm::vec3(0,1,1));
    //createSphere(verts,norms,colors,glm::vec3(0.2,-0.2,0.7),0.1,glm::vec3(0,0,1));
    
    *v = new cl_float3[verts.size()];
    *n = new cl_float3[verts.size()];
    *c = new cl_float3[verts.size()];
    
    cout<<verts.size()/3<<" triangles to be rendered"<<endl;
}

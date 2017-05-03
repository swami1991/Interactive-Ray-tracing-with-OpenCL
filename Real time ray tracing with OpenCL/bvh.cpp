//
//  bvh.cpp
//  Real time ray tracing with OpenCL
//
//  Created by Swaminathan Ramachandran on 4/11/17.
//  Copyright © 2017 Swaminathan Ramachandran. All rights reserved.
//

#include "bvh.hpp"

void CreateBVH_Texture(struct GPU_BVHNode *BVH,cl_float4 *bvh_lower,cl_float4 *bvh_higher, cl_uint4 *bvh_nodes, uint size){
    
    for(uint i=0;i<size;i++){
        bvh_lower[i].x = BVH[i].low.x; bvh_lower[i].y = BVH[i].low.y; bvh_lower[i].z = BVH[i].low.z;
        bvh_higher[i].x = BVH[i].high.x; bvh_higher[i].y = BVH[i].high.y; bvh_higher[i].z = BVH[i].high.z;
        bvh_nodes[i].x = BVH[i].u.inner.idxLeft; bvh_nodes[i].y = BVH[i].u.inner.idxRight;
        bvh_nodes[i].z = BVH[i].u.leaf.count; bvh_nodes[i].w = BVH[i].u.leaf.triStartIdx;
    }
}

void PrintBVH(struct GPU_BVHNode *BVH, int size, cl_float3 *verts){
    for(int i=0;i<size;i++){
        cout<<"Node "<<i<<endl;
        cout<<"    high: "<<BVH[i].high.x<<" "<<BVH[i].high.y<<" "<<BVH[i].high.z<<endl;
        cout<<"    low:  "<<BVH[i].low.x<<" "<<BVH[i].low.y<<" "<<BVH[i].low.z<<endl;
        if(BVH[i].u.leaf.count&0x80000000){
            cout<<"    Leaf node"<<endl;
            for(int j=0;j<(BVH[i].u.leaf.count&0x7fffffff);j++){
                cout<<verts[BVH[i].u.leaf.triStartIdx+j].x<<" "<<verts[BVH[i].u.leaf.triStartIdx+j].y<<" "<<verts[BVH[i].u.leaf.triStartIdx+j].z<<endl;
                cout<<verts[BVH[i].u.leaf.triStartIdx+j+1].x<<" "<<verts[BVH[i].u.leaf.triStartIdx+j+1].y<<" "<<verts[BVH[i].u.leaf.triStartIdx+j+1].z<<endl;
                cout<<verts[BVH[i].u.leaf.triStartIdx+j+2].x<<" "<<verts[BVH[i].u.leaf.triStartIdx+j+2].y<<" "<<verts[BVH[i].u.leaf.triStartIdx+j+2].z<<endl;
            }
        }else{
            cout<<"    Inner node"<<endl;
            cout<<"    Left "<<BVH[i].u.inner.idxLeft<<" Right "<<BVH[i].u.inner.idxRight<<endl;
        }
    }
}

uint getTri(struct BVHNode* root){
    if(root->isLeafNode()){
        struct BVHLeafNode *node = dynamic_cast<struct BVHLeafNode*>(root);
        return (uint)node->tri_idx.size();
    }else{
        struct BVHInnerNode *node = dynamic_cast<struct BVHInnerNode*>(root);
        return getTri(node->left) + getTri(node->right);
    }
}

uint getnumNodes(struct BVHNode* root){
    if(root->isLeafNode()){
        return 1;
    }else{
        struct BVHInnerNode *node = dynamic_cast<struct BVHInnerNode*>(root);
        return 1+ getnumNodes(node->left) + getnumNodes(node->right);
    }
}

void RecursiveCreateBVH_GPU(struct GPU_BVHNode *OutputArr, struct BVHNode *root, cl_float3 *verts, cl_float3 *norms, cl_float3 *colors, vector<glm::vec3> &verts_in, vector<glm::vec3> &norms_in, vector<glm::vec3> &colors_in, unsigned int &node_id, unsigned int &tri_id){
    uint cur_id = node_id;
    OutputArr[cur_id].high.x = root->high.x; OutputArr[cur_id].high.y = root->high.y; OutputArr[cur_id].high.z = root->high.z;
    OutputArr[cur_id].low.x = root->low.x; OutputArr[cur_id].low.y = root->low.y; OutputArr[cur_id].low.z = root->low.z;
    
    if(!root->isLeafNode()){
        struct BVHInnerNode *node = dynamic_cast<struct BVHInnerNode*>(root);
        int idxLeft = ++node_id;
        RecursiveCreateBVH_GPU(OutputArr, node->left, verts, norms, colors, verts_in, norms_in, colors_in, node_id, tri_id);
        int idxRight = ++node_id;
        RecursiveCreateBVH_GPU(OutputArr, node->right, verts, norms, colors, verts_in, norms_in, colors_in, node_id, tri_id);
        OutputArr[cur_id].u.inner.idxLeft = idxLeft;
        OutputArr[cur_id].u.inner.idxRight = idxRight;
    }else{
        struct BVHLeafNode *node = dynamic_cast<struct BVHLeafNode*>(root);
        uint count = (uint)node->tri_idx.size();
        OutputArr[cur_id].u.leaf.count = 0x80000000 | count;
        OutputArr[cur_id].u.leaf.triStartIdx = tri_id;
        
        for(uint i=0;i<count;i++){
            uint id = node->tri_idx[i];
            
            verts[tri_id+3*i].w = (id/3>=0)?1:2;
            verts[tri_id+3*i].x = verts_in[id].x; verts[tri_id+3*i].y = verts_in[id].y; verts[tri_id+3*i].z = verts_in[id].z;
            verts[tri_id+3*i+1].x = verts_in[id+1].x; verts[tri_id+3*i+1].y = verts_in[id+1].y; verts[tri_id+3*i+1].z = verts_in[id+1].z;
            verts[tri_id+3*i+2].x = verts_in[id+2].x; verts[tri_id+3*i+2].y = verts_in[id+2].y; verts[tri_id+3*i+2].z = verts_in[id+2].z;
            
            norms[tri_id+3*i].x = norms_in[id].x; norms[tri_id+3*i].y = norms_in[id].y; norms[tri_id+3*i].z = norms_in[id].z;
            norms[tri_id+3*i+1].x = norms_in[id+1].x; norms[tri_id+3*i+1].y = norms_in[id+1].y; norms[tri_id+3*i+1].z = norms_in[id+1].z;
            norms[tri_id+3*i+2].x = norms_in[id+2].x; norms[tri_id+3*i+2].y = norms_in[id+2].y; norms[tri_id+3*i+2].z = norms_in[id+2].z;
            
            colors[tri_id+3*i].x = colors_in[id].x; colors[tri_id+3*i].y = colors_in[id].y; colors[tri_id+3*i].z = colors_in[id].z;
            colors[tri_id+3*i+1].x = colors_in[id+1].x; colors[tri_id+3*i+1].y = colors_in[id+1].y; colors[tri_id+3*i+1].z = colors_in[id+1].z;
            colors[tri_id+3*i+2].x = colors_in[id+2].x; colors[tri_id+3*i+2].y = colors_in[id+2].y; colors[tri_id+3*i+2].z = colors_in[id+2].z;
        }
        tri_id = tri_id + 3*count;
    }
}

struct GPU_BVHNode* CreateBVH_GPU(struct BVHNode *root, cl_float3 *verts, cl_float3 *norms, cl_float3 *colors, vector<glm::vec3> &verts_in, vector<glm::vec3> &norms_in, vector<glm::vec3> &colors_in, uint &size){
    uint numTri = getTri(root);
    assert(numTri == verts_in.size()/3);
    
    uint numNodes = getnumNodes(root);
    size = numNodes;
    struct GPU_BVHNode *OutputArr = new GPU_BVHNode[numNodes];
    unsigned int node_id=0, tri_id=0;
    
    RecursiveCreateBVH_GPU(OutputArr, root, verts, norms, colors, verts_in, norms_in, colors_in, node_id, tri_id);
    
    return OutputArr;
}

void findBounds(BVHNode *Node, vector<struct BBox*> &BBoxList){
    Node->high = glm::vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    Node->low = glm::vec3(FLT_MAX,FLT_MAX,FLT_MAX);
    for(uint i=0;i<BBoxList.size();i++){
        Node->high.x = BBoxList[i]->high.x>Node->high.x?BBoxList[i]->high.x:Node->high.x;
        Node->high.y = BBoxList[i]->high.y>Node->high.y?BBoxList[i]->high.y:Node->high.y;
        Node->high.z = BBoxList[i]->high.z>Node->high.z?BBoxList[i]->high.z:Node->high.z;
        
        Node->low.x = BBoxList[i]->low.x<Node->low.x?BBoxList[i]->low.x:Node->low.x;
        Node->low.y = BBoxList[i]->low.y<Node->low.y?BBoxList[i]->low.y:Node->low.y;
        Node->low.z = BBoxList[i]->low.z<Node->low.z?BBoxList[i]->low.z:Node->low.z;
    }
}

float findCost(vector<struct BBox*> &BBoxList){
    struct BVHNode *temp = new struct BVHLeafNode;
    findBounds(temp,BBoxList);
    float Cost = dot(temp->high-temp->low,glm::vec3(1,1,1))*BBoxList.size();
    
    return Cost;
}

BVHNode* RecursiveCreateBVH(vector<struct BBox*> &BBoxList, int iter){
    if(BBoxList.size() <= 4 || iter > 10){
        struct BVHLeafNode *Node = new struct BVHLeafNode;
        findBounds(Node,BBoxList);
        
        for(uint i=0;i<BBoxList.size();i++){
            Node->tri_idx.push_back(BBoxList[i]->tri_idx);
        }
        
        return Node;
    }else{
        struct BVHInnerNode *Node = new struct BVHInnerNode;
        findBounds(Node,BBoxList);
        
        vector<struct BBox*> LeftBBoxList, RightBBoxList;
        int num_bins = 2048/(int)pow(2,iter);
        
        float optimal_cost = FLT_MAX;
        int optimal_axis=-1, optimal_bin=-1;
        for(int axis=0;axis<3;axis++){
            float w,l,h;
            if(axis ==0 ) {l = Node->low.x; h = Node->high.x;}
            else if(axis == 1) {l = Node->low.y; h = Node->high.y;}
            else if(axis == 2) {l = Node->low.z; h = Node->high.z;}
            if(h-l < 0.0001) continue;
            w = (h-l)/num_bins;
            for(int i=0;i<num_bins-1;i++){
                float split = l + w*(i+1);
                LeftBBoxList.resize(0);
                RightBBoxList.resize(0);
                for(int j=0;j<BBoxList.size();j++){
                    float c;
                    if(axis == 0) c = BBoxList[j]->center.x;
                    else if(axis == 1) c = BBoxList[j]->center.y;
                    else if(axis == 2) c = BBoxList[j]->center.z;
                    
                    if(c < split) LeftBBoxList.push_back(BBoxList[j]);
                    else RightBBoxList.push_back(BBoxList[j]);
                }
                
                if(LeftBBoxList.size()==0 || RightBBoxList.size()==0){
                    continue;
                }else{
                    float cost = 0;
                    cost += findCost(LeftBBoxList);
                    cost += findCost(RightBBoxList);
                    
                    if(cost < optimal_cost){
                        optimal_cost = cost;
                        optimal_axis = axis;
                        optimal_bin  = i;
                    }
                }
            }
        }
        if(optimal_cost == FLT_MAX){
            delete Node;
            struct BVHLeafNode *Node1 = new struct BVHLeafNode;
            findBounds(Node1,BBoxList);
            
            for(uint i=0;i<BBoxList.size();i++){
                Node1->tri_idx.push_back(BBoxList[i]->tri_idx);
            }
            
            return Node1;
        }else{
            float w,l,h;
            if(optimal_axis == 0 ) {l = Node->low.x; h = Node->high.x;}
            else if(optimal_axis == 1) {l = Node->low.y; h = Node->high.y;}
            else if(optimal_axis == 2) {l = Node->low.z; h = Node->high.z;}
            w = (h-l)/num_bins;
            
            float split = l + w*(optimal_bin+1);
            LeftBBoxList.resize(0);
            RightBBoxList.resize(0);
            for(int j=0;j<BBoxList.size();j++){
                float c;
                if(optimal_axis == 0) c = BBoxList[j]->center.x;
                else if(optimal_axis == 1) c = BBoxList[j]->center.y;
                else if(optimal_axis == 2) c = BBoxList[j]->center.z;
                
                if(c < split) LeftBBoxList.push_back(BBoxList[j]);
                else RightBBoxList.push_back(BBoxList[j]);
            }
            
            Node->left = RecursiveCreateBVH(LeftBBoxList, iter+1);
            Node->right = RecursiveCreateBVH(RightBBoxList, iter+1);
            
            return Node;
        }
    }
}

BVHNode* CreateBVH(vector<glm::vec3> &verts){
    vector<struct BBox*> BBoxList;
    for(uint i=0;i<verts.size();i=i+3){
        struct BBox *box = new struct BBox;
        
        box->high.x = fmaxf(verts[i].x, fmaxf(verts[i+1].x, verts[i+2].x));
        box->high.y = fmaxf(verts[i].y, fmaxf(verts[i+1].y, verts[i+2].y));
        box->high.z = fmaxf(verts[i].z, fmaxf(verts[i+1].z, verts[i+2].z));
        box->low.x = fminf(verts[i].x, fminf(verts[i+1].x, verts[i+2].x));
        box->low.y = fminf(verts[i].y, fminf(verts[i+1].y, verts[i+2].y));
        box->low.z = fminf(verts[i].z, fminf(verts[i+1].z, verts[i+2].z));
        
        box->center = (box->high + box->low)/2.0f;
        
        box->tri_idx = i;
        
        BBoxList.push_back(box);
    }
    
    BVHNode *root = RecursiveCreateBVH(BBoxList,0);
    
    return root;
}

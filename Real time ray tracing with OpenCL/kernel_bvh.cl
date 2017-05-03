__constant int   NUM_BOUNCES = 2;
__constant int   NUM_SAMPLES = 1;
__constant float PI          = 3.14159265359f;

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST;

struct stack{
    uint entry[20];
    uint head;
};

bool RayTriangleIntersection_bvh(float3 Point, float3 Dir, float3 v0, float3 v1, float3 v2, float3* out){
    bool ret = false;
    float t,u,v;
    float3 v0v1, v0v2;
    v0v1 = v1 - v0;
    v0v2 = v2 - v0;
    
    float3 pvec = cross(Dir,v0v2);
    float det = dot(v0v1,pvec);
    float invdet;
    float3 tvec,qvec;
    bool c1 = (fabs(det) > 0.00000001f)?true:false;
    invdet = c1?1/det:0.0f;
    
    tvec = Point-v0;
    u = dot(tvec,pvec)*invdet;
    
    qvec = cross(tvec,v0v1);
    v = dot(Dir,qvec)*invdet;
    
    bool c2 = c1?((u>=0.0f && u<=1.0f && v>=0.0f && (u+v)<=1.0f)?true:false):false;
    out->x = dot(v0v2,qvec)*invdet;
    out->y = u;
    out->z = v;
    
    return c2;
}

bool intersectLeafNode(uint triStartIdx,
                       uint count,
                       read_only image2d_t verts,
                       read_only image2d_t norms,
                       read_only image2d_t colors,
                       constant uint *img_size,
                       float3 Point, float3 Dir, float ClipVal,
                       float3 *closest_v, float3 *normal, float3 *color, int *index,
                       float *closest_t, bool shadow_calculation){
    float t,u,v;
    bool c1, intersect, ret;
    ret = false;
    uint idx;
    uint size = img_size[1];
    float3 v0,v1,v2,out;
    float3 n0,n1,n2, col0,col1,col2;
    uint x0,y0,x1,y1,x2,y2,material_type;
    for(uint i=0;i<count;i++){
        idx = triStartIdx + 3*i;
        
        x0 = idx%size; y0 = idx/size;
        x1 = (idx+1)%size; y1 = (idx+1)/size;
        x2 = (idx+2)%size; y2 = (idx+2)/size;
        
        material_type = read_imagef(verts,sampler,(int2)(x0,y0)).w;
        v0 = read_imagef(verts,sampler,(int2)(x0,y0)).xyz;
        v1 = read_imagef(verts,sampler,(int2)(x1,y1)).xyz;
        v2 = read_imagef(verts,sampler,(int2)(x2,y2)).xyz;
        
        intersect = RayTriangleIntersection_bvh(Point,Dir,v0,v1,v2,&out);
        t = (intersect)?out.x:FLT_MAX;
        u = (intersect)?out.y:-1;
        v = (intersect)?out.z:-1;

        c1 = (intersect && t<*closest_t && t > ClipVal+0.00001f)?true:false;
        c1 = shadow_calculation?c1&&(t<1.0f && t>0.00001f):c1;
        
        n0 = (c1&&!shadow_calculation)?read_imagef(norms,sampler,(int2)(x0,y0)).xyz:(float3)(0,0,0);
        n1 = (c1&&!shadow_calculation)?read_imagef(norms,sampler,(int2)(x1,y1)).xyz:(float3)(0,0,0);
        n2 = (c1&&!shadow_calculation)?read_imagef(norms,sampler,(int2)(x2,y2)).xyz:(float3)(0,0,0);
        
        col0 = (c1&&!shadow_calculation)?read_imagef(colors,sampler,(int2)(x0,y0)).xyz:(float3)(0,0,0);
        col1 = (c1&&!shadow_calculation)?read_imagef(colors,sampler,(int2)(x1,y1)).xyz:(float3)(0,0,0);
        col2 = (c1&&!shadow_calculation)?read_imagef(colors,sampler,(int2)(x2,y2)).xyz:(float3)(0,0,0);
        
        *closest_t  = (c1)?t:*closest_t;
        *normal     = (c1&&!shadow_calculation)?(u*n0 + v*n1 + (1-u-v)*n2):(*normal);
        *color      = (c1&&!shadow_calculation)?(u*col0 + v*col1 + (1-u-v)*col2):(*color);
        *closest_v  = (c1&&!shadow_calculation)?(Point + t*Dir):(*closest_v);
        *index      = (c1&&!shadow_calculation)?(int)material_type:*index;
        ret         = (c1)?true:ret;
        i           = (c1&&shadow_calculation)?count:i;
    }
    
    return ret;
}

bool intersectInnerNode(float3 Point, float3 Dir, float ClipVal,
                        float3 low, float3 high){
    bool ret = true;
    float3 invDir = 1/Dir;
    float txmin, txmax, tymin, tymax, tzmin, tzmax;
    
    float3 bounds[2];
    bounds[0] = low; bounds[1] = high;
    uint signx, signy, signz;
    signx = (invDir.x < 0)?1:0;
    signy = (invDir.y < 0)?1:0;
    signz = (invDir.z < 0)?1:0;
    
    txmin = (bounds[signx].x - Point.x) * invDir.x;
    txmax = (bounds[1-signx].x - Point.x) * invDir.x;
    tymin = (bounds[signy].y - Point.y) * invDir.y;
    tymax = (bounds[1-signy].y - Point.y) * invDir.y;
    
    ret = ((txmin > tymax) || (tymin > txmax))?false:ret;
    txmin = (tymin > txmin)?tymin:txmin;
    txmax = (tymax < txmax)?tymax:txmax;
    
    tzmin = (bounds[signz].z - Point.z) * invDir.z;
    tzmax = (bounds[1-signz].z - Point.z) * invDir.z;
    
    ret = ((txmin > tzmax) || (tzmin > txmax))?false:ret;
    txmin = (txmin > txmin)?tzmin:txmin;
    txmax = (tzmax < txmax)?tzmax:txmax;
    
    ret = (txmax < ClipVal)?false:ret;
    
    return ret;
}

bool intersectScene_bvh(read_only image2d_t lower_bounds, read_only image2d_t higher_bounds,
                        read_only image2d_t nodes,
                        constant uint *img_size,
                        read_only image2d_t verts, read_only image2d_t norms, read_only image2d_t colors,
                        float3 Point, float3 Dir, float ClipVal,
                        float3 *Isect, float3 *Normal, float3 *Color,uint *index,
                        bool shadow_calculation){
    bool c1,c2,c3;
    struct stack BVH_stack;
    BVH_stack.head = 0;
    BVH_stack.entry[BVH_stack.head++] = 0;
    
    uint bvh_size = img_size[0];
    float param_t = FLT_MAX;
    while(BVH_stack.head!=0){
        uint isLeafNode, leftNode, rightNode;
        float3 low, high;
        uint node_idx = BVH_stack.entry[--BVH_stack.head];
        uint x = node_idx%bvh_size;
        uint y = node_idx/bvh_size;
        uint4 node = read_imageui(nodes,sampler,(int2)(x,y));
        isLeafNode = node.z>>31;
        
        c1 = (isLeafNode==0)?true:false;
        low = read_imagef(lower_bounds,sampler,(int2)(x,y)).xyz;
        high = read_imagef(higher_bounds,sampler,(int2)(x,y)).xyz;
        
        c2 = intersectInnerNode(Point, Dir, ClipVal, low, high);
        leftNode = (c1&&c2)?node.x:-1;
        rightNode = (c1&&c2)?node.y:-1;

        if(c1&&c2){
            BVH_stack.entry[BVH_stack.head++]=leftNode;
            BVH_stack.entry[BVH_stack.head++]=rightNode;
        }else{
            
        }
        
        int count = (!c1)?(node.z&0x7fffffff):0;
        int triStartIdx = (!c1)?node.w:-1;
        
        if(!c1){
            c3 = intersectLeafNode(triStartIdx,count,
                                         verts,norms,colors,
                                         img_size,
                                         Point,Dir,ClipVal,
                                         Isect,Normal,Color,index,
                                         &param_t,shadow_calculation);
        }else{
            c3 = false;
        }
        
        
        BVH_stack.head = (c3&&shadow_calculation)?0:BVH_stack.head;
    }
    
    bool ret = (param_t < FLT_MAX)?true:false;
    ret = shadow_calculation?c3:ret;
    return ret;
    
}

static float get_random_bvh(unsigned int *seed0, unsigned int *seed1) {
    
    *seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);
    *seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);
    
    unsigned int ires = ((*seed0) << 16) + (*seed1);
    
    union {
        float f;
        unsigned int ui;
    } res;
    
    res.ui = (ires & 0x007fffff) | 0x40000000;
    return (res.f - 2.0f) / 2.0f;
}

float3 getRandDir_bvh(float3 Normal,unsigned int* seed0, unsigned int* seed1){
    float3 Dir, w, u, v;
    float r1,r2,r2s;
    
    w = Normal;
    u = normalize(cross((fabs(w.x) > 0.1f ? (float3)(0, 1, 0) : (float3)(1, 0, 0)), w));
    v = cross(w,u);
    
    r1 = 2.0f * PI * get_random_bvh(seed0, seed1);
    r2 = get_random_bvh(seed0, seed1);
    r2s = sqrt(r2);
    
    Dir = normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2));
    
    return Dir;
}

float3 getReflectedDir_bvh(float3 Normal, float3 Dir){
    return normalize(Dir - 2*Normal*dot(Normal,Dir));
}

float3 getRefractedDir_bvh(float3 Normal, float3 Dir){
    bool into = (dot(Normal,Dir)>0)?true:false;
    float n = into?0.66f:1.5f;
    float d = dot(Dir,Dir);
    float cos2t = 1.0f - n*n*(1.0f-d*d);
    bool tir = (cos2t<0.0f)?true:false;
    
    float3 out_dir = Dir*n - n*(into?1:-1)*(d*n + sqrt(cos2t));
    out_dir = tir?normalize(Dir - 2*Normal*dot(Normal,Dir)):normalize(out_dir);
    
    return out_dir;
    
}

uint3 radiance_bvh(read_only image2d_t lower_bounds, read_only image2d_t higher_bounds,
                   read_only image2d_t nodes,
                   constant uint *img_size,
                   read_only image2d_t verts, read_only image2d_t norms, read_only image2d_t colors,
                   float3 Eye, float3 Dir, float3 Light,
                   unsigned int *seed0, unsigned int *seed1){
    float3 Point = Eye;
    float3 RandDir = Dir;
    uint3 color = (uint3)(0,0,0);
    float diff_factor;
    float3 Isect, Normal, Color, Source;
    bool intersect=true;
    bool shadow;
    float ClipVal = 1.0f;
    float3 temp0,temp1,temp2;
    int temp3 = -1;
    int index = -1;
    bool diffuse, specular, refract;
    
    int i;
    Source = Light;
    for(i=0;(i<NUM_BOUNCES)&&(intersect);i++){
        Normal = (float3)(0,0,0);
        Isect = (float3)(0,0,0);
        Color = (float3)(0,0,0);
        
        intersect = intersectScene_bvh(lower_bounds,higher_bounds,
                                       nodes,img_size,
                                       verts, norms, colors,
                                       Point,RandDir,ClipVal,
                                       &Isect,&Normal,&Color,&index,false);
        
        Normal = normalize(Normal);
        float3 light_dir = normalize(Source-Isect);
        
        ClipVal = 0.0f;
        shadow = (intersect)?intersectScene_bvh(lower_bounds,higher_bounds,
                                                nodes,img_size,
                                                verts, norms, colors,
                                                Isect,Light-Isect,ClipVal,
                                                &temp0,&temp1,&temp2,&temp3,true):false;
        
        diffuse = (index == 1)?true:false;
        specular = (index == 2)?true:false;
        refract = (index == 3)?true:false;
        
        diff_factor = dot(Normal,light_dir);
        diff_factor = (diff_factor<0.0f)?0.0f:diff_factor;
        diff_factor = shadow?0:diff_factor;
        
        color.x += (intersect)?(uint)(floor(diff_factor * (float)255 * Color.x)):0;
        color.y += (intersect)?(uint)(floor(diff_factor * (float)255 * Color.y)):0;
        color.z += (intersect)?(uint)(floor(diff_factor * (float)255 * Color.z)):0;
        
        RandDir = (intersect&&diffuse)?getRandDir_bvh(normalize(Normal),seed0,seed1):
                (intersect&&specular)?getReflectedDir_bvh(normalize(Normal),normalize(Isect-Point)):
                (intersect&&refract)?getRefractedDir_bvh(normalize(Normal),normalize(Isect-Point)):
                (float3)(0,0,0);
        Point   = (intersect)?Isect:(float3)(0,0,0);
        Source = Point;
    }
    color.x = (uint)((float)color.x/i);
    color.y = (uint)((float)color.y/i);
    color.z = (uint)((float)color.z/i);
    return color;
    
}

float3 createCamRay(const int x_coord, const int y_coord, float3 look_up_dir, float3 look_at_dir, float3 Eye){
    
    int width = 512; int height = 512;
    /* create a local coordinate frame for the camera */
    float3 rendercamview = normalize(look_at_dir);
    float3 rendercamup = normalize(look_up_dir);
    float3 horizontalAxis = cross(rendercamview, rendercamup); horizontalAxis = normalize(horizontalAxis);
    float3 verticalAxis = cross(horizontalAxis, rendercamview); verticalAxis = normalize(verticalAxis);
    
    float3 middle = Eye + rendercamview;
    float3 horizontal = horizontalAxis * tan(45 * 0.5f * (PI / 180));
    float3 vertical   =  verticalAxis * tan(45 * -0.5f * (PI / 180));
    
    unsigned int x = x_coord;
    unsigned int y = y_coord;
    
    int pixelx = x_coord;
    int pixely = height - y_coord - 1;
    
    float sx = (float)pixelx / (width - 1.0f);
    float sy = (float)pixely / (height - 1.0f);
    
    float3 pointOnPlaneOneUnitAwayFromEye = middle + (horizontal * ((2 * sx) - 1)) + (vertical * ((2 * sy) - 1));
    
    float3 pointOnImagePlane = pointOnPlaneOneUnitAwayFromEye;
    
    float3 aperturePoint;
    
    aperturePoint = Eye;
    
    float3 apertureToImagePlane = pointOnImagePlane - aperturePoint;
    
    return apertureToImagePlane;
}

kernel void render_bvh(read_only image2d_t lower_bounds, read_only image2d_t higher_bounds,
                       read_only image2d_t nodes,
                       constant uint* img_size,
                       read_only image2d_t verts, read_only image2d_t norms, read_only image2d_t colors,
                       global uint4 *imagedata, global uint3 *accum_buffer, global float3 *cam_data){
    size_t i = get_global_id(0);
    size_t j = get_global_id(1);
    
    unsigned int seed0=i*img_size[2]+img_size[2]*img_size[2]+i*i, seed1=j*img_size[2]+img_size[2]*img_size[2]+j*j;
    float3 Eye = cam_data[0];
    float3 look_at_dir = cam_data[1];
    float3 look_up_dir = cam_data[2];
    
    uint3 color = (uint3)(0,0,0);
    float3 Light = (float3)(0.2f,0.2f,-1.0f);
    for(int k=0;k<NUM_SAMPLES;k++){
        for(int l=0;l<NUM_SAMPLES;l++){
            float3 Dir = createCamRay((const int)i,(const int)j,look_up_dir,look_at_dir,Eye);
            
            color += radiance_bvh(lower_bounds,higher_bounds,
                                  nodes,
                                  img_size,
                                  verts,norms,colors,
                                  Eye,Dir,Light,
                                  &seed0,&seed1);
            
        }
    }
    color = color/(NUM_SAMPLES*NUM_SAMPLES);
    accum_buffer[512*i+j] += color;
    
    uint3 temp = accum_buffer[512*i+j]/(img_size[2]+1);

    imagedata[i*512+j] = (uint4)(temp.x,temp.y,temp.z,255);
    
}

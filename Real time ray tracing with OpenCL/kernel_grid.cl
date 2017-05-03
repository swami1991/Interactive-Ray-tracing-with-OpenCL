__constant int   NUM_BOUNCES = 2;
__constant int   NUM_SAMPLES = 1;
__constant float PI          = 3.14159265359f;

int RayTriangleIntersection(float3 Point, float3 Dir, float3 v0, float3 v1, float3 v2, float3* out){
    int ret = 0;
    float t,u,v;
    float3 v0v1, v0v2;
    v0v1 = v1 - v0;
    v0v2 = v2 - v0;
    
    float3 pvec = cross(Dir,v0v2);
    float det = dot(v0v1,pvec);
    float invdet;
    float3 tvec,qvec;
    
    if(fabs(det) > 0.00000001f){
        invdet = 1/det;
        
        tvec = Point - v0;
        u = dot(tvec,pvec)*invdet;
        
        qvec = cross(tvec,v0v1);
        v = dot(Dir,qvec)*invdet;
        
        if(u>=0.0f && u<=1.0f && v>=0.0f && (u+v)<=1.0f){
            out->x = dot(v0v2,qvec)*invdet;
            out->y = u;
            out->z = v;
            ret = 1;
        }
        else{
            
        }
    }
    else{
        
    }
    return ret;
}

float3 world2grid(float3 vec, float3 grid_origin, float3 voxel_w, uint3 grid_dim){
    float3 out;
    out.x = floor((vec.x - grid_origin.x)/voxel_w.x);
    out.y = floor((vec.y - grid_origin.y)/voxel_w.y);
    out.z = floor((vec.z - grid_origin.z)/voxel_w.z);
    
    out.x = (out.x >= (float)grid_dim.x)?(float)grid_dim.x-1:out.x;
    out.y = (out.y >= (float)grid_dim.y)?(float)grid_dim.y-1:out.y;
    out.z = (out.z >= (float)grid_dim.z)?(float)grid_dim.z-1:out.z;
    
    return out;
}

float3 grid2world(float3 grid_id, float3 grid_origin, float3 voxel_w){
    float3 out;
    out.x = grid_id.x*voxel_w.x + grid_origin.x;
    out.y = grid_id.y*voxel_w.y + grid_origin.y;
    out.z = grid_id.z*voxel_w.z + grid_origin.z;
    
    return out;
}

int intersectGrid(global uint3 *grid_idx,
                  uint size,
                  global float3 *verts,
                  global float3 *norms,
                  global float3 *colors,
                  float3 Point, float3 Dir,
                  float3 *closest_v, float3 *normal, float3 *color,
                  float3 t_isect, float3 t_delta){
    float closest_t = 10000;
    float t,u,v;
    int c1, c2, intersect, ret;
    ret = 0;
    uint3 idx = uint3(0,0,0);
    float3 v0,v1,v2,out;
    for(uint i=0;i<size;i++){
        idx = grid_idx[i];
        
        v0 = verts[idx.x];
        v1 = verts[idx.y];
        v2 = verts[idx.z];
        
        intersect = RayTriangleIntersection(Point,Dir,v0,v1,v2,&out);
        t = (intersect==1)?out.x:10000;
        u = (intersect==1)?out.y:-1;
        v = (intersect==1)?out.z:-1;
        
        c1 = (t<closest_t)?1:0;
        c2 = ((t>=t_isect.x-t_delta.x&&t<=t_isect.x) ||
              (t>=t_isect.y-t_delta.y&&t<=t_isect.y) ||
              (t>=t_isect.z-t_delta.z&&t<=t_isect.z))?1:0;
        
        closest_t  = (c1&&c2)?t:closest_t;
        *normal    = (c1&&c2)?(u*norms[idx.x] + v*norms[idx.y] + (1-u-v)*norms[idx.z]):(*normal);
        *color     = (c1&&c2)?(u*colors[idx.x] + v*colors[idx.y] + (1-u-v)*colors[idx.z]):(*color);
        *closest_v = (c1&&c2)?(Point + t*Dir):(*closest_v);
        ret        = (c1&&c2)?1:ret;
    }
    
    return ret;
}

void gridTraversalInitialization(float3 voxel_w,
                                 float3 grid_origin,
                                 uint3 grid_dim,
                                 float3 Point, float3 Dir,
                                 float3 *voxel_id, float3 *t_isect, float3 *t_delta,
                                 float3 *step, float3 *out){
    float3 grid_id_temp, grid_id_temp1, t_isect1;
    
    *voxel_id = world2grid(Point,grid_origin,voxel_w,grid_dim);
    
    grid_id_temp.x = (Dir.x>0.0f)?(voxel_id->x+1.0f):voxel_id->x;
    grid_id_temp.y = (Dir.y>0.0f)?(voxel_id->y+1.0f):voxel_id->y;
    grid_id_temp.z = (Dir.z>0.0f)?(voxel_id->z+1.0f):voxel_id->z;
    
    grid_id_temp1 = grid_id_temp;
    
    grid_id_temp = grid2world(grid_id_temp,grid_origin,voxel_w);
    t_isect->x = (Dir.x!=0.0f)?(grid_id_temp.x-Point.x)/Dir.x:10000;
    t_isect->y = (Dir.y!=0.0f)?(grid_id_temp.y-Point.y)/Dir.y:10000;
    t_isect->z = (Dir.z!=0.0f)?(grid_id_temp.z-Point.z)/Dir.z:10000;
    
    grid_id_temp1.x = (Dir.x>0.0f)?(grid_id_temp1.x+1.0f):(grid_id_temp1.x-1);
    grid_id_temp1.y = (Dir.y>0.0f)?(grid_id_temp1.y+1.0f):(grid_id_temp1.y-1);
    grid_id_temp1.z = (Dir.z>0.0f)?(grid_id_temp1.z+1.0f):(grid_id_temp1.z-1);
    
    grid_id_temp1 = grid2world(grid_id_temp1,grid_origin,voxel_w);
    t_isect1.x = (Dir.x!=0.0f)?(grid_id_temp1.x-Point.x)/Dir.x:10000;
    t_isect1.y = (Dir.y!=0.0f)?(grid_id_temp1.y-Point.y)/Dir.y:10000;
    t_isect1.z = (Dir.z!=0.0f)?(grid_id_temp1.z-Point.z)/Dir.z:10000;
    
    t_delta->x = (Dir.x!=0.0f)?(t_isect1.x- t_isect->x):10000;
    t_delta->y = (Dir.y!=0.0f)?(t_isect1.y- t_isect->y):10000;
    t_delta->z = (Dir.z!=0.0f)?(t_isect1.z- t_isect->z):10000;
    
    step->x = (Dir.x<0.0f)?-1:1;
    step->y = (Dir.y<0.0f)?-1:1;
    step->z = (Dir.z<0.0f)?-1:1;
    
    out->x = (Dir.x<0.0f)?-1:(float)grid_dim.x;
    out->y = (Dir.y<0.0f)?-1:(float)grid_dim.y;
    out->z = (Dir.z<0.0f)?-1:(float)grid_dim.z;
}

int isOccluded(global uint3 *grid_idx,
               uint size,
               global float3 *verts,
               float3 Point, float3 Dir,
               float3 t_isect, float3 t_delta){
    float t;
    int c1, c2, intersect, ret;
    ret = 0;
    uint3 idx = (uint3)(0,0,0);
    float3 v0,v1,v2,out;
    out = (float3)(0,0,0);
    for(uint i=0;((i<size)&&(ret==0));i++){
        idx = grid_idx[i];
        
        v0 = verts[idx.x];
        v1 = verts[idx.y];
        v2 = verts[idx.z];
        
        intersect = RayTriangleIntersection(Point,Dir,v0,v1,v2,&out);
        
        c1 = ((intersect==1)&&(out.x>0.00001)&&(out.x<1.0f))?1:0;
        c2 = ((out.x>=t_isect.x-t_delta.x&&out.x<=t_isect.x) ||
              (out.x>=t_isect.y-t_delta.y&&out.x<=t_isect.y) ||
              (out.x>=t_isect.z-t_delta.z&&out.x<=t_isect.z))?1:0;
        
        ret = (c1&&c2)?1:0;
    }
    
    return ret;
}

int intersectScene(global uint3 *grid_idx,
                   global uint *arr_size,
                   global float3 *verts,
                   global float3 *norms,
                   global float3 *colors,
                   global uint *idx,
                   global uint3 *grid_dim,
                   global float3 *voxel_w,
                   global float3 *grid_origin,
                   float3 Point, float3 Dir, float ClipVal,
                   float3 *closest_v, float3 *normal, float3 *color, int shadow){
    float3 voxel_id, t_isect, t_delta, step, out;
    int c1,c2,c3,c4,c5,c6,c7,c8;
    int flag = 0;
    int intersect = 0;
    int index = 0;
    
    float3 voxel_width = voxel_w[0];
    float3 grid_org = grid_origin[0];
    uint3 grid_dimensions = grid_dim[0];
    
    int x,y,z;
    uint nx,ny,nz;
    uint begin, end;
    int size;
    
    gridTraversalInitialization(voxel_width,grid_org,grid_dimensions,
                                Point,Dir,
                                &voxel_id,&t_isect,&t_delta,
                                &step,&out);
    
    do{
        c1 = ((t_isect.x < t_isect.y)&&(t_isect.x < t_isect.z))?1:0;
        c2 = ((t_isect.z < t_isect.y)&&(t_isect.z < t_isect.x))?1:0;
        c3 = ((t_isect.y < t_isect.x)&&(t_isect.y < t_isect.z))?1:0;
        c5 = ((t_isect.x == t_isect.y)&&(t_isect.x < t_isect.z))?1:0;
        c6 = ((t_isect.x == t_isect.z)&&(t_isect.x < t_isect.y))?1:0;
        c7 = ((t_isect.y == t_isect.z)&&(t_isect.y < t_isect.x))?1:0;
        c8 = ((t_isect.x == t_isect.y)&&(t_isect.x == t_isect.z))?1:0;
        
        voxel_id.x = (c1||c5||c6||c8)?(voxel_id.x + step.x):voxel_id.x;
        t_isect.x  = (c1||c5||c6||c8)?(t_isect.x + t_delta.x):t_isect.x;
        
        voxel_id.z = (c2||c6||c7||c8)?(voxel_id.z + step.z):voxel_id.z;
        t_isect.z  = (c2||c6||c7||c8)?(t_isect.z + t_delta.z):t_isect.z;
        
        voxel_id.y = (c3||c5||c7||c8)?(voxel_id.y + step.y):voxel_id.y;
        t_isect.y  = (c3||c5||c7||c8)?(t_isect.y + t_delta.y):t_isect.y;
        
        c4 = ((voxel_id.x == out.x) || (voxel_id.z == out.z) || (voxel_id.y == out.y))?1:0;
        
        flag = (c4)?1:flag;
        
        
        x = (int)voxel_id.x; y = (int)voxel_id.y; z = (int)voxel_id.z;
        nx = grid_dimensions.x; ny = grid_dimensions.y; nz = grid_dimensions.z;
        
        index = x*ny*nz+y*nz+z;
        c1 = (flag==0 && x >=0 && y >=0 && z >=0)?1:0;
        c2 = (index+1 < nx*ny*nz)?1:0;
        c3 = (index < nx*ny*nz)?1:0;
        c4 = ((t_isect.x-t_delta.x>ClipVal) || (t_isect.y-t_delta.y>ClipVal) || (t_isect.z-t_delta.z>ClipVal))?1:0;
        
        begin = (c1&&c3)?idx[index]:0;
        end = (c1&&c2)?(idx[index+1]):((c1&&!c2)?arr_size[0]:0);
        size = end-begin;
        
        intersect = (c4&&size>0)?(shadow?
                                  (isOccluded(&grid_idx[begin],size,
                                              verts,
                                              Point,Dir,
                                              t_isect,t_delta))
                                  :(intersectGrid(&grid_idx[begin],size,
                                                  verts,norms,colors,
                                                  Point,Dir,
                                                  closest_v,normal,color,
                                                  t_isect,t_delta))):0;
        
        flag = (size>0)?intersect:flag;
        
    }while(flag == 0);
    
    return intersect;
}

static float get_random(unsigned int *seed0, unsigned int *seed1) {
    
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

float3 getRandDir(float3 Normal,unsigned int* seed0, unsigned int* seed1){
    float3 Dir, w, u, v;
    float r1,r2,r2s;
    
    w = Normal;
    u = normalize(cross((fabs(w.x) > 0.1f ? (float3)(0, 1, 0) : (float3)(1, 0, 0)), w));
    v = cross(w,u);
    
    r1 = 2.0f * PI * get_random(seed0, seed1);
    r2 = get_random(seed0, seed1);
    r2s = sqrt(r2);
    
    Dir = normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2));
    
    return Dir;
}

uint3 radiance(global uint3 *grid_idx,
               global uint *arr_size,
               global float3 *verts,
               global float3 *norms,
               global float3 *colors,
               global uint *idx,
               global uint3 *grid_dim,
               global float3 *voxel_w,
               global float3 *grid_origin,
               float3 Eye, float3 Dir, float3 Light,
               unsigned int *seed0, unsigned int *seed1) {
    
    float3 Point = Eye;
    float3 RandDir = Dir;
    uint3 color = (uint3)(0,0,0);
    float clipVal = 1, diff_factor;
    float3 Isect, Normal, Color;
    int intersect=1,shadow;
    float3 temp1,temp2,temp3;
    
    
    int i;
    for(i=0;(i<NUM_BOUNCES)&&(intersect==1);i++){
        Normal = (float3)(0,0,0);
        Isect = (float3)(0,0,0);
        Color = (float3)(0,0,0);
        
        intersect = intersectScene(grid_idx, arr_size,
                                   verts,norms,colors,
                                   idx,
                                   grid_dim,voxel_w,grid_origin,
                                   Point,RandDir,clipVal,
                                   &Isect,&Normal,&Color,0);
        
        
        clipVal = 0;
        RandDir = Light-Isect;
        shadow = (intersect==1)?(intersectScene(grid_idx, arr_size,
                                                verts,norms,colors,
                                                idx,
                                                grid_dim,voxel_w,grid_origin,
                                                Isect,RandDir,clipVal,
                                                &temp1,&temp2,&temp3,1)):0;
        
        
        Normal = normalize(Normal);
        float3 light_dir = normalize(Light-Isect);
        diff_factor = dot(Normal,light_dir);
        diff_factor = (diff_factor<0.0f)?0:diff_factor*(1-shadow);
        
        color.x += (intersect==1)?(uint)(floor(diff_factor * (float)255 * Color.x)):0;
        color.y += (intersect==1)?(uint)(floor(diff_factor * (float)255 * Color.y)):0;
        color.z += (intersect==1)?(uint)(floor(diff_factor * (float)255 * Color.z)):0;
        
        Point   = (intersect==1)?Isect:(float3)(0,0,0);
        RandDir = (intersect==1)?getRandDir(normalize(Normal),seed0,seed1):(float3)(0,0,0);
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

kernel void render_grid(global uint3 *grid_idx,
                   global uint *arr_size,
                   global float3 *verts,
                   global float3 *norms,
                   global float3 *colors,
                   global uint *idx,
                   global uint3 *grid_dim,
                   global float3 *voxel_w,
                   global float3 *grid_origin,
                   constant uint *f_num,
                   global uint4 *imagedata, global uint3 *accum_buffer, global float3 *cam_data){
    size_t i = get_global_id(0);
    size_t j = get_global_id(1);
    
    unsigned int seed0=i*f_num[0]+f_num[0]*f_num[0]+i*i, seed1=j*f_num[0]+f_num[0]*f_num[0]+j*j;
    float3 Eye = cam_data[0];
    float3 look_at_dir = cam_data[1];
    float3 look_up_dir = cam_data[2];
    uint3 color = (uint3)(0,0,0);
    float3 Light = (float3)(0,0.4,0.5);
    for(int k=0;k<NUM_SAMPLES;k++){
        for(int l=0;l<NUM_SAMPLES;l++){
            float x = (float)get_random(&seed0,&seed1)/(512)+(float)i/512-0.5;
            float y = (float)get_random(&seed0,&seed1)/(512)+(float)j/512-0.5;
            float3 Dir = createCamRay((const int)i,(const int)j,look_up_dir,look_at_dir,Eye);
            
            color += radiance(grid_idx, arr_size,
                              verts,norms,colors,idx,
                              grid_dim,voxel_w,grid_origin,
                              Eye,Dir,Light,&seed0,&seed1);
            
        }
    }
    color = color/(NUM_SAMPLES*NUM_SAMPLES);
    accum_buffer[512*i+j] += color;
    
    uint3 temp = accum_buffer[512*i+j]/(f_num[0]+1);
    
    imagedata[i*512+j] = (uint4)(temp.x,temp.y,temp.z,255);
}

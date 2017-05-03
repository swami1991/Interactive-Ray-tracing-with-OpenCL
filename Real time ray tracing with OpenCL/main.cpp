//
//  main.cpp
//  Real time path tracer with OpenCL
//
//  Created by Swaminathan Ramachandran on 3/23/17.
//
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>

#include "struct.hpp"

#include <sys/time.h>

using namespace std;

cl_context context;
cl_context_properties properties[5];
cl_kernel kernel;
cl_command_queue command_queue;
cl_program program;
cl_uint num_of_platforms=0;
cl_platform_id platform_id;
cl_device_id device_id;
cl_uint num_of_devices=0;
cl_int err;

uint img_size[3];
uint framenumber;
cl_mem d_size, d_verts, d_norms, d_colors;
cl_mem output_image, d_accum_buffer, d_bvh_lower, d_bvh_higher, d_bvh_nodes, d_camera;

cl_float4 *verts, *norms, *colors;
cl_float4 *bvh_lower, *bvh_higher;
cl_uint4 *bvh_nodes;
cl_float3 camera[3];

uint* pixels;
cl_uint3 *accum_buffer;

cl_mem d_grid_idx, d_arr_size, d_idx, d_grid_dim, d_voxel_w, d_grid_origin, d_f_num;

GLFWwindow* window;

CGLContextObj gl_ctx;
CGLShareGroupObj gl_sharegroup;

GLuint programID;

GLuint output_color, output_verts, VertexArrayID;

#if USE_GRID
const char *KERNEL_FILE_PATH="/Users/sram/Spring2017/CSC562/proj3/Real time ray tracing with OpenCL/Real time ray tracing with OpenCL/kernel_grid.cl";
#endif
#if USE_BVH
const char *KERNEL_FILE_PATH="/Users/sram/Spring2017/CSC562/proj3/Real time ray tracing with OpenCL/Real time ray tracing with OpenCL/kernel_bvh.cl";
#endif

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
        VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
        FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
    
    
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    
    
    
    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    
    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

int initGL(){
    if(!glfwInit()){
        cerr<<"Failed to initialize GLFW"<<endl;
        return 1;
    }
    
    glfwWindowHint(GLFW_SAMPLES,1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(PIXELS_X, PIXELS_Y, "Real time ray tracing", NULL, NULL);
    if(window == NULL){
        cerr<<"Failed to open GLFW Window"<<endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    
    if(glewInit() != GLEW_OK){
        cerr<<"Failed to initialize GLEW"<<endl;
        glfwTerminate();
        return 1;
    }
    
    programID = LoadShaders("/Users/sram/Spring2017/CSC562/proj3/Real time ray tracing with OpenCL/Real time ray tracing with OpenCL/vertexshader","/Users/sram/Spring2017/CSC562/proj3/Real time ray tracing with OpenCL/Real time ray tracing with OpenCL/fragmentshader");
    
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    glGenBuffers(1, &output_color);
    glBindBuffer(GL_ARRAY_BUFFER, output_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cl_uint4)*PIXELS_X*PIXELS_Y, 0, GL_DYNAMIC_DRAW);
    
    float *screen_verts = new float[3*PIXELS_Y*PIXELS_X];
    for(int i=0;i<3*PIXELS_X*PIXELS_Y;i=i+3){
        float x = (float)(i/3)/PIXELS_X - PIXELS_X/2;
        float y = (float)((i/3)%PIXELS_X) - PIXELS_X/2;
        screen_verts[i] = x/PIXELS_X;
        screen_verts[i+1] = y/PIXELS_Y;
        screen_verts[i+2] = 0.0f;
    }
    
    glGenBuffers(1, &output_verts);
    glBindBuffer(GL_ARRAY_BUFFER, output_verts);
    glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*PIXELS_X*PIXELS_Y, screen_verts, GL_STATIC_DRAW);
    
    return 0;
}

int initCL(){
    
    if(clGetPlatformIDs(1, &platform_id, &num_of_platforms)!= CL_SUCCESS){
        printf("Unable to get platform_id\n");
        return 1;
    }
    
    if(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices) != CL_SUCCESS){
        printf("Unable to get device_id\n");
        return 1;
    }
    
    gl_ctx        = CGLGetCurrentContext();
    gl_sharegroup = CGLGetShareGroup(gl_ctx);
    

    properties[0]= CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
    properties[1]= (cl_context_properties) gl_sharegroup;
    properties[2]= CL_CONTEXT_PLATFORM;
    properties[3]= (cl_context_properties) platform_id;
    properties[4]= 0;
    
    context = clCreateContext(properties,1,&device_id,NULL,NULL,&err);
    
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    
    string source;
    ifstream file(KERNEL_FILE_PATH);
    if (!file){
        cout << "\nNo OpenCL file found!" << endl << "Exiting..." << endl;
        system("PAUSE");
        exit(1);
    }
    while (!file.eof()){
        char line[2048];
        file.getline(line, 2047);
        source += line;
    }
    
    const char* kernel_source = source.c_str();
    
    program = clCreateProgramWithSource(context,1,(const char **)&kernel_source, NULL, &err);
    if(err != CL_SUCCESS){
        printf("Error creating program\n");
        return 1;
    }
    
    // compile the program
    err = clBuildProgram(program,0,0,"",0,0);
    if (err != CL_SUCCESS) {
        char *buff_erro;
        cl_int errcode;
        size_t build_log_len;
        errcode = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_len);
        if (errcode) {
            printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
            exit(-1);
        }
        
        buff_erro = (char*)malloc(build_log_len);
        if (!buff_erro) {
            printf("malloc failed at line %d\n", __LINE__);
            exit(-2);
        }
        
        errcode = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, build_log_len, buff_erro, NULL);
        if (errcode) {
            printf("clGetProgramBuildInfo failed at line %d\n", __LINE__);
            exit(-3);
        }
        
        fprintf(stderr,"Build log: \n%s\n", buff_erro); //Be careful with  the fprint
        free(buff_erro);
        fprintf(stderr,"clBuildProgram failed\n");
        exit(EXIT_FAILURE);
    }
    
#if USE_GRID
    kernel = clCreateKernel(program, "render_grid", &err);
#endif
#if USE_BVH
    kernel = clCreateKernel(program, "render_bvh", &err);
#endif
    
    return 0;
}

#if USE_GRID
void setupCL(){
    vector<glm::vec3> temp_verts, temp_norms, temp_colors;
    loadScene(temp_verts,temp_norms,temp_colors,&verts,&norms,&colors);
    
    camera[0].x = 0; camera[0].y = 0; camera[0].z = -1;
    camera[1].x = 0; camera[1].y = 0; camera[1].z = 1;
    camera[2].x = 0; camera[2].y = 1; camera[2].z = 0;
    
    accum_buffer = new cl_uint3[PIXELS_Y*PIXELS_X];
    
    for(int i=0;i<temp_verts.size();i++){
        verts[i].x = temp_verts[i].x; verts[i].y = temp_verts[i].y; verts[i].z = temp_verts[i].z;
        norms[i].x = temp_norms[i].x; norms[i].y = temp_norms[i].y; norms[i].z = temp_norms[i].z;
        colors[i].x = temp_colors[i].x; colors[i].y = temp_colors[i].y; colors[i].z = temp_colors[i].z;
    }
    
    Grid grid_model;
    grid_model.createGrid(temp_verts);
    
    output_image = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, output_color, &err);
    d_accum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_uint3)*PIXELS_X*PIXELS_Y, nullptr, &err);
    
    d_verts = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(verts[0])*temp_verts.size(), nullptr, nullptr);
    d_norms = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(norms[0])*temp_verts.size(), nullptr, nullptr);
    d_colors = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(colors[0])*temp_verts.size(), nullptr, nullptr);
    d_grid_idx = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(grid_model.ids[0])*grid_model.arr_size, nullptr, nullptr);
    d_arr_size = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint), nullptr, nullptr);
    d_idx = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(grid_model.size[0])*grid_model.arr_size, nullptr, nullptr);
    d_grid_dim = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint3), nullptr, nullptr);
    d_voxel_w = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3), nullptr, nullptr);
    d_grid_origin = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3), nullptr, nullptr);
    d_f_num = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint), nullptr, nullptr);
    d_camera = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3)*3, nullptr, nullptr);
    
    clEnqueueWriteBuffer(command_queue, d_verts, CL_TRUE, 0, sizeof(verts[0])*temp_verts.size(), verts, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_norms, CL_TRUE, 0, sizeof(verts[0])*temp_verts.size(), norms, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_colors, CL_TRUE, 0, sizeof(verts[0])*temp_verts.size(), colors, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_grid_idx, CL_TRUE, 0, sizeof(grid_model.ids[0])*grid_model.arr_size, grid_model.ids, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_arr_size, CL_TRUE, 0, sizeof(cl_uint), &grid_model.arr_size, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_idx, CL_TRUE, 0, sizeof(grid_model.size[0])*grid_model.arr_size, grid_model.size, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_grid_dim, CL_TRUE, 0, sizeof(cl_uint3), &grid_model.grid_dim, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_voxel_w, CL_TRUE, 0, sizeof(cl_float3), &grid_model.voxel_w, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(command_queue, d_grid_origin, CL_TRUE, 0, sizeof(cl_float3), &grid_model.grid_origin, 0, nullptr, nullptr);
    
    framenumber = 0;
    
}
#endif
#if USE_BVH
void setupCL(){
    
    vector<glm::vec3> temp_verts, temp_norms, temp_colors;
    loadScene(temp_verts,temp_norms,temp_colors,&verts,&norms,&colors);
    
    camera[0].x = 0; camera[0].y = 0; camera[0].z = -1;
    camera[1].x = 0; camera[1].y = 0; camera[1].z = 1;
    camera[2].x = 0; camera[2].y = 1; camera[2].z = 0;
    
    accum_buffer = new cl_uint3[PIXELS_Y*PIXELS_X];
    
    uint size = 0;
    struct GPU_BVHNode *BVH = CreateBVH_GPU(CreateBVH(temp_verts),verts,norms,colors,temp_verts,temp_norms,temp_colors,size);
    
    img_size[0] = (floor(sqrt((float)size))==sqrt((float)size))?sqrt((float)size):sqrt((float)size)+1;
    img_size[1] = (floor(sqrt((float)temp_verts.size()))==sqrt((float)temp_verts.size()))?sqrt((float)temp_verts.size()):sqrt((float)temp_verts.size())+1;
    bvh_lower = new cl_float4[img_size[0]*img_size[0]];
    bvh_higher = new cl_float4[img_size[0]*img_size[0]];
    bvh_nodes = new cl_uint4[img_size[0]*img_size[0]];
    CreateBVH_Texture(BVH,bvh_lower,bvh_higher,bvh_nodes,size);;
    cout<<"Done building BVH"<<endl;
    
    cl_image_format format = {CL_RGBA,CL_UNSIGNED_INT8};
    cl_image_desc desc;
    desc.image_width = PIXELS_X; desc.image_height = PIXELS_Y; desc.image_depth = 1;
    desc.image_type = CL_MEM_OBJECT_IMAGE2D; desc.image_array_size = 1;
    
    output_image = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, output_color, &err);
    if(err != CL_SUCCESS)
        cout<<"Failed to create OpenCL buffer object from GL buffer object"<<endl;
    d_accum_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_uint3)*PIXELS_X*PIXELS_Y, nullptr, &err);
    
    format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_FLOAT;
    desc.image_width = img_size[0]; desc.image_height = img_size[0]; desc.image_depth = 1;
    desc.image_type = CL_MEM_OBJECT_IMAGE2D; desc.image_array_size = 1;
    d_bvh_lower = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, nullptr, &err);
    d_bvh_higher = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, nullptr, &err);
    
    format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_UNSIGNED_INT32;
    desc.image_width = img_size[0]; desc.image_height = img_size[0]; desc.image_depth = 1;
    desc.image_type = CL_MEM_OBJECT_IMAGE2D; desc.image_array_size = 1;
    d_bvh_nodes = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, nullptr, &err);
    
    format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_FLOAT;
    desc.image_width = img_size[1]; desc.image_height = img_size[1]; desc.image_depth = 1;
    desc.image_type = CL_MEM_OBJECT_IMAGE2D; desc.image_array_size = 1;
    d_verts = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, nullptr, &err);
    d_norms = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, nullptr, &err);
    d_colors = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc, nullptr, &err);
    
    d_size = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uint)*3, nullptr, nullptr);
    d_camera = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_float3)*3, nullptr, nullptr);
    
    size_t bvh_img_origin[3] = {0,0,0};
    size_t bvh_img_region[3] = {img_size[0],img_size[0],1};
    
    size_t verts_img_origin[3] = {0,0,0};
    size_t verts_img_region[3] = {img_size[1],img_size[1],1};
    
    clEnqueueWriteImage(command_queue, d_bvh_lower, CL_FALSE, bvh_img_origin, bvh_img_region, 0, 0, bvh_lower, 0, nullptr, nullptr);
    clEnqueueWriteImage(command_queue, d_bvh_higher, CL_FALSE, bvh_img_origin, bvh_img_region, 0, 0, bvh_higher, 0, nullptr, nullptr);
    clEnqueueWriteImage(command_queue, d_bvh_nodes, CL_FALSE, bvh_img_origin, bvh_img_region, 0, 0, bvh_nodes, 0, nullptr, nullptr);
    clEnqueueWriteImage(command_queue, d_verts, CL_FALSE, verts_img_origin, verts_img_region, 0, 0, verts, 0, nullptr, nullptr);
    clEnqueueWriteImage(command_queue, d_norms, CL_FALSE, verts_img_origin, verts_img_region, 0, 0, norms, 0, nullptr, nullptr);
    clEnqueueWriteImage(command_queue, d_colors, CL_FALSE, verts_img_origin, verts_img_region, 0, 0, colors, 0, nullptr, nullptr);
    
    framenumber = 0;
}
#endif

void run_kernel(){
    if(framenumber==0){
        for(int i=0;i<PIXELS_X*PIXELS_Y;i++){
            accum_buffer[i].x = 0;
            accum_buffer[i].y = 0;
            accum_buffer[i].z = 0;
        }
        clEnqueueWriteBuffer(command_queue, d_accum_buffer, CL_FALSE, 0, sizeof(cl_uint3)*PIXELS_X*PIXELS_Y, accum_buffer, 0, nullptr, nullptr);
    }
    img_size[2] = framenumber;

    clEnqueueWriteBuffer(command_queue, d_camera, CL_FALSE, 0, sizeof(cl_float3)*3, camera, 0, nullptr, nullptr);
#if USE_GRID
   clEnqueueWriteBuffer(command_queue, d_f_num, CL_FALSE, 0, sizeof(cl_uint), &framenumber, 0, nullptr, nullptr);
    
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_grid_idx);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_arr_size);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_verts);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_norms);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_colors);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), &d_idx);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), &d_grid_dim);
    clSetKernelArg(kernel, 7, sizeof(cl_mem), &d_voxel_w);
    clSetKernelArg(kernel, 8, sizeof(cl_mem), &d_grid_origin);
    clSetKernelArg(kernel, 9, sizeof(cl_mem), &d_f_num);
    clSetKernelArg(kernel, 10, sizeof(cl_mem), &output_image);
    clSetKernelArg(kernel, 11, sizeof(cl_mem), &d_accum_buffer);
    clSetKernelArg(kernel, 12, sizeof(cl_mem), &d_camera);
#endif
#if USE_BVH
    clEnqueueWriteBuffer(command_queue, d_size, CL_FALSE, 0, sizeof(cl_uint)*3, img_size, 0, nullptr, nullptr);
    
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_bvh_lower);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_bvh_higher);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_bvh_nodes);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_size);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_verts);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), &d_norms);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), &d_colors);
    clSetKernelArg(kernel, 7, sizeof(cl_mem), &output_image);
    clSetKernelArg(kernel, 8, sizeof(cl_mem), &d_accum_buffer);
    clSetKernelArg(kernel, 9, sizeof(cl_mem), &d_camera);
#endif
    
    glFinish();
    
    clEnqueueAcquireGLObjects(command_queue, 1, &output_image, 0, nullptr, nullptr);
    
    size_t global_wg[2] = {PIXELS_X,PIXELS_Y};
    size_t local_wg[2] = {8,8};
    clEnqueueNDRangeKernel(command_queue, kernel, 2, nullptr, global_wg, local_wg, 0, nullptr, nullptr);
    
    clFinish(command_queue);
    
    clEnqueueReadBuffer(command_queue, output_image, CL_TRUE, 0, sizeof(cl_uint4)*PIXELS_Y*PIXELS_X, pixels, 0, nullptr, nullptr);
    
    clEnqueueReleaseGLObjects(command_queue, 1, &output_image, 0, nullptr, nullptr);
    
    clFinish(command_queue);
}

void cleanupCL(){
#if USE_GRID
    clReleaseMemObject(d_grid_idx);
    clReleaseMemObject(d_arr_size);
    clReleaseMemObject(d_idx);
    clReleaseMemObject(d_grid_dim);
    clReleaseMemObject(d_voxel_w);
    clReleaseMemObject(d_grid_origin);
    clReleaseMemObject(d_f_num);
#endif
#if USE_BVH
    clReleaseMemObject(d_bvh_lower);
    clReleaseMemObject(d_bvh_higher);
    clReleaseMemObject(d_bvh_nodes);
    clReleaseMemObject(d_size);
#endif
    clReleaseMemObject(d_verts);
    clReleaseMemObject(d_norms);
    clReleaseMemObject(d_colors);
    clReleaseMemObject(output_image);
    clReleaseMemObject(d_accum_buffer);
    clReleaseMemObject(d_camera);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}

void cleanupGL(){
    glDeleteBuffers(1, &output_verts);
    glDeleteBuffers(1, &output_color);
    glDeleteProgram(programID);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void run_gl(){
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(programID);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, output_verts);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, output_color);
    glVertexAttribPointer(
                          1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_INT,           // type
                          GL_FALSE,           // normalized?
                          16,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    glDrawArrays(GL_POINTS, 0, 3*PIXELS_X*PIXELS_Y);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void user_input(){
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        camera[0].x = camera[0].x + 0.05*camera[1].x;
        camera[0].y = camera[0].y + 0.05*camera[1].y;
        camera[0].z = camera[0].z + 0.05*camera[1].z;
        framenumber = 0;
    }else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        camera[0].x = camera[0].x - 0.05*camera[1].x;
        camera[0].y = camera[0].y - 0.05*camera[1].y;
        camera[0].z = camera[0].z - 0.05*camera[1].z;
        framenumber = 0;
    }
}

int main(int argc, const char** argv){
    struct timeval start_time,end_time;
    
    if(initGL()==1)
        return 1;
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    if(initCL()==1)
        return 1;
    
    glFinish();
    
    setupCL();
    
    double lastTime = glfwGetTime();
    double rate = 0.0;
    int numFrames = 0;
    do{
        double currentTime = glfwGetTime();
        numFrames++;
        
        run_kernel();
        run_gl();
        
        framenumber++;
        
        user_input();
        
        char text[256];
        if(currentTime-lastTime >= 1.0){
            rate = 1000.0/(double)numFrames;
            lastTime += 1.0;
            numFrames = 0;
            cout<<"The time for one frame is "<<rate<<" ms/frame"<<endl;
        }
    }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window)==0);

    cleanupCL();
    cleanupGL();
    
    cout<<"Done"<<endl;
    
    free(pixels);
    
    return 0;
}

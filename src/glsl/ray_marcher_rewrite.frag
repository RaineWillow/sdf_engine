#version 420 compatibility
#extension GL_EXT_gpu_shader4 : enable

#define MAX_MARCHING_STEPS 40

uniform vec2 u_resolution;

//general object uniforms
//uniform int numObjects; //the number of objects in the scene

uniform sampler2D shapes;
uniform vec2 shapesBufferResolution;
uniform int shapesItemSize;

uniform sampler2D BVHUnion;
uniform vec2 BVHUnionBufferResolution;
uniform int BVHUnionItemSize;



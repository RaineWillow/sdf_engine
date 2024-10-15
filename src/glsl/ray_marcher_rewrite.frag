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

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shine;
  float reflectivity;
  float opacity;
  float IOR;
};

struct Surface {
  float sd;
  Material mat;
}

//quaternion transform functions---------------------------------------------------

vec4 quat(in vec3 v, in float a)
{
  return vec4(v * sin(a / 2.0), cos(a / 2.0));
}

vec4 quat_inv(in vec4 q)
{
  return vec4(-q.xyz, q.w);
}

vec4 p2q(in vec3 p)
{
  return vec4(p, 0);
}

vec4 q_mul(in vec4 q1, in vec4 q2)
{
  return vec4(q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y, 
              q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x, 
              q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w, 
              q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z);
}

vec3 rotateq(in vec3 p, in vec4 q)
{
  return q_mul(q_mul(q, p2q(p)), quat_inv(q)).xyz;
}

vec3 transform(vec3 p, vec3 offset, vec3 rotOrigin, vec4 quat) {
  //first, transform p by the offset
  p -= offset;
  //then, rotate p, keeping in mind the rotation origin
  p = rotateq(p-rotOrigin, quat) + rotOrigin;
  return p;
}
//---------------------------------------------------------------------------------

//memory access functions----------------------------------------------------------
vec4 accessMemoryParameter(sampler2D inBuffer, int index, int parameter, vec2 bufferResolution, int itemSize) {
  int maxIndexPerRow = int(bufferResolution.x/float(itemSize));

  int rowPosition = index/maxIndexPerRow;
  int colPosition = (index-rowPosition*maxIndexPerRow)*itemSize + parameter;

  return texelFetch(inBuffer, ivec2(colPosition, rowPosition), 0);
}

vec4 accessShapeParameter(int index, int parameter) {
  return accessMemoryParameter(shapes, index, parameter, shapesBufferResolution, shapesItemSize);
}

vec4 accessBVHUnionParameter(int index, int parameter) {
  return accessMemoryParameter(BVHUnion, index, parameter, BVHUnionBufferResolution, BVHUnionItemSize);
}
//end------------------------------------------------------------------------------

//memory conversion----------------------------------------------------------------
struct dInt {
  int val1;
  int val2;
};

struct iBool {
  bool truth;
  int val;
};

struct Pointer {
  int address;
  int type;
};

int convertPixToInt(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int num = ((set0 << 24) + (set1 << 16) + (set2 << 8) + set3);

  return num;
}

float convertPixToNum(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int num = ((set0 << 24) + (set1 << 16) + (set2 << 8) + set3);

  return intBitsToFloat(num);
}

dInt convertPixToDInt(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int num1 = ((set0 << 8) + set1);
  int num2 = ((set2 << 8) + set3);

  return dInt(num1, num2);
}

iBool convertPixToIBool(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int num = ((set0 << 16) + (set1 << 8) + set2);

  return iBool(set3 > 0, num);
}

bool convertPixToBool(vec4 pixel) {
  return pixel.r > 0.0;
}

vec3 convertPixToCol(vec4 pixel) {
  return pixel.rgb;
}

vec4 convertPixToFDat(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  return vec4(set0, set1, set2, set3);
}

Pointer convertPixToPointer(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int num = (set0 << 16) + (set1 << 8) + set2;

  return Pointer(num, set3);
}

//end------------------------------------------------------------------------------

//signed distance functions-------------------------------------------------------
float sdSphere(vec3 p, float r) {
  return length(p) - r;
}

float sdBox(vec3 p, vec3 b) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}
//end------------------------------------------------------------------------------

//BVH Tree intersecting functions--------------------------------------------------
vec2 boxIntersection( in vec3 ro, in vec3 rd, vec3 boxSize) 
{
    vec3 m = 1.0/rd; // can precompute if traversing a set of aligned boxes
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return vec2(-1.0); // no intersection
    return vec2( tN, tF );
}

vec2 boxIntersectionPrecomupute( in vec3 ro, in vec3 rd, vec3 boxSize, vec3 m) 
{
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return vec2(-1.0); // no intersection
    return vec2( tN, tF );
}

float sdAxisBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

struct AABB {
  vec3 pos;
  vec3 bound;
};

AABB getBoundingBoxFromPointer(Pointer item) {
  if (item.type == 2) {
    vec3 boxPos = vec3(
      convertPixToNum(accessBVHUnionParameter(item.address, 1)),
      convertPixToNum(accessBVHUnionParameter(item.address, 2)),
      convertPixToNum(accessBVHUnionParameter(item.address, 3))
    );
    vec3 boxSize = vec3(
      convertPixToNum(accessBVHUnionParameter(item.address, 4)),
      convertPixToNum(accessBVHUnionParameter(item.address, 5)),
      convertPixToNum(accessBVHUnionParameter(item.address, 6))
    );

    return AABB(boxPos, boxSize);
  } else if (item.type == 1) {
    vec3 boxPos = vec3(
      convertPixToNum(accessShapeParameter(item.address, 2)) + convertPixToNum(accessShapeParameter(item.address, 12)), 
      convertPixToNum(accessShapeParameter(item.address, 3)) + convertPixToNum(accessShapeParameter(item.address, 13)), 
      convertPixToNum(accessShapeParameter(item.address, 4)) + convertPixToNum(accessShapeParameter(item.address, 14))
    );
    vec3 boxSize = vec3(
      convertPixToNum(accessShapeParameter(item.address, 15)), 
      convertPixToNum(accessShapeParameter(item.address, 16)), 
      convertPixToNum(accessShapeParameter(item.address, 17))
    );

    return AABB(boxPos, boxSize);
  }
}
//end------------------------------------------------------------------------------

//stack recursion defs-------------------------------------------------------------
struct BVHStackFrame {
  Pointer currentAddress;
  int children[8];
  int remainingChildren;
  bool processed;
};

struct OrganizerFrame {
  int next;
  float dist;
  int child;
};

const int maxStackSize = 50;

BVHStackFrame BVHStack[maxStackSize];
int itemsOnBVHStack = 0;


void pushBVHStackFrame(Pointer newItem) {
  if (itemsOnBVHStack < maxStackSize) {
    BVHStack[itemsOnBVHStack] = BVHStackFrame(newItem, int[8](0, 0, 0, 0, 0, 0, 0, 0), 0, false);
    itemsOnBVHStack += 1;
  }
}

void popBVHStackFrame() {
  itemsOnBVHStack -= 1;
}

void clearBVHStack() {
  itemsOnBVHStack = 0;
}
//end------------------------------------------------------------------------------

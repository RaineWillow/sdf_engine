#version 420 compatibility
#extension GL_EXT_gpu_shader4 : enable

precision highp float;
precision highp int;

#define MAX_MARCHING_STEPS 40
#define PRECISION 0.01

uniform vec2 u_resolution;

//general object uniforms
//uniform int numObjects; //the number of objects in the scene

#define NULL_PTR_TYPE 0

#define SHAPES_PTR_TYPE 1
uniform sampler2D shapes;
uniform vec2 shapesBufferResolution;
uniform int shapesItemSize;
uniform vec4 defaultMatPointer;
uniform vec4 headLightPointer;

#define BVH_PTR_TYPE 2
uniform sampler2D BVHUnion;
uniform vec2 BVHUnionBufferResolution;
uniform int BVHUnionItemSize;

uniform vec3 cameraPosition;
uniform vec3 cameraForward;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

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
  int set0 = int(round(pixel.r*255.0));
  int set1 = int(round(pixel.g*255.0));
  int set2 = int(round(pixel.b*255.0));
  int set3 = int(round(pixel.a*255.0));

  int num = ((set0 << 24) | (set1 << 16) | (set2 << 8) | set3);

  return num;
}

float convertPixToNum(vec4 pixel) {
  int set0 = int(round(pixel.r*255.0));
  int set1 = int(round(pixel.g*255.0));
  int set2 = int(round(pixel.b*255.0));
  int set3 = int(round(pixel.a*255.0));

  int num = ((set0 << 24) | (set1 << 16) | (set2 << 8) | set3);

  return intBitsToFloat(num);
}

dInt convertPixToDInt(vec4 pixel) {
  int set0 = int(round(pixel.r*255.0));
  int set1 = int(round(pixel.g*255.0));
  int set2 = int(round(pixel.b*255.0));
  int set3 = int(round(pixel.a*255.0));

  int num1 = ((set0 << 8) | set1);
  int num2 = ((set2 << 8) | set3);

  return dInt(num1, num2);
}

iBool convertPixToIBool(vec4 pixel) {
  int set0 = int(round(pixel.r*255.0));
  int set1 = int(round(pixel.g*255.0));
  int set2 = int(round(pixel.b*255.0));
  int set3 = int(round(pixel.a*255.0));

  int num = ((set0 << 16) | (set1 << 8) | set2);

  return iBool(set3 > 0, num);
}

bool convertPixToBool(vec4 pixel) {
  return pixel.r > 0.0;
}

vec3 convertPixToCol(vec4 pixel) {
  return pixel.rgb;
}

vec4 convertPixToFDat(vec4 pixel) {
  int set0 = int(round(pixel.r*255.0));
  int set1 = int(round(pixel.g*255.0));
  int set2 = int(round(pixel.b*255.0));
  int set3 = int(round(pixel.a*255.0));

  return vec4(set0, set1, set2, set3);
}

Pointer convertPixToPointer(vec4 pixel) {
  int set0 = int(round(pixel.r*255.0));
  int set1 = int(round(pixel.g*255.0));
  int set2 = int(round(pixel.b*255.0));
  int set3 = int(round(pixel.a*255.0));

  int num = (set0 << 16) | (set1 << 8) | set2;

  return Pointer(num, set3);
}

//end------------------------------------------------------------------------------

struct Material {
  vec3 albedo;
  float metallic;
  float shine;
  float reflectivity;
  float opacity;
  float IOR;
};

struct Surface {
  vec3 normal;
  Material mat;
};

struct RayResult {
  vec3 p;
  bool hit;
  Pointer hitAddr;
};

struct Light {
  vec3 position;
  vec3 color;
  float intensity;
  vec3 attenuation;
  Pointer next;
};

//quaternion transform functions---------------------------------------------------

vec4 quat(in vec3 v, in float a) {
  return vec4(v * sin(a / 2.0), cos(a / 2.0));
}

vec4 quat_inv(in vec4 q) {
  return vec4(-q.xyz, q.w);
}

vec4 p2q(in vec3 p) {
  return vec4(p, 0);
}

vec4 q_mul(in vec4 q1, in vec4 q2) {
  return vec4(q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y, 
              q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x, 
              q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w, 
              q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z);
}

vec3 rotateq(in vec3 p, in vec4 q) {
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

//Primitive signed distance functions----------------------------------------------
#define SD_SPHERE 0
float sdSphere(vec3 p, float r) {
  return length(p) - r;
}

#define SD_BOX 1
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
  if (item.type == BVH_PTR_TYPE) {
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
  } else if (item.type == SHAPES_PTR_TYPE) {
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
#define maxBVHStackSize 20

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

BVHStackFrame BVHStack[maxBVHStackSize];
int itemsOnBVHStack = 0;


void pushBVHStackFrame(Pointer newItem) {
  if (itemsOnBVHStack < maxBVHStackSize) {
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

#define maxShapeStackSize 40

struct ShapeStackFrame {
  Pointer currentAddress;
  vec3 pos;
  int remainingChildren;
  bool processed;
};

ShapeStackFrame shapeStack[maxShapeStackSize];
int shapesOnShapeStack = 0;

void pushShapeStackFrame(Pointer newItem) {
  if (shapesOnShapeStack < maxShapeStackSize) {
    shapeStack[shapesOnShapeStack] = ShapeStackFrame(newItem, vec3(0), 0, false);
    shapesOnShapeStack += 1;
  }
}

void popShapeStackFrame() {
  shapesOnShapeStack -= 1;
}

void clearShapeStack() {
  shapesOnShapeStack = 0;
}

//end------------------------------------------------------------------------------

Material materialFromPointer(Pointer matPointer) {
  vec3 albedo = convertPixToCol(accessShapeParameter(matPointer.address, 0));
  float metallic = convertPixToNum(accessShapeParameter(matPointer.address, 1));
  float shine = convertPixToNum(accessShapeParameter(matPointer.address, 2));
  float reflectivity = convertPixToNum(accessShapeParameter(matPointer.address, 3));
  float opacity = convertPixToNum(accessShapeParameter(matPointer.address, 4));
  float IOR = convertPixToNum(accessShapeParameter(matPointer.address, 5));

  return Material(albedo, metallic, shine, reflectivity, opacity, IOR);
}

Light lightFromPointer(Pointer lightPointer) {
  vec3 position = vec3(
    convertPixToNum(accessShapeParameter(lightPointer.address, 0)),
    convertPixToNum(accessShapeParameter(lightPointer.address, 1)),
    convertPixToNum(accessShapeParameter(lightPointer.address, 2))
  );

  vec3 color = convertPixToCol(accessShapeParameter(lightPointer.address, 3));
  float intensity = convertPixToNum(accessShapeParameter(lightPointer.address, 4));

  vec3 attenuation = vec3(
    convertPixToNum(accessShapeParameter(lightPointer.address, 5)),
    convertPixToNum(accessShapeParameter(lightPointer.address, 6)),
    convertPixToNum(accessShapeParameter(lightPointer.address, 7))
  );

  Pointer next = convertPixToPointer(accessShapeParameter(lightPointer.address, 8));

  return Light(position, color, intensity, attenuation, next);
}

float sdPrimitive(vec3 p, Pointer currentShape, int objID, float boundRadius) {
  vec4 param_26 = accessShapeParameter(currentShape.address, 26);
  vec4 param_27 = accessShapeParameter(currentShape.address, 27);
  vec4 param_28 = accessShapeParameter(currentShape.address, 28);
  

  if (objID == SD_SPHERE) {
    float sphereRadius = convertPixToNum(param_26);
    return sdSphere(p, sphereRadius);
  } else if (objID == SD_BOX) {
    vec3 boxSize = vec3(
      convertPixToNum(param_26),
      convertPixToNum(param_27),
      convertPixToNum(param_28)
    );
    return sdBox(p, boxSize);
  }

  return boundRadius;
}

float sdPrimitiveWithTransform(vec3 p, Pointer currentShape, float boundRadius) {

  int objID = convertPixToIBool(accessShapeParameter(currentShape.address, 0)).val;
  
  
  vec3 objOffset = vec3(
    convertPixToNum(accessShapeParameter(currentShape.address, 2)),
    convertPixToNum(accessShapeParameter(currentShape.address, 3)),
    convertPixToNum(accessShapeParameter(currentShape.address, 4))
  );

  vec4 shapeQuat = vec4(
    convertPixToNum(accessShapeParameter(currentShape.address, 5)),
    convertPixToNum(accessShapeParameter(currentShape.address, 6)),
    convertPixToNum(accessShapeParameter(currentShape.address, 7)),
    convertPixToNum(accessShapeParameter(currentShape.address, 8))
  );

  vec3 rotOrigin = vec3(
    convertPixToNum(accessShapeParameter(currentShape.address, 9)),
    convertPixToNum(accessShapeParameter(currentShape.address, 10)),
    convertPixToNum(accessShapeParameter(currentShape.address, 11))
  );

  p = transform(p, objOffset, rotOrigin, shapeQuat);
  
  return sdPrimitive(p, currentShape, objID, boundRadius);
}

float sdShapeNode(vec3 p, Pointer address, float boundRadius) {
  
  pushShapeStackFrame(address);

  float minDist = boundRadius;

  while (shapesOnShapeStack > 0) {
    int stackPointer = shapesOnShapeStack-1;

    Pointer currentShape = shapeStack[stackPointer].currentAddress;
    iBool objectId = convertPixToIBool(accessShapeParameter(currentShape.address, 0));
    
    
    //check to see if the node is a primative node
    minDist = min(minDist, objectId.truth ? sdPrimitiveWithTransform(p, currentShape, boundRadius) : 0.0);
    shapesOnShapeStack = objectId.truth ? shapesOnShapeStack - 1 : shapesOnShapeStack;
  }
  clearShapeStack();
  return minDist;
}

RayResult castRay(vec3 ro, vec3 rd, float boundRadius) {
  vec3 p = ro;

  vec3 m = 1.0/rd;

  Pointer root = Pointer(0, 2);

  bool hit = false;

  Pointer hitAddr;

  AABB rootBound = getBoundingBoxFromPointer(root);

  if (boxIntersectionPrecomupute(p-rootBound.pos, rd, rootBound.bound, m).y != -1.0) {
    pushBVHStackFrame(root);
  }

  float depth = boundRadius;

  while (itemsOnBVHStack > 0) {
    int stackPointer = itemsOnBVHStack-1;
    Pointer currentItem = BVHStack[stackPointer].currentAddress;

    if (BVHStack[stackPointer].processed) {
      int remainingChildren = BVHStack[stackPointer].remainingChildren;
      if (remainingChildren > 0) {
        Pointer nextChild = convertPixToPointer(accessBVHUnionParameter(currentItem.address, BVHStack[stackPointer].children[remainingChildren-1]));
        pushBVHStackFrame(nextChild);
        BVHStack[stackPointer].remainingChildren -= 1;
      } else {
        popBVHStackFrame();
      }
    } else if (currentItem.type == BVH_PTR_TYPE) {
      //algorithm for sorting objects by distance and inserting in order by distance and intersection
      ///*
      OrganizerFrame data[8];
      int closestPos = 0;
      int numChildren = 0;

      for (int i = 0; i < 8; i++) {
        Pointer currentChild = convertPixToPointer(accessBVHUnionParameter(currentItem.address, 7+i));
        if (currentChild.type != NULL_PTR_TYPE) {
          AABB currentBound = getBoundingBoxFromPointer(currentChild);
          float dist = sdAxisBox(p-currentBound.pos, currentBound.bound);
          if (boxIntersectionPrecomupute(p-currentBound.pos, rd, currentBound.bound, m).y != -1.0 && dist < boundRadius) {
            int foundPos = closestPos;
            data[numChildren] = OrganizerFrame(-1, dist, 7+i);

            int nextClosest = closestPos;

            for (int j = 0; j < numChildren; j++) {
              if (data[closestPos].dist >= dist) {
                data[numChildren].next = closestPos;
                closestPos = numChildren;
                break;
              }

              if (data[nextClosest].dist <= dist) {
                if (data[nextClosest].next == -1) {
                  data[nextClosest].next=numChildren;
                  break;
                } else if (data[data[nextClosest].next].dist >= dist) {
                  data[numChildren].next = data[nextClosest].next;
                  data[nextClosest].next = numChildren;
                  break;
                } else {
                  nextClosest = data[nextClosest].next;
                }
              }
            }
            numChildren += 1;
          }
        }
      }

      BVHStack[stackPointer].remainingChildren = numChildren;
      int nextClosest = closestPos;

      for (int i = 0; i < numChildren; i++) {
        int currentChild = data[nextClosest].child;
        BVHStack[stackPointer].children[numChildren-1-i] = currentChild;
        nextClosest = data[nextClosest].next;
      }
      //*/
      //end of algorithm

      //algorithm for dumping in objects irrespective of order or actual intersection
      /*
      for (int i = 0; i < 8; i++) {
        Pointer currentChild = convertPixToPointer(accessBVHUnionParameter(currentItem.address, 7+i));
        if (currentChild.type != NULL_PTR_TYPE) {
          BVHStack[stackPointer].children[BVHStack[stackPointer].remainingChildren] = 7+i;
          BVHStack[stackPointer].remainingChildren += 1;
        }
      }
      */
      //end of algorithm
      BVHStack[stackPointer].processed = true;
    } else if (currentItem.type == SHAPES_PTR_TYPE) {
      AABB objectBound = getBoundingBoxFromPointer(currentItem);

      vec2 intersectionPoints = boxIntersectionPrecomupute(p - objectBound.pos, rd, objectBound.bound, m);

      float currentDepth = max(intersectionPoints.x, 0.0);

      if (depth >= currentDepth) {
        float maxDist = intersectionPoints.y;
        float lastDist = boundRadius;
        float numREPEATS = 0.0;
        for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
          float dist = sdShapeNode(p + currentDepth * rd, currentItem, boundRadius);
          //float dist = sdPrimitiveWithTransform(p+currentDepth*rd, currentItem, boundRadius);
          //pushShapeStackFrame(currentItem);
          //popShapeStackFrame();
          if (currentDepth >= maxDist || currentDepth >= depth || lastDist < dist) {
            break;
          } else if (dist <= PRECISION) {
            if (currentDepth <= depth) {
              hit = true;
              hitAddr = currentItem;
              //hit = numREPEATS/float(MAX_MARCHING_STEPS);
              depth = currentDepth;
            }
            break;
          }

          currentDepth += dist;
          lastDist = dist;
        }
        //hit = numREPEATS/float(MAX_MARCHING_STEPS);
      }
      
      popBVHStackFrame();
    }
  }

  clearBVHStack();

  return RayResult(p + rd*depth, hit, hitAddr);
}

vec3 calcNormal(vec3 p, Pointer address, float boundRadius) {
  vec2 e = vec2(1.0, -1.0) * 0.0005; //epsilon

  return normalize(
    e.xyy * sdShapeNode(p + e.xyy, address, boundRadius) +
    e.yyx * sdShapeNode(p + e.yyx, address, boundRadius) +
    e.yxy * sdShapeNode(p + e.yxy, address, boundRadius) +
    e.xxx * sdShapeNode(p + e.xxx, address, boundRadius)
  );
}

vec3 calcLights(vec3 p, vec3 normal, vec3 rd, Material mat) {
  Pointer next = convertPixToPointer(headLightPointer);
  vec3 ambientLight = vec3(0.0);
  vec3 partialLight = vec3(0.0);

  while (next.type != NULL_PTR_TYPE) {
    Light nextLight = lightFromPointer(next);

    float lightDist = length(nextLight.position - p);
    float attenuation = 1.0 / (nextLight.attenuation.x + nextLight.attenuation.y * lightDist + nextLight.attenuation.z * lightDist * lightDist);

    vec3 lightDir = normalize(nextLight.position - p);

    //begin generating the ambient component
    ambientLight += nextLight.color * nextLight.intensity;

    float dotLN = clamp(dot(lightDir, normal), 0., 1.);
    vec3 diffuse = mat.albedo * nextLight.color * dotLN * attenuation;

    float dotRV = clamp(dot(reflect(-lightDir, normal), -rd), 0., 1.);
    vec3 specular = nextLight.color * pow(dotRV, mat.shine) * smoothstep(1.0, 10.0, mat.shine);

    partialLight += attenuation * (diffuse + specular);

    next = nextLight.next;
  }

  ambientLight = ambientLight * mat.albedo * 0.1;

  return clamp(ambientLight + partialLight, vec3(0.0), vec3(1.0));
}

void main() {
  vec2 uv = (gl_FragCoord.xy-.5*u_resolution.xy)/u_resolution.y;
  vec3 backgroundColor = vec3(0.0, 0.0, 0.0);

  vec3 col = backgroundColor;

  vec3 ro = cameraPosition;
  vec3 rd = mat3(cameraRight, cameraUp, -cameraForward) * normalize(vec3(uv, -1.2));

  RayResult foundIntersection = castRay(ro, rd, 2000.);

  if (foundIntersection.hit==true) {
    vec3 normal = calcNormal(foundIntersection.p, foundIntersection.hitAddr, 2000.);

    Material hitMat = materialFromPointer(convertPixToPointer(defaultMatPointer));

    col = calcLights(foundIntersection.p, normal, rd, hitMat);
  }

  gl_FragColor = vec4(col, 1.0);
}

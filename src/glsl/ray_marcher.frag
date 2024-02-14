#define maxObjects 10000


uniform vec2 u_resolution;

/*
uniform int objects[maxObjects]; //the ids of the objects in the scene
uniform vec3 objectPos[maxObjects]; //the positions of the objects in the scene
uniform vec4 objectRot[maxObjects]; //the rotation of the object in the scene
uniform float objectBoundR[maxObjects]; //the bounding sphere of the object
uniform vec3 objectCol[maxObjects]; //the colors of objects in the scene
*/

//general object uniforms
uniform int numObjects; //the number of objects in the scene

uniform sampler2D shapes;
uniform vec2 shapesBufferResolution;
uniform int shapesItemSize;

uniform sampler2D BVHUnion;
uniform vec2 BVHUnionBufferResolution;
uniform int BVHUnionItemSize;

const int MAX_MARCHING_STEPS = 100;

int maxIterations = 1024;


int numShapeTests = 0;

struct Material {
  vec3 col;
};

struct Surface {
  float sd;
  Material mat;
};

struct Ray {
  float sd;
  Material mat;
  bool hit;
  int id;
  bool altColor;
  int numShapeTests;
};

vec3 transform(vec3 p, vec3 q) {
  return p-q;
}

//memory access functions----------------------------------------------------------
vec4 accessMemoryParameter(sampler2D inBuffer, int index, int parameter, vec2 bufferResolution, int itemSize) {
  int maxIndexPerRow = int(bufferResolution.x/float(itemSize));

  int rowPosition = index/maxIndexPerRow;
  int colPosition = (index-rowPosition*maxIndexPerRow)*itemSize + parameter;

  return texelFetch(inBuffer, ivec2(colPosition, rowPosition), 0);
}

vec4 accessShapeParameter(int shapeIndex, int parameter) {
  return accessMemoryParameter(shapes, shapeIndex, parameter, shapesBufferResolution, shapesItemSize);
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

  int signBit = set0 >> 7;

  int num = ((set0 << 16) + (set1 << 8) + set2) - 16777216 * signBit;

  return num;
}

float convertPixToNum(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int signBit = set0 >> 7;

  int num;
  float mantissa;

  if (set2 == 255 && set3 == 255) {
    num = 0;
    mantissa = -(float(set0*256 + set1)/65535.0);
  } else {
    num = ((set0 << 8) + set1) - 65536*signBit;
    mantissa = (float(set2*256 + set3)/65535.0) * float(-signBit | 1);
  }


  return float(num) + mantissa;
}

float convertPixToHighFloat(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);


  int num = set0;
  float mantissa = float((set1 << 16) + (set2 << 8) + set3)/16777215.0;
  if (num > 127) {
    num -= 256;
    mantissa = -mantissa;
  }

  return float(num) + mantissa;
}

dInt convertPixToDInt(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  ivec2 signBits = ivec2(set0 << 7, set2 << 7);

  int num1 = (set0*256 + set1) - 65536*signBits.x;
  int num2 = (set2*256 + set3) - 65536*signBits.y;

  return dInt(num1, num2);
}

iBool convertPixToIBool(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int signBit = set0 >> 7;

  int num = ((set0 << 16) + (set1 << 16) + set2) - 16777216*signBit;

  return iBool(set3 > 0, num);
}

bool convertPixToBool(vec4 pixel) {
  return pixel.r > 0.0;
}

vec3 convertPixToCol(vec4 pixel) {
  return pixel.rgb;
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

//surface distance functions-------------------------------------------------------
Surface sdSphere(vec3 p, float r, Material mat) {
  return Surface(length(p) - r, mat);
}
//end------------------------------------------------------------------------------

Ray rayUnion(Ray ray1, Ray ray2) {
  if (ray2.sd < ray1.sd) return ray2;
  return ray1;
}

//BVH Tree intersecting functions--------------------------------------------------
vec2 sphIntersect( in vec3 ro, in vec3 rd, in vec3 ce, float ra )
{
    vec3 oc = ro - ce;
    float b = dot( oc, rd );
    vec3 qc = oc - b*rd;
    float h = ra*ra - dot( qc, qc );
    if( h<0.0 ) return vec2(-1.0); // no intersection
    h = sqrt( h );
    return vec2( -b-h, -b+h );
}

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

float sdAxisBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdFloor(vec3 p) {
  return p.y + 30.;
}
//end------------------------------------------------------------------------------

//stack recursion defs-------------------------------------------------------------
struct BVHStackFrame {
  Pointer currentAddress;
  int children[8];
  int remainingChildren;
  bool processed;
};

const int maxStackSize = 100;

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

//Ray drawPrimative(vec3 p, float prec, int index);

//<- insertion point for all ShapeComposites

Ray drawObject(vec3 p, float prec, int index) {

  iBool objectId = convertPixToIBool(accessShapeParameter(index, 0));
  if (objectId.val == 0) {
    vec3 objPos = vec3(convertPixToNum(accessShapeParameter(index, 2)), convertPixToNum(accessShapeParameter(index, 3)), convertPixToNum(accessShapeParameter(index, 4)));
    float objectRadius = convertPixToNum(accessShapeParameter(index, 32));
    Material objectMat = Material(convertPixToCol(accessShapeParameter(index, 13)));
    Surface obj = sdSphere(transform(p, objPos), objectRadius, objectMat);
    return Ray(obj.sd, obj.mat, obj.sd <= prec, index, false, 0);
  }
}

vec3 calcNormal(vec3 p, float prec, int index) {
    vec2 e = vec2(1.0, -1.0) * 0.0005; // epsilon
    return normalize(
      e.xyy * drawObject(p + e.xyy, prec, index).sd +
      e.yyx * drawObject(p + e.yyx, prec, index).sd +
      e.yxy * drawObject(p + e.yxy, prec, index).sd +
      e.xxx * drawObject(p + e.xxx, prec, index).sd
    );
}

Ray rayMarch(vec3 ro, vec3 rd, float boundRadius, vec3 backgroundColor) {
  Material defaultColor = Material(backgroundColor);
  Ray closest = Ray(boundRadius, defaultColor, false, -1, false, 0);
  vec3 p = ro;
  float newI = 0.0;

  Pointer root = Pointer(0, 2);
  pushBVHStackFrame(root);
  int numIterations = 0;

  float lastDepth = boundRadius;

  while (itemsOnBVHStack > 0) {
    int pointerPosition = itemsOnBVHStack-1;
    Pointer currentItem = BVHStack[pointerPosition].currentAddress;

    if (BVHStack[pointerPosition].processed) {
      int remainingChildren = BVHStack[pointerPosition].remainingChildren;
      if (remainingChildren > 0) {
        Pointer nextChild = convertPixToPointer(accessBVHUnionParameter(currentItem.address, BVHStack[pointerPosition].children[remainingChildren-1]));
        pushBVHStackFrame(nextChild);
        BVHStack[pointerPosition].remainingChildren -= 1;
      } else {
        popBVHStackFrame();
      }
    } else if (currentItem.type == 2) {
      vec3 boxPos = vec3(
        convertPixToNum(accessBVHUnionParameter(currentItem.address, 1)),
        convertPixToNum(accessBVHUnionParameter(currentItem.address, 2)),
        convertPixToNum(accessBVHUnionParameter(currentItem.address, 3))
      );
      vec3 boxSize = vec3(
        convertPixToNum(accessBVHUnionParameter(currentItem.address, 4)),
        convertPixToNum(accessBVHUnionParameter(currentItem.address, 5)),
        convertPixToNum(accessBVHUnionParameter(currentItem.address, 6))
      );

      if (boxIntersection(transform(p, boxPos), rd, boxSize).y != -1) {
        for (int i = 0; i < 8; i++) {
          Pointer currentChild = convertPixToPointer(accessBVHUnionParameter(currentItem.address, 7+i));
          if (currentChild.type != 0) {
            BVHStack[pointerPosition].children[BVHStack[pointerPosition].remainingChildren] = 7+i;
            BVHStack[pointerPosition].remainingChildren += 1;
          }
        }

        BVHStack[pointerPosition].processed=true;
      } else {
        popBVHStackFrame();
      }
    } else if (currentItem.type == 1) {
      int i = currentItem.address;
      vec3 objPos = vec3(convertPixToNum(accessShapeParameter(i, 2)), convertPixToNum(accessShapeParameter(i, 3)), convertPixToNum(accessShapeParameter(i, 4)));
      float objectRadius = convertPixToNum(accessShapeParameter(i, 9));
      vec3 objBound = vec3(convertPixToNum(accessShapeParameter(i, 10)), convertPixToNum(accessShapeParameter(i, 11)), convertPixToNum(accessShapeParameter(i, 12)));
      vec3 objectCol = convertPixToCol(accessShapeParameter(i, 13));
      vec2 intersectionPoints = sphIntersect(p, rd, objPos, objectRadius);
      if (intersectionPoints.y != -1) {
        float maxDist = intersectionPoints.y;
        Ray nextClosest = Ray(boundRadius, defaultColor, false, -1, false, 0);
        float depth = intersectionPoints.x;
        for (int j = 0; j < MAX_MARCHING_STEPS; j++) {
          nextClosest = rayUnion(nextClosest, drawObject(p+depth*rd, 0.01, i));
          if (nextClosest.hit || depth >= maxDist) {
            break;
          } else {
            depth += nextClosest.sd;
          }
        }
        if (nextClosest.hit) {
          if (depth < lastDepth) {
            closest = nextClosest;
            lastDepth = depth;
          }
        }
      }
      popBVHStackFrame();
    }
  }

  p = p+lastDepth*rd;
  vec3 normal = calcNormal(p, 0.01, closest.id);

  if (closest.hit) {
    vec3 lightPosition = ro;
    vec3 lightDirection = normalize(lightPosition-p);
    float dif = clamp(dot(normal, lightDirection), 0.1, 1.);
    closest.mat.col*=dif;
  } else {
    if (!closest.altColor) {
      closest.mat.col = backgroundColor;
    }
  }

  //float shapesTested = float(closest.numShapeTests)/200.0;

  //vec3 red = vec3(1.0, 0.0, 0.0);
  //vec3 black = vec3(0.0, 0.0, 0.0);
  //closest.col = mix(black, red, min(1.0, newI));
  //closest.col.g = shapesTested;
  //closest.hit=true;
  
  return closest;
}

void main() {
  vec2 uv = (gl_FragCoord.xy-.5*u_resolution.xy)/u_resolution.y;
  vec3 backgroundColor = vec3(0.5, 0.5, 1.0);

  vec3 col = vec3(0);

  vec3 ro = vec3(0, 0, -25);
  vec3 rd = normalize(vec3(uv, 1.2));


  Ray result = rayMarch(ro, rd, 1024., backgroundColor);
  col = result.mat.col;
  if (result.hit) {
    col = pow(col, vec3(0.7));
  }

  /*
  vec2 bCheck = boxIntersection(ro-ro, rd, vec3(10.0, 10.0, 10.0));
  if (bCheck.y > 14.0) {
    col = vec3(0., 1., 0.);
  } else {
    col = vec3(1.0, 0.0, 0.0);
  }
  */
/*
  if (convertPixToNum(accessShapeParameter(1, 2)) == 4.0) {
    col = vec3(0.0, 1.0, 0.0);
  } else {
    col = vec3(1.0, 0.0, 0.0);
  }
*/


  gl_FragColor = vec4(col, 1.0);
}

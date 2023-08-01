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
uniform vec2 memoryBufferResolution;
uniform sampler2D memoryBuffer;

const int MAX_MARCHING_STEPS = 255;

struct Surface {
  float sd;
  vec3 col;
};

struct Ray {
  float sd;
  vec3 col;
  bool hit;
  int id;
};

vec3 transform(vec3 p, vec3 q) {
  return p-q;
}

vec4 accessShapeParameter(int shapeIndex, int parameter) {
  int maxIndexPerRow = int(memoryBufferResolution.x/96.0);

  int rowPosition = shapeIndex/maxIndexPerRow;
  int colPosition = (shapeIndex - rowPosition*maxIndexPerRow)*96 + parameter;

  return texelFetch(memoryBuffer, ivec2(colPosition, rowPosition), 0);
}

struct dInt {
  int val1;
  int val2;
};

struct iBool {
  bool truth;
  int val;
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

  int num = ((set0 << 8) + set1) - 65536*signBit;
  float mantissa = (float(set2*256 + set3)/65535.0) * (-signBit | 1);


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

  ivec2 signBits = vec2(set0 << 7, set2 << 7);

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

Surface sdSphere(vec3 p, float r, vec3 col) {
  return Surface(length(p) - r, col);
}

Ray rayUnion(Ray ray1, Ray ray2) {
  if (ray2.sd < ray1.sd) return ray2;
  return ray1;
}

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

Ray drawObject(vec3 p, float prec, int index) {

  iBool objectId = convertPixToIBool(accessShapeParameter(index, 0));
  if (objectId.val == 0) {
    vec3 objPos = vec3(convertPixToNum(accessShapeParameter(index, 2)), convertPixToNum(accessShapeParameter(index, 3)), convertPixToNum(accessShapeParameter(index, 4)));
    float objectRadius = convertPixToNum(accessShapeParameter(index, 9));
    vec3 objectCol = convertPixToCol(accessShapeParameter(index, 13));
    Surface obj = sdSphere(transform(p, objPos), objectRadius, objectCol);
    return Ray(obj.sd, obj.col, obj.sd <= prec, index);
  }
}

Ray drawObjects(vec3 p, vec3 rd, float prec, float boundRadius) {
  int index = 0;
  Ray closest = Ray(boundRadius, vec3(0), false, -1);

  for (int i = 0; i < maxObjects; i++) {
    if (i > numObjects || closest.hit) {
      break;
    }

    vec3 objPos = vec3(convertPixToNum(accessShapeParameter(i, 2)), convertPixToNum(accessShapeParameter(i, 3)), convertPixToNum(accessShapeParameter(i, 4)));
    float objectRadius = convertPixToNum(accessShapeParameter(i, 9));
    if (sphIntersect(p, rd, objPos, objectRadius).x != -1) {
      closest = rayUnion(closest, drawObject(p, prec, i));
    }
  }
  return closest;
}

vec3 calcNormal(vec3 p, float prec, int index) {
    vec2 e = vec2(1.0, -1.0) * 0.0005; // epsilon
    return normalize(
      e.xyy * drawObject(p + e.xyy, prec, index).sd +
      e.yyx * drawObject(p + e.yyx, prec, index).sd +
      e.yxy * drawObject(p + e.yxy, prec, index).sd +
      e.xxx * drawObject(p + e.xxx, prec, index).sd);
}

Ray rayMarch(vec3 ro, vec3 rd, float boundRadius) {
  Ray closest = Ray(boundRadius, vec3(0), false, -1);
  vec3 p = ro;
  float newI = 0.0;
  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
    closest = drawObjects(p, rd, 0.005, boundRadius);
    //closest.sd = max(sdCapsule(p, p, p+rd*boundRadius, 0.1), closest.sd);
    if (closest.hit || closest.sd >= boundRadius) {
      break;
    } else {
      p += rd*closest.sd;
    }
    newI = float(i)/float(MAX_MARCHING_STEPS);
  }




  if (closest.hit) {
    vec3 normal = calcNormal(p, 0.005, closest.id);
    vec3 lightPosition = ro;
    vec3 lightDirection = normalize(lightPosition-p);
    float dif = clamp(dot(normal, lightDirection), 0.1, 1.);
    closest.col*=dif;
  }
/*
  vec3 red = vec3(1.0, 0.0, 0.0);
  vec3 black = vec3(0.0, 0.0, 0.0);
  closest.col = mix(black, red, min(1.0, newI));
  closest.hit=true;
  */
  return closest;
}

void main() {
  vec2 uv = (gl_FragCoord.xy-.5*u_resolution.xy)/u_resolution.y;
  vec3 backgroundColor = vec3(0.5, 0.5, 1.0);

  vec3 col = vec3(0);

  vec3 ro = vec3(0, 0, -10);
  vec3 rd = normalize(vec3(uv, 1.2));


  Ray result = rayMarch(ro, rd, 255.);
  if (result.hit) {
    col = result.col;
    col = pow(col, vec3(0.7));
  } else {
    col = backgroundColor;
  }
/*
  if (convertPixToNum(accessShapeParameter(1, 2)) == 4.0) {
    col = vec3(0.0, 1.0, 0.0);
  } else {
    col = vec3(1.0, 0.0, 0.0);
  }
*/


  gl_FragColor = vec4(col, 1.0);
}

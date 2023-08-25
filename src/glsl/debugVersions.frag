int convertPixToInt(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);

  int signBit = 0;//set0 >> 7;

  int num = 0;//((set0 << 16) + (set1 << 8) + set2) - 16777216 * signBit;

  return num;
}

float convertPixToNum(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int signBit = 0;//set0 >> 7;

  int num = 0;//((set0 << 8) + set1) - 65536*signBit;
  float mantissa = 0.0;//(float(set2*256 + set3)/65535.0) * float(-signBit | 1);


  return float(num) + mantissa;
}

float convertPixToHighFloat(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);


  int num = set0;
  float mantissa = 0.0;//float((set1 << 16) + (set2 << 8) + set3)/16777215.0;
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

  ivec2 signBits = ivec2(0, 0);

  int num1 = 0;//(set0*256 + set1) - 65536*signBits.x;
  int num2 = 0;//(set2*256 + set3) - 65536*signBits.y;

  return dInt(num1, num2);
}

iBool convertPixToIBool(vec4 pixel) {
  int set0 = int(pixel.r*255.0);
  int set1 = int(pixel.g*255.0);
  int set2 = int(pixel.b*255.0);
  int set3 = int(pixel.a*255.0);

  int signBit =0;// set0 >> 7;

  int num = 0;//((set0 << 16) + (set1 << 16) + set2) - 16777216*signBit;

  return iBool(set3 > 0, num);
}

//------------------------------------------------------------------------------------
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
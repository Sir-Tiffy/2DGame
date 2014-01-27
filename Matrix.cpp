#include "StdAfx.h"
#include "Matrix.h"
const mat4 mat4::identity(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
/*
mat4::mat4(){}

mat4::mat4(float a,float b,float c,float d,  float e,float f,float g,float h,  float i,float j,float k,float l,  float m,float n,float o,float p){
	data[0] = a;
	data[1] = b;
	data[2] = c;
	data[3] = d;
	data[4] = e;
	data[5] = f;
	data[6] = g;
	data[7] = h;
	data[8] = i;
	data[9] = j;
	data[10] = k;
	data[11] = l;
	data[12] = m;
	data[13] = n;
	data[14] = o;
	data[15] = p;
};

mat4::mat4(float* data){
	for (unsigned char i = 0; i < 4; i++) this->data[i] = data[i];
}

mat4 mat4::operator*(float right) const{
	mat4 result((float*)data);
	for (unsigned char i = 0; i < 16; i++) result.data[i] *= right;
	return result;
}

mat4 mat4::operator*(const mat4& right) const{
	mat4 result(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	for (unsigned char row = 0; row < 4*4; row+=4){
		for (unsigned char column = 0; column < 4; ++column){
			for (unsigned char i = 0; i < 4; ++i){
				result.data[row+column] += data[row+i]*right.data[i*4+column];
			}
		}
	}
	return result;
}

const mat4 mat4::identity(1,0,0,0,
					0,1,0,0,
					0,0,1,0,
					0,0,0,1);



mat3::mat3(){}

mat3::mat3(float a,float b,float c,  float d, float e,float f,  float g,float h,float i){
	data[0] = a;
	data[1] = b;
	data[2] = c;
	data[3] = d;
	data[4] = e;
	data[5] = f;
	data[6] = g;
	data[7] = h;
	data[8] = i;
};


mat3 mat3::operator*(const mat3& right) const{
	mat3 result;
	for (char i = 0; i < 3; ++i)
		for (char j = 0; j < 3; ++j)
			for (char x = 0; x < 3; ++x)
				result.data[i*3+j] = data[i*3+x] * right.data[x*3+j];

	return result;
}

const mat3 mat3::identity(	1,0,0,
							0,1,0,
							0,0,1);*/
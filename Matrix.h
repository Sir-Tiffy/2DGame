#pragma once
#include "Vector.h"

template<typename T, unsigned char MAJOR, unsigned char MINOR> class matN{
public:
	T data[MAJOR*MINOR];
	matN(){};
	matN(T* data){
		for (unsigned int i = 0; i < MAJOR*MINOR; ++i) this->data[i] = data[i];
	}

	const static matN GetIdentity() {
		static_assert(MAJOR==MINOR, "Attempt to find the identity of a non-square matrix!");
		static matN identity;
		for (unsigned char major = 0; major < MAJOR; ++major){
			for (unsigned char minor = 0; minor < MINOR; ++minor){
				identity[major*MINOR+minor] = major==minor;
			}
		}
		return identity;
	}

	matN<T,MINOR,MAJOR> Transpose() const{
		matN<T,MINOR,MAJOR> result;
		for (unsigned char major = 0; major < MAJOR; ++major){
			for (unsigned char minor = 0; minor < MINOR; ++minor){
				result[minor*MAJOR+major] = data[major*MINOR+minor];
			}
		}
		return result;
	}

	matN operator*(const matN& right) const{
		matN result;
		for (unsigned int i = 0; i < MAJOR*MINOR; ++i) result.data[i] = 0;
		for (unsigned char row = 0; row < MAJOR*MINOR; row+=MINOR){
			for (unsigned char column = 0; column < MINOR; ++column){
				for (unsigned char i = 0; i < 4; ++i){
					result.data[row+column] += data[row+i]*right.data[i*4+column];
				}
			}
		}
		return result;
	}

	matN operator*(const T& right) const{
		matN result;
		for (unsigned int i = 0; i < MAJOR*MINOR; ++i) result[i] = data[i]*right;
		return result;
	}
	matN operator+(const matN& right) const{
		matN result;
		for (unsigned int i = 0; i < MAJOR*MINOR; +=i) result[i] = data[i]+right;
		return result;
	}

	bool operator==(const matN& right) const{
		for (unsigned int i = 0; i < MAJOR*MINOR; ++i) if (data[i] != right.data[i]) return false;
		return true;
	}
	
	T operator[](unsigned int id) const{
		return data[id];
	}
	T& operator[](unsigned int id){
		return data[id];
	}
};

class mat4:public matN<float,4,4>{
public:
	mat4(float x0y0,float x1y0,float x2y0,float x3y0,
		 float x0y1,float x1y1,float x2y1,float x3y1,
		 float x0y2,float x1y2,float x2y2,float x3y2,
		 float x0y3,float x1y3,float x2y3,float x3y3){
		data[0] = x0y0;
		data[1] = x1y0;
		data[2] = x2y0;
		data[3] = x3y0;
		
		data[4] = x0y1;
		data[5] = x1y1;
		data[6] = x2y1;
		data[7] = x3y1;

		data[8] = x0y2;
		data[9] = x1y2;
		data[10] = x2y2;
		data[11] = x3y2;

		data[12] = x0y3;
		data[13] = x1y3;
		data[14] = x2y3;
		data[15] = x3y3;
	}
	mat4(const matN<float,4,4>& right){
		for (unsigned char i = 0; i < 16; ++i) data[i] = right.data[i];
	}
	mat4(){}

	const static mat4 identity;
};

//typedef matN<float,4,4> mat4;
typedef matN<float,3,3> mat3;

template<typename T, unsigned int N> Vec::vecN<T,N> operator*(const Vec::vecN<T,N>& left, const matN<T,N,N>& right){
	Vec::vecN<T,N> result;
	for (unsigned int i = 0; i < N; ++i) result[i] = 0;
	for (unsigned int row = 0; row < N; ++row){
		for (unsigned int column = 0; column < N; ++column){
			result[row] += left[column]*right[row*N+column];
		}
	}
	return result;
}
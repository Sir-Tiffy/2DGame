#pragma once
#include "Vector.h"

template<typename T, unsigned int MAJOR, unsigned int MINOR> class mat{
	T data[MAJOR*MINOR];
};


template<typename T> class mat<T,4,4>{
public:
	const static mat<T,4,4> identity;
	union{
		T data[4*4];
		struct{T a00,a01,a02,a03,a10,a11,a12,a13,a20,a21,a22,a23,a30,a31,a32,a33;};
	};
	mat(){};
	mat(const T* data){
		memcpy(data,this->data,16*sizeof(T));
	}
	mat(T b00,T b10,T b20,T b30,T b01,T b11,T b21,T b31,T b02,T b12,T b22,T b32,T b03,T b13,T b23,T b33){
		a00 = b00;
		a01 = b01;
		a02 = b02;
		a03 = b03;
		a10 = b10;
		a11 = b11;
		a12 = b12;
		a13 = b13;
		a20 = b20;
		a21 = b21;
		a22 = b22;
		a23 = b23;
		a30 = b30;
		a31 = b31;
		a32 = b32;
		a33 = b33;
	}

	mat Transpose() const{
		return mat(a00,a01,a02,a03,a10,a11,a12,a13,a20,a21,a22,a23,a30,a31,a32,a33);
	}
	T Determinant() const{
		return a00*(a11*(a22*a33-a23*a32)-a21*(a12*a33-a13*a32)+a31*(a12*a23-a13*a22))-a10*(a01*(a22*a33-a23*a32)-a21*(a02*a33-a03*a32)+a31*(a02*a23-a03*a22))+a20*(a01*(a12*a33-a13*a32)-a11*(a02*a33-a03*a32)+a31*(a02*a13-a03*a12))-a30*(a01*(a12*a23-a13*a22)-a11*(a02*a23-a03*a22)+a21*(a02*a13-a03*a12));
	}
	bool operator==(const mat& right) const{
		return memcmp(data,right.data,4*4*sizeof(T)) == 0;
	}

	mat operator+(const mat& right) const{
		mat result;
		for (unsigned int i = 0; i < 4*4; ++i) result.data[i] = data[i]+right[i];
		return result;
	}
	mat operator*(const T& right) const{
		mat result;
		for (unsigned int i = 0; i < 4*4; ++i) result.data[i] = data[i]*right;
		return result;
	}

	Vec::vecN<T,4> operator*(const Vec::vecN<T,4>& right) const{
		return Vec::vecN<T,4>(
			a00 * right.x + a10 * right.y + a20 * right.z + a30 * right.w,
			a01 * right.x + a11 * right.y + a21 * right.z + a31 * right.w,
			a02 * right.x + a12 * right.y + a22 * right.z + a32 * right.w,
			a03 * right.x + a13 * right.y + a23 * right.z + a33 * right.w
		);
	};

	mat operator*(const mat& right) const{
		mat result;
		for (unsigned int row = 0; row < 4; ++row){
			for (unsigned int column = 0; column < 4; ++column){
				result.data[column*4+row] = 
					data[row+0*4] * right.data[column*4+0] + 
					data[row+1*4] * right.data[column*4+1] + 
					data[row+2*4] * right.data[column*4+2] + 
					data[row+3*4] * right.data[column*4+3];
			}
		}
		return result;
	}
};

typedef mat<float,4,4> mat4;

/*template<typename T, unsigned int N> Vec::vecN<T,N> operator*(const Vec::vecN<T,N>& left, const matN<T,N,N>& right){
	Vec::vecN<T,N> result;
	for (unsigned int i = 0; i < N; ++i) result[i] = 0;
	for (unsigned int row = 0; row < N; ++row){
		for (unsigned int column = 0; column < N; ++column){
			result[row] += left[column]*right[row*N+column];
		}
	}
	return result;
}*/

/*
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
}*/
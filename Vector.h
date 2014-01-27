#pragma once

namespace Vec{

	template<typename T, unsigned int N> class vecData{
	public:
		T data[N];
	};
	
	template<typename T> class vecData<T,2>{
	public:
		union{
			T data[2];
			struct{T x, y;};
		};
	};
	template<typename T> class vecData<T,3>{
	public:
		union{
			T data[3];
			struct{T x, y, z;};
			struct{T r, g, b;};
		};
	};
	template<typename T> class vecData<T,4>{
	public:
		union{
			T data[4];
			struct{T x, y, z, w;};
			struct{T r, g, b, a;};
		};
	};

	template<typename T, unsigned int N> class vecN:public vecData<T,N>{
	public:
		vecN(){}
		vecN(float x, float y){
			static_assert(N>=2,"Cannot construct vector with 2 arguments!");
			data[0] = x;
			data[1] = y;
		}
		vecN(float x, float y, float z){
			static_assert(N>=3,"Cannot construct vector with 3 arguments!");
			data[0] = x;
			data[1] = y;
			data[2] = z;
		}
		vecN(float x, float y, float z, float w){
			static_assert(N>=4, "Cannot consturct vector with 4 arguments!");
			data[0] = x;
			data[1] = y;
			data[2] = z;
			data[3] = w;
		}
		explicit vecN(T* newData){
			for (unsigned int i = 0; i < N; ++i) data[i] = newData[i];
		}
		vecN<T,N> unit() const{
			const T len = length();
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]/len;
			return result;
		}

		vecN cross(const vecN& right){
			static_assert(N==3,"Cannot cross a non-3 vector!");
			return vecN<T,3>(y*right.z - z*right.y, z*right.x - x*right.z, x*right.y - y*right.x);
		}

		T dot (const vecN<T,N>& right) const{
			T result = 0;
			for (unsigned int i = 0; i < N; ++i) result += data[i]*right[i];
			return result;
		}
		T lengthSqr() const{
			T result = 0;
			for (unsigned int i = 0; i < N; ++i) result += data[i]*data[i];
			return result;
		}
		T length() const{
			return sqrt(lengthSqr());
		}
	
		vecN<T,N> operator-() const {
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = -data[i];
			return result;
		}
		vecN<T,N> operator+(const vecN<T,N>& right) const{
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]+right[i];
			return result;
		}
		vecN<T,N> operator-(const vecN<T,N>& right) const{
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]-right[i];
			return result;
		}
		vecN<T,N> operator*(const vecN<T,N>& right) const{
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]*right[i];
			return result;
		}
		vecN<T,N> operator/(const vecN<T,N>& right) const{
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]/right[i];
			return result;
		}

		void operator+=(const vecN<T,N>& right){
			for (unsigned int i = 0; i < N; ++i) data[i] += right[i];
		}
		void operator-=(const vecN<T,N>& right){
			for (unsigned int i = 0; i < N; ++i) data[i] -= right[i];
		}
		void operator*=(const vecN<T,N>& right){
			for (unsigned int i = 0; i < N; ++i) data[i] *= right[i];
		}
		void operator/=(const vecN<T,N>& right){
			for (unsigned int i = 0; i < N; ++i) data[i] /= right[i];
		}

		bool operator==(const vecN<T,N>& right) const{
			bool result = true;
			for (unsigned int i = 0; i < N; ++i) result = result&& data[i]==right[i];
			return result;
		}

		vecN<T,N> operator*(T right) const{
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]*right;
			return result;
		}
		vecN<T,N> operator/(T right) const{
			vecN<T,N> result;
			for (unsigned int i = 0; i < N; ++i) result.data[i] = data[i]/right;
			return result;
		}
		void operator*=(T right){
			for (unsigned int i = 0; i < N; ++i) data[i] *= right;
		}
		void operator/=(T right){
			for (unsigned int i = 0; i < N; ++i) data[i] /= right;
		}

		inline const T operator[](unsigned int i) const{
			return data[i];
		}
		inline T& operator[](unsigned int i){
			return data[i];
		}
	};

	/*template<typename T> class vecN<T,2>:public vecData<T,2>{
	public:
		union{
			T data[2];
			struct{T x, y;};
		};
		vecN(T x, T y):x(x),y(y){}
		vecN(){}
		explicit vecN(T* newData){
			for (unsigned int i = 0; i < 2; ++i) data[i] = newData[i];
		}
		vecN<T,2> unit() const{
			const T len = length();
			return vecN<T,2>(x/len,y/len);
		}
	};

	template<typename T> class vecN<T,3>:public vecData<T,3>{
	public:
		union{
			T data[3];
			struct{T x, y, z;};
			struct{T r, g, b;};
		};
		vecN(T x, T y, T z):x(x),y(y),z(z){}
		vecN(){}
		explicit vecN(T* newData){
			for (unsigned int i = 0; i < 3; ++i) data[i] = newData[i];
		}

		vecN cross(const vecN& right) const{
			return vecN(y*right.z - z*right.y, z*right.x - x*right.z, x*right.y - y*right.x);
		}

		vecN<T,2> xy(){
			return vecN<T,2>(x,y);
		}

		vecN<T,3> unit() const{
			const T len = length();
			return vecN<T,3>(x/len,y/len,z/len);
		}
	};

	template<typename T> class vecN<T,4>:public vecBase<T,4>{
	public:
		union{
			T data[4];
			struct{T x, y, z, w;};
			struct{T r, g, b, a;};
		};
		vecN(T x, T y, T z, T w):x(x),y(y),z(z),w(w){}
		vecN(){}
		explicit vecN(T* newData){
			for (unsigned int i = 0; i < 4; ++i) data[i] = newData[i];
		}

		vecN<T,2> xy() const{
			return vecN<T,2>(x,y);
		}
		vecN<T,3> xyz() const{
			return vecN<T,2>(x,y,z);
		}
		vecN<T,4> unit() const{
			const T len = length();
			return vecN<T,4>(x/len,y/len,z/len,w/len);
		}
	};*/

	typedef vecN<float, 2> vec2;
	typedef vecN<float, 3> vec3;
	typedef vecN<float, 4> vec4;

	
	
	int LuaCreateVec2(lua_State* L, vec2 vec);
	int LuaCreateVec2(lua_State* L, float x, float y);
	int LuaLoadVec2(lua_State* L);
	
	int LuaCreateVec3(lua_State* L, vec3 vec);
	int LuaCreateVec3(lua_State* L, float x, float y, float z);
	int LuaLoadVec3(lua_State* L);
	
	int LuaCreateVec4(lua_State* L, vec4 vec);
	int LuaCreateVec4(lua_State* L, float x, float y, float z, float w);
	int LuaLoadVec4(lua_State* L);
}

/*
class vec2{
public:
	union{
		float data[2];
		struct{float x, y;};
	};
	vec2(float x, float y);
	vec2(float* data);
	vec2();

	float dot(const vec2& right) const;
	float lengthSqr() const;
	float length() const;
	vec2 unit() const;
	
	vec2 operator-() const; //negate
	vec2 operator+(const vec2& right) const;
	vec2 operator-(const vec2& right) const;
	vec2 operator*(const vec2& right) const;
	vec2 operator/(const vec2& right) const;
	
	void operator+=(const vec2& right);
	void operator-=(const vec2& right);
	void operator*=(const vec2& right);
	void operator/=(const vec2& right);

	bool operator==(const vec2& right) const;

	vec2 operator*(float right) const;
	vec2 operator/(float right) const;
};

class vec3{
public:
	union{
		float data[3];
		struct{float x, y, z;};
		struct{float r, g, b;};
	};
	vec3();
	vec3(float x, float y, float z);
	vec3(float* data);

	float dot(const vec3& right) const;
	float lengthSqr() const;
	float length() const;
	vec3 unit() const;

	vec3 cross(const vec3& right) const;
	
	vec3 operator-() const; //negate
	vec3 operator+(const vec3& right) const;
	vec3 operator-(const vec3& right) const;
	vec3 operator*(const vec3& right) const;
	vec3 operator/(const vec3& right) const;

	void operator+=(const vec3& right);
	void operator-=(const vec3& right);
	void operator*=(const vec3& right);
	void operator/=(const vec3& right);

	bool operator==(const vec3& right) const;

	vec3 operator*(float right) const;
	vec3 operator/(float right) const;
};*/

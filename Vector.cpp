#include "StdAfx.h"
#include "Vector.h"

using namespace std;

namespace Vec{
	int LuaCreateVec2(lua_State* L, float x, float y){
		new(lua_newuserdata(L,sizeof(vec2)))(vec2)(x,y);
		luaL_setmetatable(L,"Vector2");
		return 1;
	}
	int LuaCreateVec2(lua_State* L, vec2 vec){
		new(lua_newuserdata(L,sizeof(vec2)))(vec2)(vec);
		luaL_setmetatable(L,"Vector2");
		return 1;
	}
	static int CreateVec2(lua_State* L){
		return LuaCreateVec2(L,(float)luaL_optnumber(L,1,0),(float)luaL_optnumber(L,2,0));
	}
	static int Vec2Dot(lua_State* L);
	static int Vec2_Index(lua_State* L){
		vec2* vec = (vec2*)luaL_checkudata(L,1,"Vector2");
		string str = luaL_checkstring(L,2);
		if (str == "x") lua_pushnumber(L,vec->x);
		else if (str == "y") lua_pushnumber(L,vec->y);
		else if (str == "unit") LuaCreateVec2(L,vec->unit());
		else if (str == "length") lua_pushnumber(L, vec->length());
		else if (str == "lengthSqr") lua_pushnumber(L, vec->lengthSqr());
		else if (str == "dot") lua_pushcfunction(L, Vec2Dot);
		else return luaL_error(L, "%s is not a valid member of vec2",str);
		return 1;
	}
	static int Vec2_NewIndex(lua_State* L){
		vec2* vec = (vec2*)luaL_checkudata(L,1,"Vector2");
		string str = luaL_checkstring(L,2);
		if (str == "x") vec->x = (float)luaL_checknumber(L,3);
		else if (str == "y") vec->y = (float)luaL_checknumber(L,3);
		else return luaL_error(L, "%s is not a valid member of vec2",str);
		return 0;
	}
	static int Vec2_ToString(lua_State* L){
		vec2* vec = (vec2*)luaL_checkudata(L,1,"Vector2");
		lua_pushfstring(L,"(%f, %f)",vec->x,vec->y);
		return 1;
	}
	static int Vec2Dot(lua_State* L){
		vec2* left = (vec2*)luaL_checkudata(L, 1, "Vector2");
		vec2* right = (vec2*)luaL_checkudata(L, 2, "Vector2");
		lua_pushnumber(L, left->dot(*right));
		return 1;
	}
	static int Vec2_Add(lua_State* L){
		vec2* left = (vec2*)luaL_checkudata(L,1,"Vector2");
		vec2* right = (vec2*)luaL_checkudata(L,2,"Vector2");
		return LuaCreateVec2(L,*left+*right);
	}
	static int Vec2_Sub(lua_State* L){
		vec2* left = (vec2*)luaL_checkudata(L,1,"Vector2");
		vec2* right = (vec2*)luaL_checkudata(L,2,"Vector2");
		return LuaCreateVec2(L,*left-*right);
	}
	static int Vec2_unm(lua_State* L){
		vec2* vec = (vec2*)luaL_checkudata(L,1,"Vector2");
		return LuaCreateVec2(L,-(*vec));
	}
	static int Vec2_eq(lua_State* L){
		vec2* left = (vec2*)luaL_checkudata(L,1,"Vector2");
		vec2* right = (vec2*)luaL_checkudata(L,2,"Vector2");
		lua_pushboolean(L, (*left)==(*right));
		return 1;
	}
	static int Vec2_Mul(lua_State* L){
		vec2* left = (vec2*)luaL_checkudata(L, 1, "Vector2");
		if (lua_isnumber(L, 2)){
			return LuaCreateVec2(L,(*left)*(float)lua_tonumber(L,2));
		}
		vec2* right = (vec2*)luaL_testudata(L, 2, "Vector2");
		if (right == NULL){
			return luaL_argerror(L, 2, "Vector2 or Number expected");
		}
		return LuaCreateVec2(L, (*left)*(*right));
	}
	static int Vec2_Div(lua_State* L){
		vec2* left = (vec2*)luaL_checkudata(L, 1, "Vector2");
		if (lua_isnumber(L, 2)){
			return LuaCreateVec2(L,(*left)/(float)lua_tonumber(L,2));
		}
		vec2* right = (vec2*)luaL_testudata(L, 2, "Vector2");
		if (right == NULL){
			return luaL_argerror(L, 2, "Vector2 or Number expected");
		}
		return LuaCreateVec2(L, (*left)/(*right));
	}


	int LuaLoadVec2(lua_State* L){
		static const luaL_Reg lib[] = {
			{"new",CreateVec2},
			{NULL,NULL}
		};
		static const luaL_Reg metatable[] = {
			{"__index",Vec2_Index},
			{"__newindex",Vec2_NewIndex},
			{"__tostring",Vec2_ToString},
			{"__add", Vec2_Add},
			{"__sub", Vec2_Sub},
			{"__mul", Vec2_Mul},
			{"__div", Vec2_Div},
			{"__unm", Vec2_unm},
			{"__eq", Vec2_eq},
			{NULL,NULL}
		};

		luaL_newlib(L,lib);

		luaL_newmetatable(L,"Vector2");
		luaL_setfuncs(L,metatable,0);
		lua_pop(L,1);
		return 1;
	}



	int LuaCreateVec3(lua_State* L, float x, float y, float z){
		new(lua_newuserdata(L,sizeof(vec3)))(vec3)(x,y,z);
		luaL_setmetatable(L,"Vector3");
		return 1;
	}
	int LuaCreateVec3(lua_State* L, vec3 vec){
		new(lua_newuserdata(L,sizeof(vec3)))(vec3)(vec);
		luaL_setmetatable(L,"Vector3");
		return 1;
	}
	static int CreateVec3(lua_State* L){
		return LuaCreateVec3(L,(float)luaL_optnumber(L,1,0),(float)luaL_optnumber(L,2,0),(float)luaL_optnumber(L,3,0));
	}
	static int Vec3Dot(lua_State* L);
	static int Vec3Cross(lua_State* L);
	static int Vec3_Index(lua_State* L){
		vec3* vec = (vec3*)luaL_checkudata(L,1,"Vector3");
		string str = luaL_checkstring(L,2);
		if (str == "x") lua_pushnumber(L,vec->x);
		else if (str == "y") lua_pushnumber(L,vec->y);
		else if (str == "z") lua_pushnumber(L, vec->z);
		else if (str == "unit") LuaCreateVec3(L,vec->unit());
		else if (str == "length") lua_pushnumber(L, vec->length());
		else if (str == "lengthSqr") lua_pushnumber(L, vec->lengthSqr());
		else if (str == "dot") lua_pushcfunction(L, Vec3Dot);
		else if (str == "cross") lua_pushcfunction(L, Vec3Cross);
		else return luaL_error(L, "%s is not a valid member of vec3",str);
		return 1;
	}
	static int Vec3_NewIndex(lua_State* L){
		vec3* vec = (vec3*)luaL_checkudata(L,1,"Vector3");
		string str = luaL_checkstring(L,2);
		if (str == "x") vec->x = (float)luaL_checknumber(L,3);
		else if (str == "y") vec->y = (float)luaL_checknumber(L,3);
		else if (str == "z") vec->z = (float)luaL_checknumber(L,3);
		else return luaL_error(L, "%s is not a valid member of vec3",str);
		return 0;
	}
	static int Vec3_ToString(lua_State* L){
		vec3* vec = (vec3*)luaL_checkudata(L,1,"Vector3");
		lua_pushfstring(L,"(%f, %f, %f)",vec->x,vec->y,vec->z);
		return 1;
	}

	static int Vec3Cross(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L, 1, "Vector2");
		vec3* right = (vec3*)luaL_checkudata(L, 2, "Vector2");
		return LuaCreateVec3(L, left->cross(*right));
	}
	static int Vec3Dot(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L, 1, "Vector2");
		vec3* right = (vec3*)luaL_checkudata(L, 2, "Vector2");
		lua_pushnumber(L, left->dot(*right));
		return 1;
	}
	static int Vec3_Add(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L,1,"Vector2");
		vec3* right = (vec3*)luaL_checkudata(L,2,"Vector2");
		return LuaCreateVec3(L,*left+*right);
	}
	static int Vec3_Sub(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L,1,"Vector2");
		vec3* right = (vec3*)luaL_checkudata(L,2,"Vector2");
		return LuaCreateVec3(L,*left-*right);
	}
	static int Vec3_unm(lua_State* L){
		vec3* vec = (vec3*)luaL_checkudata(L,1,"Vector2");
		return LuaCreateVec3(L,-(*vec));
	}
	static int Vec3_eq(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L,1,"Vector2");
		vec3* right = (vec3*)luaL_checkudata(L,2,"Vector2");
		lua_pushboolean(L, (*left)==(*right));
		return 1;
	}
	static int Vec3_Mul(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L, 1, "Vector2");
		if (lua_isnumber(L, 2)){
			return LuaCreateVec3(L,(*left)*(float)lua_tonumber(L,2));
		}
		vec3* right = (vec3*)luaL_testudata(L, 2, "Vector2");
		if (right == NULL){
			return luaL_argerror(L, 2, "Vector2 or Number expected");
		}
		return LuaCreateVec3(L, (*left)*(*right));
	}
	static int Vec3_Div(lua_State* L){
		vec3* left = (vec3*)luaL_checkudata(L, 1, "Vector2");
		if (lua_isnumber(L, 2)){
			return LuaCreateVec3(L,(*left)/(float)lua_tonumber(L,2));
		}
		vec3* right = (vec3*)luaL_testudata(L, 2, "Vector2");
		if (right == NULL){
			return luaL_argerror(L, 2, "Vector2 or Number expected");
		}
		return LuaCreateVec3(L, (*left)/(*right));
	}


	int LuaLoadVec3(lua_State* L){
		static const luaL_Reg lib[] = {
			{"new",CreateVec3},
			{NULL,NULL}
		};
		static const luaL_Reg metatable[] = {
			{"__index",Vec3_Index},
			{"__newindex",Vec3_NewIndex},
			{"__tostring",Vec3_ToString},
			{"__add", Vec3_Add},
			{"__sub", Vec3_Sub},
			{"__mul", Vec3_Mul},
			{"__div", Vec3_Div},
			{"__unm", Vec3_unm},
			{"__eq", Vec3_eq},
			{NULL,NULL}
		};

		luaL_newlib(L,lib);

		luaL_newmetatable(L,"Vector3");
		luaL_setfuncs(L,metatable,0);
		lua_pop(L,1);
		return 1;
	}



	
	int LuaCreateVec4(lua_State* L, float x, float y, float z, float w){
		new(lua_newuserdata(L,sizeof(vec4)))(vec4)(x,y,z,w);
		luaL_setmetatable(L,"Vector4");
		return 1;
	}
	int LuaCreateVec4(lua_State* L, vec4 vec){
		new(lua_newuserdata(L,sizeof(vec4)))(vec4)(vec);
		luaL_setmetatable(L,"Vector4");
		return 1;
	}
	static int CreateVec4(lua_State* L){
		return LuaCreateVec4(L,
			luaL_checknumber(L,1),
			luaL_checknumber(L,2),
			luaL_checknumber(L,3),
			luaL_checknumber(L,4));
	}
	static int Vec4_Index(lua_State* L){
		vec4* vec = (vec4*)luaL_checkudata(L,1,"Vector4");
		string str = luaL_checkstring(L,2);
		if (str == "x" || str == "r") lua_pushnumber(L,vec->x);
		else if (str == "y" || str == "g") lua_pushnumber(L,vec->y);
		else if (str == "z" || str == "b") lua_pushnumber(L, vec->z);
		else if (str == "w" || str == "a") lua_pushnumber(L, vec->w);
		else return luaL_error(L, "%s is not a valid member of vec4",str);
		return 1;
	}
	static int Vec4_NewIndex(lua_State* L){
		vec4* vec = (vec4*)luaL_checkudata(L,1,"Vector4");
		string str = luaL_checkstring(L,2);
		if (str == "x" || str == "r") vec->x = (float)luaL_checknumber(L,3);
		else if (str == "y" || str == "g") vec->y = (float)luaL_checknumber(L,3);
		else if (str == "z" || str == "b") vec->z = (float)luaL_checknumber(L,3);
		else if (str == "w" || str == "a") vec->w = (float)luaL_checknumber(L,3);
		else return luaL_error(L, "%s is not a valid member of vec4",str);
		return 0;
	}
	static int Vec4_ToString(lua_State* L){
		vec4* vec = (vec4*)luaL_checkudata(L,1,"Vector4");
		lua_pushfstring(L,"(%f, %f, %f, %f)",vec->x,vec->y, vec->z, vec->w);
		return 1;
	}


	int LuaLoadVec4(lua_State* L){
		static const luaL_Reg lib[] = {
			{"new",CreateVec4},
			{NULL,NULL}
		};
		static const luaL_Reg metatable[] = {
			{"__index",Vec4_Index},
			{"__newindex",Vec4_NewIndex},
			{"__tostring",Vec4_ToString},
			{NULL,NULL}
		};

		luaL_newlib(L,lib);

		luaL_newmetatable(L,"Vector4");
		luaL_setfuncs(L,metatable,0);
		lua_pop(L,1);
		return 1;
	}


}
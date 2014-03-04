#include "StdAfx.h"
#include "GameObject.h"
#include "Engine.h"

using namespace std;
using namespace Vec;

	int RedirectedPrint(lua_State* L);
namespace GameObject{

	Sprite::Sprite()
		:UV(0,0,1,1),
		colour(1,1,1,1),
		position(0,0),
		size(1,1),
		layer(0),
		visible(true),
		texture(nullptr),
		textureHandle(LUA_NOREF),
		modified(true)
	{}

	unsigned int Sprite::GetSortKey(){
		if (modified){
			//sortKey = (((texture!=nullptr)?(texture->textureID<<8):0))|layer;
			sortKey = (layer<<8)|((texture!=nullptr)?texture->textureID:0);
			modified = false;
		}
		return sortKey;
	}

	static int CreateSpriteUserdata(lua_State* L){
		Sprite* newSprite = new (lua_newuserdata(L,sizeof(Sprite)))(Sprite)();
		luaL_setmetatable(L,"Sprite");
		/*lua_pushlightuserdata(L,(void*)&Engine::instance->LuaGameObjectsKey);
		lua_gettable(L,LUA_REGISTRYINDEX);
		lua_pushnumber(L,lua_rawlen(L,-1));
		lua_pushvalue(L,-3);
		lua_settable(L,-3);
		lua_pop(L,1);*/
		lua_pushvalue(L,1);
		newSprite->lua_handle = luaL_ref(L,LUA_REGISTRYINDEX);
		Engine::instance->AddGameObject(newSprite);
		return 1;
	}

	static int DestroySprite(lua_State* L){
		Sprite* sprite = (Sprite*)luaL_checkudata(L,1,"Sprite");
		/*lua_pushnil(L);
		while(lua_next(L,LUA_REGISTRYINDEX)!=0){
			if (lua_rawequal(L,1,-1)){
				lua_pushvalue(L,-2);
				lua_pushnil(L);
				lua_settable(L,LUA_REGISTRYINDEX);
				break;
			}
			lua_pop(L,1);
		}*/
		if (sprite->lua_handle){
			luaL_unref(L,LUA_REGISTRYINDEX, sprite->lua_handle);
			sprite->lua_handle = LUA_NOREF;
		}
		if (sprite->GetTexture() != nullptr){
			luaL_unref(L,LUA_REGISTRYINDEX, sprite->GetTextureHandle());
			sprite->SetTexture(nullptr,LUA_NOREF);
		}
		Engine::instance->RemoveGameObject(sprite);
		return 0;
	}
	
	const static char* const SPRITEINDEX[] = {
		"Position",
		"Size",
		"Visible",
		"Colour",
		"Layer",
		"UV",
		"Texture",
		"Destroy",
		nullptr
	};	
	const static char* const SPRITENEWINDEX[] = {
		"Position",
		"Size",
		"Visible",
		"Colour",
		"Layer",
		"UV",
		"Texture",
		//"Destroy",
		nullptr
	};
	enum {
		SPRITE_POSITION,
		SPRITE_SIZE,
		SPRITE_VISIBLE,
		SPRITE_COLOUR,
		SPRITE_LAYER,
		SPRITE_UV,
		SPRITE_TEXTURE,
		SPRITE_DESTROY
	};

	static int Sprite_Index(lua_State* L){
		Sprite* sprite = (Sprite*)luaL_checkudata(L,1,"Sprite");
		switch(luaL_checkoption(L,2,nullptr,SPRITEINDEX)){
			case SPRITE_POSITION: return LuaCreateVec2(L,sprite->position);
			case SPRITE_SIZE: return LuaCreateVec2(L,sprite->size);
			case SPRITE_VISIBLE: lua_pushboolean(L,sprite->visible); return 1;
			case SPRITE_COLOUR: return LuaCreateVec4(L, sprite->colour);
			case SPRITE_LAYER: lua_pushnumber(L,sprite->GetLayer()); return 1;
			case SPRITE_UV: return LuaCreateVec4(L, sprite->UV);
			case SPRITE_TEXTURE:
				if (sprite->GetTexture() == nullptr) lua_pushnil(L);
				else lua_rawgeti(L,LUA_REGISTRYINDEX, sprite->GetTextureHandle());
				return 1;
			case SPRITE_DESTROY: lua_pushcfunction(L,DestroySprite); return 1;
		}
		return luaL_error(L,"Failed to handle member '%s' of Sprite!",lua_tostring(L,2));
	}
	static int Sprite_NewIndex(lua_State* L){
		Sprite* sprite = (Sprite*)luaL_checkudata(L,1,"Sprite");
		switch(luaL_checkoption(L,2,nullptr,SPRITENEWINDEX)){
			case SPRITE_POSITION: sprite->position = *(vec2*)luaL_checkudata(L,3,"Vector2"); return 0;
			case SPRITE_SIZE: sprite->size = *(vec2*)luaL_checkudata(L,3,"Vector2"); return 0;
			case SPRITE_VISIBLE: sprite->visible = lua_toboolean(L,3)!=0; return 0;
			case SPRITE_COLOUR:{
				if (luaL_testudata(L,3,"Vector3")) sprite->colour = vec4(*(vec3*)lua_touserdata(L,3),1);
				else if (luaL_testudata(L,3,"Vector4")) sprite->colour = *(vec4*)lua_touserdata(L,3);
				else return luaL_argerror(L,3,"Vector3 or Vector4 expected");
				return 0;
			} case SPRITE_LAYER: {
				const double newLayer = luaL_checknumber(L,3);
				if ((unsigned char)newLayer != newLayer) return luaL_argerror(L,3,"Sprite layer must be an integer from 0-255");
				sprite->SetLayer((unsigned char)newLayer);
				return 0;
			} case SPRITE_UV: sprite->UV = *(vec4*)luaL_checkudata(L,3,"Vector4"); return 0;
			case SPRITE_TEXTURE:
				if (lua_isnoneornil(L,3)){
					luaL_unref(L,LUA_REGISTRYINDEX, sprite->GetTextureHandle());
					sprite->SetTexture(nullptr, LUA_NOREF);
				} else {
					
					Texture::Texture* tex = (Texture::Texture*)luaL_checkudata(L,3,"Texture");
					sprite->SetTexture(tex, luaL_ref(L,LUA_REGISTRYINDEX));
				}
				return 0;
			default: return luaL_error(L,"Failed to handle member '%s' of Sprite!",lua_tostring(L,2));
		}
		return 0;
	}
	static int Sprite_GC(lua_State* L){
		Sprite* s = (Sprite*)lua_touserdata(L,1);
		DestroySprite(L);
		s->~Sprite();
		return 0;
	}
	static int Sprite_ToString(lua_State* L){
		lua_pushstring(L,"Sprite");
		return 1;
	}
	static int Sprite_IsSprite(lua_State* L){
		lua_pushboolean(L,luaL_testudata(L,1,"Sprite")!=NULL);
		return 1;
	}

	int LuaLoadSprite(lua_State* L){
		static const luaL_Reg lib[] = {
			{"new",CreateSpriteUserdata},
			{"IsSprite",Sprite_IsSprite},
			{NULL, NULL}
		};

		static const luaL_Reg metatable[] = {
			{"__index",Sprite_Index},
			{"__newindex",Sprite_NewIndex},
			{"__gc",Sprite_GC},
			{"__tostring",Sprite_ToString},
			{NULL,NULL}
		};

		luaL_newlib(L,lib);

		luaL_newmetatable(L,"Sprite");
		luaL_setfuncs(L,metatable,0);
		lua_pop(L,1);

		return 1;
	}

	Camera::Camera():position(.5,.5),scale(1),rotation(0){}

	static int Camera_Index(lua_State* L){ //TODO: lua_option
		Camera* cam = (Camera*)luaL_checkudata(L,1,"Camera");
		string index = luaL_checkstring(L,2);
		if (index == "Position")
			return Vec::LuaCreateVec2(L,cam->position);
		else if (index == "Scale")
			lua_pushnumber(L,cam->scale);
			//return Vec::LuaCreateVec2(L,cam->size);
		else if (index == "Rotation")
			lua_pushnumber(L,cam->rotation);
		else return luaL_error(L,"%s is not a valid member of Camera!",index.c_str());
		return 1;
	}

	static int Camera_NewIndex(lua_State* L){ //TODO: lua_option
		Camera* cam = (Camera*)luaL_checkudata(L,1,"Camera");
		string index = luaL_checkstring(L,2);
		if (index == "Position"){
			cam->position = *((vec2*)luaL_checkudata(L,3,"Vector2"));
			Engine::instance->RecalculateCamera(cam);
		} else if (index == "Scale"){
			//cam->size = *(vec2*)luaL_checkudata(L,3,"Vector2");
			cam->scale = (float)luaL_checknumber(L,3);
			Engine::instance->RecalculateCamera(cam);
		} else if (index == "Rotation"){
			cam->rotation = (float)luaL_checknumber(L,3);
			Engine::instance->RecalculateCamera(cam);
		} else return luaL_error(L,"%s is not a valid member of Camera!",index.c_str());
		return 0;
	}

	static int Camera_GC(lua_State* L){
		Camera* cam = (Camera*)lua_touserdata(L,1);
		cam->~Camera();
		return 0;
	}

	static int Camera_ToString(lua_State* L){
		lua_pushstring(L,"Camera");
		return 0;
	}

	void LuaLoadCamera(lua_State* L){
		static const luaL_Reg cameraMetatable[] = {
			{"__index",Camera_Index},
			{"__newindex",Camera_NewIndex},
			{"__gc",Camera_GC},
			{"__tostring",Camera_ToString},
			{NULL, NULL}
		};
		luaL_newmetatable(L,"Camera");
		luaL_setfuncs(L,cameraMetatable,0);
		lua_pop(L,1);

		Engine::instance->RecalculateCamera(new(lua_newuserdata(L,sizeof(Camera)))(Camera)());
		luaL_setmetatable(L,"Camera");

		lua_setglobal(L,"Camera");
	}
}
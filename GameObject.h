#pragma once
#include "Vector.h"
#include "Texture.h"

namespace GameObject{

	/*struct Object{
		vec2 position;
		vec2 size;
		vec3 colour;
		vec4 UV;
		float rotation;
		unsigned char layer;
		bool visible:1;
		bool anchored:1;
	};*/

	class Sprite{
	private:
		Texture::Texture* texture;
	public:
		Vec::vec4 UV;
		Vec::vec4 colour;
		Vec::vec2 position;
		Vec::vec2 size;
		int lua_handle;
	private:
		unsigned int sortKey;
		int textureHandle;
		unsigned char layer;
		bool modified:1;
	public:
		bool visible:1;
		inline Texture::Texture* GetTexture(){return texture;}
		inline void SetTexture(Texture::Texture* newTexture, int newHandle){
			texture = newTexture;
			textureHandle = newHandle;
			modified = true;
		}
		inline int GetTextureHandle(){return textureHandle;}
		inline unsigned char GetLayer(){return layer;}
		inline void SetLayer(int newLayer){
			layer = newLayer;
			modified = true;
		}

		unsigned int GetSortKey();

		Sprite();
	};

	class Camera{
	public:
		Vec::vec2 position;
		float scale;
		float rotation;
		Camera();
	};

	int LuaLoadSprite(lua_State* L);
	void LuaLoadCamera(lua_State* L);
};
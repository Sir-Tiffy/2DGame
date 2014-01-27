#pragma once
#include "StdAfx.h"

namespace Texture{
	class Texture{
		friend int Texture::CreateTexture(lua_State* L);
	private:
		Texture();
	public:
		~Texture();
		std::string filename;
		int registryHandle;
		int width, height;
		unsigned int textureHandle;
		unsigned short id;
		short bpp;
	};

	int LuaLoadTexture(lua_State* L);
}
#pragma once
#include "StdAfx.h"

namespace Texture{
	/*class Texture{
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
	};*/

	struct Texture{
		std::string filename;

		enum bitmapFormats {
			RGB=GL_RGB,
			RGBA=GL_RGBA,
			BGR=GL_BGR_EXT,
			BGRA=GL_BGRA_EXT,
		} bitmapFormat;

		int registryHandle;
		unsigned int width,height;
		unsigned int textureHandle;
		unsigned short textureID;
		short bpp;
	};

	int LuaLoadTexture(lua_State* L);
}
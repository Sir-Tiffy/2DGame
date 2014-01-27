#include "StdAfx.h"
#include "Texture.h"
#include "Renderer.h"

using namespace std;

string SimplifyFilename(const char* filename, const size_t filenameLength){
	string result;
	result.reserve(filenameLength);
	char last = 0;
	for (unsigned int i = 0; i < filenameLength; ++i){
		char c = filename[i];
		if (c == '/') c = '\\';
		if (c == '\\' && last == '\\') continue;
		result += last = c;
	}
	result.shrink_to_fit();
	return result;
}

string SimplifyFilename(string filename){
	return SimplifyFilename(filename.data(), filename.length());
};



namespace Texture{
	unordered_map<string,Texture*> textures;
	vector<unsigned short> ids(1);

	static unsigned short GetUniqueID(){
		unsigned short i;
		for (i = 1; i < ids.size(); ++i){
			if (ids[i] == 0) return ids[i] = i;
		}
		ids.emplace_back(i);
		return i;
	}

	static void ClearID(unsigned short id){
		ids[id] = 0;
	}

	Texture::Texture():
		textureHandle(0),
		id(GetUniqueID())
	{}

	Texture::~Texture(){
		ClearID(id);
	}

	int LoadTexture(lua_State* L, Texture* tex, string filename){
		tex->filename = filename;

		ifstream file(filename, ios::in|ios::binary);
		if (!file.good()) return luaL_error(L, "Failed to load file '%s': File not found!",filename.data());
		char id[3];
		file.read(id,2);
		if (id[0] == 'B' && id[1] == 'M'){ //bmp
			unsigned int dataStart;
			file.ignore(8);
			file.read((char*)&dataStart,4);
			unsigned int headerSize;
			file.read((char*)&headerSize,4);
			if (headerSize != 40){
				file.close();
				return luaL_error(L, "Failed to load file '%s': Unsupported header version!",filename);
			}
			int height, width;
			short bpp;
			file.read((char*)&width,4);
			file.read((char*)&height,4);
			width = abs(width);
			height = abs(height);
			file.ignore(2);
			file.read((char*)&bpp,2);

			if (bpp != 24){
				file.close();
				return luaL_error(L, "Failed to load file %s: Unsupported bits per pixel!",filename);
			}

			tex->height = height;
			tex->width = width;
			tex->bpp = bpp;
			unsigned int compressionMethod;
			file.read((char*)&compressionMethod,4);
			if (compressionMethod != 0){
				file.close();
				return luaL_error(L,"Failed to load file '%s': Unsupported compression method!",filename);
			}

			file.seekg(dataStart,ios_base::beg);
			if (!file.good()){
				file.close();
				return luaL_error(L,"Failed to load file '%s': Invalid file structure!",filename);
			}

			vector<char> data(width*height*bpp/8);

			/*unsigned int num = 0;
			for (int y = 0; y <height; ++y){
				for (int x = 0; x < width; ++x){
					for (short c = bpp/8-1; c >=0; --c){
						data[(y*width+x)*bpp/8+c] = file.get();
					}
				}
			}*/
			
			file.read(data.data(), width*height*bpp/8);

			file.close();
			Renderer::UploadTexture(L, tex,data,width,height,bpp);

		} else {
			file.read(id+2,1);
			return luaL_error(L, "Failed to load file '%s': Unsupported file format!",filename.data());
		}

		textures[filename] = tex;
		return 0;
	}

	int CreateTexture(lua_State* L){
		size_t filenameLength;
		const char* filename = luaL_checklstring(L,1,&filenameLength);
		string textureFilename = SimplifyFilename(filename, filenameLength);

		if (textures.count(textureFilename)==1){ //texture exists
			lua_getfield(L,LUA_REGISTRYINDEX, "weak");
			lua_rawgeti(L,-1, textures[textureFilename]->registryHandle);
		} else { //texture doesn't exist
			Texture* tex = new (lua_newuserdata(L,sizeof(Texture)))(Texture)();
			luaL_setmetatable(L,"Texture");
			lua_getfield(L,LUA_REGISTRYINDEX, "weak");
			lua_pushvalue(L,-2);
			tex->registryHandle = luaL_ref(L,-2);
			lua_pop(L,1);

			LoadTexture(L, tex, textureFilename);
		}
		return 1;
	}

	int Texture_GC(lua_State* L){
		Texture* tex = (Texture*)lua_touserdata(L,1);
		lua_getfield(L,LUA_REGISTRYINDEX, "weak");
		luaL_unref(L,-1, tex->registryHandle);
		textures.erase(tex->filename);
		Renderer::DeleteTexture(tex);
		tex->~Texture();
		return 0;
	}

	int Texture_Tostring(lua_State* L){
		Texture* tex = (Texture*)luaL_checkudata(L,1,"Texture");
		lua_pushstring(L,tex->filename.data());
		return 1;
	}

	int LuaLoadTexture(lua_State* L){
		const static luaL_Reg metatable[] = {
			{"__gc",Texture_GC},
			{"__tostring",Texture_Tostring},
			{NULL,NULL}
		};
		luaL_newmetatable(L,"Texture");
		luaL_setfuncs(L,metatable,0);
		lua_pop(L,1);


		lua_createtable(L,0,1);
		lua_pushstring(L,"new");
		lua_pushcfunction(L,CreateTexture);
		lua_rawset(L,-3);
		return 1;
	}
}
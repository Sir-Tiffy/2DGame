#include "StdAfx.h"
#include "Texture.h"
#include "Renderer.h"
#include "Vector.h"

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

inline string SimplifyFilename(string filename){
	return SimplifyFilename(filename.data(), filename.length());
};

unsigned int ReadBigEndian(ifstream& file, unsigned char bytes){
	unsigned int result = 0;
	for (int i = (bytes-1)*8; i >= 0; i -= 8){
		result |= file.get()<<i;
	}
	return result;
}

inline unsigned int ReadLittleEndian(ifstream& file, char count){
	unsigned int result = 0;
	file.read((char*)&result,count);
	return result;
}

namespace Texture{
	static unordered_map<string,Texture*> textures;
	static vector<unsigned short> ids(1);

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
	/*
	Texture::Texture():
		textureHandle(0),
		id(GetUniqueID())
	{}

	Texture::~Texture(){
		ClearID(id);
	}*/


	inline bool IsPowerOf2(unsigned int num){
		return num!=0 && !(num&(num-1));
	}

	static bool LoadBMP(lua_State* L, Texture* tex, ifstream& file, vector<char>& data){
		tex->bitmapFormat = Texture::bitmapFormats::BGR;
		//file header
		const unsigned int fileSize = ReadLittleEndian(file,4);
		file.ignore(4);
		const unsigned int dataOffset = ReadLittleEndian(file,4);
		//dib header

		enum dibHeaders{
			BITMAPCOREHEADER = 12,
			OS21XBITMAPHEADER = 12,
			BITMAPCOREHEADER2 = 64,
			OS22XBITMAPHEADER = 64,
			BITMAPINFOHEADER = 40,
			BITMAPV2INFOHEADER = 52,
			BITMAPV3INFOHEADER = 56,
			BITMAPV4HEADER = 108,
			BITMAPV5HEADER = 124
		};

		const unsigned int dibHeaderSize = ReadLittleEndian(file,4);
		
		unsigned int compressionMethod = 0;
		unsigned int imageSize = 0;
		unsigned int coloursInPalette;
		unsigned short numColourPlanes;
		bool flip;

		switch (dibHeaderSize){
			case BITMAPCOREHEADER:{
				tex->width = ReadLittleEndian(file,2);
				tex->height = ReadLittleEndian(file,2);
				flip = true;
				numColourPlanes = ReadLittleEndian(file,2);
				tex->bpp = ReadLittleEndian(file,2);
				break;
			} case BITMAPINFOHEADER: {
				tex->width = abs((signed int)ReadLittleEndian(file,4));
				const signed int height = ReadLittleEndian(file,4);
				flip = height > 0;
				tex->height = abs(height);

				numColourPlanes = ReadLittleEndian(file,2);
				tex->bpp = ReadLittleEndian(file,2);
				compressionMethod = ReadLittleEndian(file,4);
				imageSize = ReadLittleEndian(file,4);
				file.ignore(8);
				coloursInPalette = ReadLittleEndian(file,4);
				file.ignore(4);
				break;
			} default:
				lua_pushfstring(L,"Unsupported bmp DIB header (%d)!",dibHeaderSize);
				return false;
		}

		enum compressionMethods{
			Bl_RGB, Bl_RLE8, Bl_RLE4, Bl_BITFIELDS, Bl_JPEG, Bl_PNG, Bl_ALPHABITFIELDS
		};
		if (numColourPlanes != 1){
			lua_pushfstring(L,"Unsupported bmp colour planes (%d)!",numColourPlanes);
			return false;
		}
		if (compressionMethod != Bl_RGB){
			lua_pushfstring(L,"Unsupported bmp compression method (%d)!",compressionMethod);
			return false;
		}
		if (!IsPowerOf2(tex->width)){
			lua_pushfstring(L,"Texture width (%d) must be a power of 2!",tex->width);
			return false;
		} else if (!IsPowerOf2(tex->height)){
			lua_pushfstring(L,"Texture height (%d) must be a power of 2!",tex->height);
			return false;
		}

		file.seekg(dataOffset,ios_base::beg);
		if (!file.good()){
			lua_pushfstring(L,"Error reading file!");
			return false;
		}

		data.resize(tex->width*tex->height*tex->bpp/8);
		const unsigned char padding = (4-((tex->width*tex->bpp/8)%4))%4;
		if (flip){
			for (unsigned int y = 0; y < tex->height; ++y){
				file.read(data.data()+y*tex->width*tex->bpp/8,tex->width*tex->bpp/8);
				file.ignore(padding);
			}
		} else {
			for (int y = tex->height-1; y >= 0; --y){
				file.read(data.data()+y*tex->width*tex->bpp/8,tex->width*tex->bpp/8);
				file.ignore(padding);
			}
		}

		return true;
	}
	static bool LoadPNG(lua_State* L, Texture* tex, ifstream& file, vector<char>& data){
		enum chunkTypes{
			IHDR = 0x52444849,//0x49484452,
			PLTE = 0x45544c50,//0x504c5445,
			IDAT = 0x54414449,//0x49444154,
			IEND = 0x444e4549,//0x49454e44,
			//tRNS = 0x534e5274 //0x74524e53
		};
		enum colourType{
			GREYSCALE = 0,
			TRUECOLOUR = 2,
			INDEXEDCOLOUR = 3,
			GREYSCALE_ALPHA = 4,
			TRUECOLOUR_ALPHA = 6
		};
		struct pixel{
			unsigned char red, green, blue;
		};

		vector<pixel> palette;
		vector<char> compressedData;

		unsigned char colourType, compressionMethod, filterMethod, interlaceMethod;
		
		unsigned int chunkLength;
		unsigned int chunkType;
		do {
			chunkLength = ReadBigEndian(file,4);
			file.read((char*)&chunkType,4);
				char temp[5];
				memcpy(temp,&chunkType,4);
				temp[4] = 0;
			switch(chunkType){
				case IHDR:
					tex->width = ReadBigEndian(file,4);
					tex->height = ReadBigEndian(file,4);
					tex->bpp = file.get();
					colourType = file.get();
					compressionMethod = file.get();
					filterMethod = file.get();
					interlaceMethod = file.get();

					if (!IsPowerOf2(tex->width)){
						lua_pushfstring(L,"Texture width (%d) must be a power of 2!",tex->width);
						return false;
					}
					if (!IsPowerOf2(tex->height)){
						lua_pushfstring(L,"Texture width (%d) must be a power of 2!",tex->height);
						return false;
					}
					switch(colourType){
						case TRUECOLOUR:
							if (tex->bpp != 8){
								lua_pushfstring(L,"PNG bit depth (%d) not supported, only 24 or 32!",tex->bpp*3);
								return false;
							}
							tex->bpp = 24;
							tex->bitmapFormat = Texture::RGB;
							break;
						case TRUECOLOUR_ALPHA:
							if (tex->bpp != 8){
								lua_pushfstring(L,"PNG bit depth (%d) not supported, only 24 or 32!",tex->bpp*4);
							}
							tex->bpp = 32;
							tex->bitmapFormat = Texture::RGBA;
							break;
						case GREYSCALE: case GREYSCALE_ALPHA:
							lua_pushstring(L,"Greyscale PNGs are not supported!");
							return false;
						case INDEXEDCOLOUR:
							lua_pushstring(L,"Indexed PNGs are not supported!");
							return false;
						default:
							lua_pushfstring(L,"Unknown PNG colour type (%d)!",colourType);
							return false;
					}
					if (compressionMethod != 0){
						lua_pushfstring(L,"Unknown PNG compression method (%d)!",compressionMethod);
						return false;
					}
					if (filterMethod != 0){
						lua_pushfstring(L,"Unknown PNG filter method (%d)!", filterMethod);
						return false;
					}
					if (interlaceMethod != 0){
						lua_pushstring(L,"PNG interlace is not supported!");
						return false;
					}

					break;
				case PLTE:
					palette.resize(chunkLength/3);
					file.read((char*)palette.data(), chunkLength);
					break;
				case IDAT:{
					const auto start = compressedData.size();
					compressedData.resize(start+chunkLength);
					file.read(compressedData.data()+start,chunkLength);
					break;
				} case IEND:
					break;
				default:
					if (!(chunkType&0x20)){
						char temp[5];
						memcpy(temp,&chunkType,4);
						temp[4] = 0;
						lua_pushfstring(L,"Unknown critical PNG chunk '%s'!",temp);
						return false;
					}
					file.ignore(chunkLength);
			}
			file.ignore(4);
		} while (chunkType != IEND);
		file.close();

		const unsigned int scanlineWidth = tex->width*tex->bpp/8 + 1;

		
		unsigned long destSize = scanlineWidth*tex->height;
		vector<char> decompressedData(destSize);
		const int result = uncompress((unsigned char*)decompressedData.data(), &destSize, (unsigned char*)compressedData.data(), compressedData.size());
		compressedData.clear();

		switch(result){
			case Z_BUF_ERROR:
				lua_pushfstring(L,"PNG uncompressed data larger than expected (%d expected, got %d)!",data.size(), destSize);
				return false;
			case Z_MEM_ERROR:
				data.clear();
				lua_pushstring(L,"Not enough memory to decompress PNG data!");
				return false;
			case Z_DATA_ERROR:
				lua_pushstring(L,"PNG Data is corrupted!");
				return false;
			case Z_OK: break;
			default:
				lua_pushstring(L,"An unknown error occured during decompression!");
				return false;
		}

		data.resize(tex->width*tex->height*tex->bpp/8);
		unsigned int count = 0;
		for (int scanline = tex->height-1; scanline >= 0; --scanline){
			const unsigned int index = scanline*scanlineWidth;
			const unsigned char encoding = decompressedData[index];
			if (encoding != 0){
				lua_pushfstring(L,"PNG scanline encoding (%d) not supported!",encoding);
				return false;
			}
			memcpy(data.data()+(count),decompressedData.data()+(index+1),scanlineWidth-1);
			count += scanlineWidth-1;
			/*for (unsigned int i = 0; i < scanlineWidth-1; ++i){
				data[count++] = decompressedData[index+i+1];
			}*/
		}

		return true;
	}

	static bool LoadTexture(lua_State* L, Texture* tex, vector<char>& data){
		ifstream file(tex->filename,ios::in|ios::binary);
		if (!file.good()){
			lua_pushstring(L, "File not found!");
			return false;
		}

		char header[8];
		file.read((char*)&header,8);
		
		if (strncmp(header,"BM",2)==0){
			file.seekg(2,ios_base::beg);
			return LoadBMP(L, tex,file,data);
		}
		if ((header[0]&0x80) && strncmp(header+1,"PNG\x0D\x0A\x1A\x0A",7) == 0) return LoadPNG(L, tex,file,data);

		lua_pushstring(L, "Unknown file format!");
		return false;
	}

	static int CreateTexture(lua_State* L){
		size_t filenameLength;
		const char* filename = luaL_checklstring(L,1,&filenameLength);
		string textureFilename = SimplifyFilename(filename, filenameLength);
		if (textureFilename.find(".\\") != string::npos){
			textureFilename.~string();
			return luaL_error(L, "Failed to load texture '%s': Invalid filename!",textureFilename.c_str());
		}

		if (textures.count(textureFilename)){ //texture exists
			lua_getfield(L,LUA_REGISTRYINDEX, "weak");
			lua_rawgeti(L,-1, textures[textureFilename]->registryHandle);
			return 1;
		}

		Texture* tex = new (lua_newuserdata(L,sizeof(Texture)))(Texture)();
		vector<char> data;
		tex->filename = textureFilename;
		tex->registryHandle = LUA_NOREF;
		tex->textureHandle = tex->textureID = 0;

		if (!LoadTexture(L,tex,data)){
			lua_pushfstring(L,"Failed to load texture '%s': ",textureFilename.c_str());
			lua_pushvalue(L,-2);
			lua_concat(L,2);
			textureFilename.~string(); //failed to load texture
			data.~vector();
			return lua_error(L);
		}
		Renderer::UploadTexture(L,tex,data);

		tex->textureID = GetUniqueID();
		textures[filename] = tex;

		luaL_setmetatable(L,"Texture");
		lua_getfield(L,LUA_REGISTRYINDEX, "weak");
		lua_pushvalue(L,-2);
		tex->registryHandle = luaL_ref(L,-2);
		lua_pop(L,1);
		return 1;
	}

	static int Texture_GC(lua_State* L){
		Texture* tex = (Texture*)lua_touserdata(L,1);
		lua_getfield(L,LUA_REGISTRYINDEX, "weak");
		luaL_unref(L,-1, tex->registryHandle);
		textures.erase(tex->filename);
		Renderer::DeleteTexture(tex);
		ClearID(tex->textureID);
		tex->~Texture();
		return 0;
	}

	static int Texture_Tostring(lua_State* L){
		Texture* tex = (Texture*)luaL_checkudata(L,1,"Texture");
		lua_pushstring(L,tex->filename.data());
		return 1;
	}

	static int Texture_SetFilter(lua_State* L){
		Texture* tex = (Texture*)luaL_checkudata(L,1,"Texture");
		const static char* const filters[] = {
			"Linear",
			"Nearest",
			NULL
		};
		switch(luaL_checkoption(L,2,0,filters)){
			case 0: Renderer::SetTextureFilter(tex,Renderer::TextureFilter::LINEAR); break;
			case 1: Renderer::SetTextureFilter(tex,Renderer::TextureFilter::NEAREST); break;
		}
		return 0;
	}

	static int Texture_Index(lua_State* L){
		Texture* tex = (Texture*)luaL_checkudata(L,1,"Texture");
		const static char* const textureIndicies[] = {
			"width","height","size","SetFilter",0
		};
		enum {
			WIDTH, HEIGHT, SIZE, SETFILTER
		};
		switch(luaL_checkoption(L,2,NULL,textureIndicies)){
			case WIDTH:
				lua_pushnumber(L,tex->width);
				return 1;
			case HEIGHT:
				lua_pushnumber(L,tex->height);
				return 1;
			case SIZE:
				return Vec::LuaCreateVec2(L,(float)tex->width,(float)tex->height);
			case SETFILTER:
				lua_pushcfunction(L,Texture_SetFilter);
				return 1;
		}
		return luaL_error(L,"Failed to handle index '%s' of Texture!",lua_tostring(L,2));
	}

	int LuaLoadTexture(lua_State* L){
		const static luaL_Reg metatable[] = {
			{"__gc",Texture_GC},
			{"__tostring",Texture_Tostring},
			{"__index", Texture_Index},
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
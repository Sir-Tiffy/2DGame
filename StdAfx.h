#pragma once
#pragma warning(disable : 4996) 

#include <vector>
#include <string>
#include <exception>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <memory>
#include <queue>
#include <random>

#include <stdlib.h>

#include "lua/lua.hpp"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <windowsx.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "zlib/zlib.h"

typedef HWND WindowHandle;

const static float PI = 3.14159265359f;
const static float TAU = 6.28318530718f;

#define STR_HELP(x) #x
#define STR(x) STR_HELP(x)
#define ERROR_INFO "\nFile: "__FILE__ "\nLine: " STR(__LINE__)
/*#define ERROR(str) throw runtime_error(str ERROR_INFO)
#define ASSERT(condition, str) if (!(condition)) ERROR(str)*/

#include "Vector.h"
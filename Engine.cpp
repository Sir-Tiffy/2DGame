#include "StdAfx.h"
#include "Engine.h"
#include "Texture.h"

using namespace std;

const static char WaitKey;

void Engine::UpdatePhysics(){
}

void Engine::UpdateLogic(){
	RunScripts();
}

/*void Engine::SetFullscreen(bool setting){
	renderer.CloseWindow();
	fullscreen = setting;
	currentWindow = renderer.OpenWindow(width, height, windowTitle, fullscreen, resizable);
}

void Engine::ToggleFullscreen(){
	return SetFullscreen(!fullscreen);
}*/

double Engine::CalculateTime(){
	LARGE_INTEGER largeInteger;
	QueryPerformanceFrequency(&largeInteger);
	const double freq = (double)largeInteger.QuadPart;
	QueryPerformanceCounter(&largeInteger);
	return largeInteger.QuadPart/freq;
}

double Engine::GetTime(){
	return currentTime;
}

/*void Engine::BeginScripts(){
	lua_State* thread = lua_newthread(L);
	int code = luaL_loadfile(thread,"scripts/main.lua");
	if (code != LUA_OK){
		MessageBox(NULL, lua_tostring(L,-1), "Script error!",MB_DEFAULT_DESKTOP_ONLY);
		return;
	}
	code = lua_resume(thread,NULL,0);
	switch(code){
		case LUA_YIELD:{
			if (lua_islightuserdata(thread,-2) && lua_touserdata(thread,-2)==(void*)&WaitKey){
				waitingLuaThreads.emplace_back(thread,currentTime+lua_tonumber(thread,-1));
				lua_pop(thread,2);
			} else {
				MessageBox(NULL,"Attempt to yield from outside a coroutine!","Script error!",MB_DEFAULT_DESKTOP_ONLY);
			}
			break;
		}
		case LUA_OK:
			break;
		case LUA_ERRRUN:{
			MessageBox(NULL,lua_tostring(thread,-1),"Script error!",MB_DEFAULT_DESKTOP_ONLY);
			break;
		}
		default:
			break;
	}
}*/

void Engine::StartScript(lua_State* thread, unsigned int args){
	if (!lua_isthread(L,-1)) throw logic_error("Error starting script!\nTop of global stack is not a thread!");
	const int code = lua_resume(thread,NULL,args);
	switch(code){
		case LUA_YIELD:{
			if (lua_islightuserdata(thread,-2) && lua_touserdata(thread,-2)==(void*)&WaitKey){
				waitingLuaThreads.emplace_back(thread,currentTime+lua_tonumber(thread,-1),luaL_ref(L,LUA_REGISTRYINDEX));
				lua_pop(thread,2);
				return;
			}
			MessageBox(NULL,"Attempt to yield from outside a coroutine!","Script error!",MB_DEFAULT_DESKTOP_ONLY);
			break;
		}
		case LUA_ERRRUN:{
			MessageBox(NULL,lua_tostring(thread,-1),"Script error!",MB_DEFAULT_DESKTOP_ONLY);
			break;
		}
	}
	lua_pop(L,1);
}

void Engine::RunScripts(){
	for (ThreadObject& threadObject:waitingLuaThreads){
		if (threadObject.waitTime > currentTime) continue;
		lua_pushnumber(threadObject.thread,deltaTime);
		lua_pushnumber(threadObject.thread,currentTime);
		lua_checkstack(threadObject.thread,2); //CHECK
		const int msg = lua_resume(threadObject.thread,NULL,2);
		if (msg == LUA_YIELD){
			if (lua_islightuserdata(threadObject.thread,-2) && lua_touserdata(threadObject.thread,-2)==(void*)&WaitKey){
				threadObject.waitTime = currentTime+lua_tonumber(threadObject.thread,-1);
				lua_pop(threadObject.thread,2);
			} else {
				threadObject.thread = nullptr;
				luaL_unref(L,LUA_REGISTRYINDEX, threadObject.threadHandle);
				MessageBox(NULL,"Attempt to yield from outside a coroutine!","Script error!",MB_DEFAULT_DESKTOP_ONLY);
			}
		} else {
			if (msg != LUA_OK) MessageBox(NULL,lua_tostring(threadObject.thread,-1),"Script error!",MB_DEFAULT_DESKTOP_ONLY);
			luaL_unref(L,LUA_REGISTRYINDEX, threadObject.threadHandle);
			threadObject.thread = nullptr;
		}
	}
	waitingLuaThreads.erase(std::remove_if(waitingLuaThreads.begin(),waitingLuaThreads.end(),[](ThreadObject thread){return thread.thread==nullptr;}),waitingLuaThreads.end());
	lua_gc(L,LUA_GCSTEP,1);

	/*waitingLuaThreads.erase(std::remove_if(
			waitingLuaThreads.begin(),
			waitingLuaThreads.end(),
			[](lua_State* thread){
				const int msg = lua_resume(thread,NULL,0);
				if (msg == LUA_YIELD){
					return false;
				}
				else if (msg != LUA_OK) MessageBox(NULL, lua_tostring(thread,-1),"Script error!",MB_DEFAULT_DESKTOP_ONLY);
				return true;
			}
		),waitingLuaThreads.end());*/
	/*for (lua_State* thread:waitingLuaThreads){
		lua_resume(thread,NULL,0);
	}*/
}

vector<string> GetAllFilesInDir(const char* dir, const char* ext){
	vector<string> result;
	WIN32_FIND_DATA data;
	char path[MAX_PATH];
	PathCombine(path, dir, ext);
	HANDLE find = FindFirstFile(path, &data);
	if (find != INVALID_HANDLE_VALUE){
		do {
			PathCombine(path,dir,data.cFileName);
			result.emplace_back(path);
		} while (FindNextFile(find, &data) != 0);
		FindClose(find);
	}
	PathCombine(path, dir, "*");
	find = FindFirstFile(path, &data);
	if (find != INVALID_HANDLE_VALUE){
		do {
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && data.cFileName[0] != '.'){
				PathCombine(path,dir,data.cFileName);
				vector<string> dirResult = GetAllFilesInDir(path, ext);
				result.insert(result.end(), dirResult.begin(), dirResult.end());
			}
		} while (FindNextFile(find, &data) != 0);
		FindClose(find);
	}
	return result;
}

void Engine::Render(){
	return renderer.Render(objects);
}

void Engine::BeginLoop(){
	double lastTime = currentTime = CalculateTime();
	//if (luaL_dofile(L,"main.lua") != LUA_OK)
	//	auto a = lua_tostring(L,1);
	//BeginScripts();

	lua_getglobal(L,"math");
	lua_getfield(L,-1,"randomseed");
	lua_pushnumber(L,GetTime());
	lua_call(L,1,0);
	lua_pop(L,1);

	//for (string& script:GetAllFilesInDir("scripts\\","*.lua")){
		lua_State* thread = lua_newthread(L);
		if (luaL_loadfile(thread,"scripts/main.lua"/*script.c_str()*/) == LUA_OK)
			StartScript(thread, 0);
		else
			MessageBox(NULL, lua_tostring(thread,-1), "Script error!", MB_DEFAULT_DESKTOP_ONLY);
	//}

	while (running){
		input.GetInput(currentWindow);
		currentTime = CalculateTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		UpdatePhysics();
		UpdateLogic();
		Render();
		//renderer.Render(objects);
	}
	renderer.CloseWindow();
}

void Engine::OnWindowResize(int w, int h){
	width = w;
	height = h;
	renderer.OnWindowResize(w,h);
}

void Engine::ReadConfig(){
	width = 20*30*2;//800;
	height = 20*16*2;//600;
	fullscreen = false;
	vsync = true;
	resizable = true;
}

/*static int RedirectedPrint(lua_State* L){
	const unsigned int numElements = lua_gettop(L);
	stringstream s;
	for (unsigned int i = 1; i <= numElements; ++i){
		//Engine::instance->outputLog<<lua_tostring(L, i);
		//if (i != numElements-1) Engine::instance->outputLog<<' ';
		auto a = lua_type(L,i);
		const char* str = lua_tostring(L, i);
		if (str != nullptr) s<<str;
		else s<<"NULL";

		if (i != numElements) s<<' ';
	}
	//Engine::instance->outputLog<<endl;
	MessageBox(NULL,s.str().c_str(),"Print",MB_DEFAULT_DESKTOP_ONLY);
	return 0;
}*/

static int RedirectedPrint (lua_State *L) {
	stringstream stream;
	int n = lua_gettop(L);	/* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);	/* function to be called */
		lua_pushvalue(L, i);	 /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);	/* get result */
		if (s == NULL){
			stream.~stream();
			return luaL_error(L,
				 LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		}
		//if (i>1) luai_writestring("\t", 1);
		if (i > 1) stream<<' ';
		//luai_writestring(s, l);
		stream<<s;
		lua_pop(L, 1);	/* pop result */
	}
	//luai_writeline();
	stream<<'\n';
	//MessageBox(NULL,stream.str().c_str(),"Print",MB_DEFAULT_DESKTOP_ONLY);
	Engine::instance->SetWindowTitle(stream.str());
	Engine::instance->outputLog<<stream;
	return 0;
}

void Engine::SetWindowTitle(string title){
	SetWindowText(currentWindow,title.c_str());
}

static int Wait(lua_State* L){
	auto a = lua_type(L,1);
	const double num = luaL_optnumber(L,1,0);
	lua_pushlightuserdata(L,(void*)&WaitKey);
	lua_pushnumber(L,num);
	return lua_yield(L,2);
}

static int Tick(lua_State*L){
	lua_pushnumber(L,Engine::instance->GetTime());
	return 1;
}

void Engine::AddGameObject(GameObject::Sprite* sprite){
	objects.push_back(sprite);
}

void Engine::RemoveGameObject(GameObject::Sprite* sprite){
	for (auto i = objects.begin(); i != objects.end(); ++i){
		if ((*i)==sprite){
			*i = std::move(objects.back());
			return objects.pop_back();
		}
	}
}

void Engine::RecalculateCamera(GameObject::Camera* camera){
	renderer.RecalculateCamera(camera->position,camera->scale, camera->rotation);
}

void Engine::ReceiveEvent(const Event* event){
	switch(event->type){
		case EVENT_WINDOW_CLOSE: running = false; return;
	}
	BroadcastEvent(event);
}

static int ScreenCoordsToWorld(lua_State* L){
	const Vec::vec2 result = Engine::instance->renderer.ScreenToWorld((float)luaL_checknumber(L,1),(float)luaL_checknumber(L,2));
	lua_pushnumber(L,result.x);
	lua_pushnumber(L,result.y);
	return 2;
}

int LuaScreen_Index(lua_State* L){
	const static char* const members[] = {
		"Width","Height","Fullscreen","Resizable","ToWorldCoords",
		NULL
	};
	enum {
	LUASCREEN_WIDTH,LUASCREEN_HEIGHT,LUASCREEN_FULLSCREEN,LUASCREEN_RESIZABLE,LUASCREEN_TO_WORLD_COORDS
	};	
	switch(luaL_checkoption(L,2,NULL, members)){
		case LUASCREEN_WIDTH:
			lua_pushnumber(L,Engine::instance->width);
			return 1;
		case LUASCREEN_HEIGHT:
			lua_pushnumber(L,Engine::instance->height);
			return 1;
		case LUASCREEN_FULLSCREEN:
			lua_pushboolean(L,Engine::instance->fullscreen);
			return 1;
		case LUASCREEN_RESIZABLE:
			lua_pushboolean(L,Engine::instance->resizable);
			return 1;
		case LUASCREEN_TO_WORLD_COORDS:
			lua_pushcfunction(L,ScreenCoordsToWorld);
			return 1;
	}
	return lua_error(L);
}

void Engine::SetWidth(int newWidth){
	renderer.ResizeWindow(width = newWidth,height);
}

void Engine::SetHeight(int newHeight){
	renderer.ResizeWindow(width,height = newHeight);
}

void Engine::SetResizable(bool newResizable){
	renderer.SetResizable(resizable=newResizable);
}

int LuaScreen_NewIndex(lua_State* L){
	const static char* const members[] = {
		"Width","Height","Fullscreen","Resizable",
		NULL
	};
	enum {
	LUASCREEN_WIDTH,LUASCREEN_HEIGHT,LUASCREEN_FULLSCREEN,LUASCREEN_RESIZABLE
	};	
	switch(luaL_checkoption(L,2,NULL,members)){
		case LUASCREEN_WIDTH:
			Engine::instance->SetWidth(luaL_checkint(L,3));
			return 0;
		case LUASCREEN_HEIGHT:
			Engine::instance->SetHeight(luaL_checkint(L,3));
			return 0;
		case LUASCREEN_FULLSCREEN:
			//lua_pushboolean(L,Engine::instance->fullscreen);
			//return 1;
			return luaL_error(L,"Setting window fullscreen is not yet supported!");
		case LUASCREEN_RESIZABLE:
			Engine::instance->SetResizable(lua_toboolean(L,3)!=0);
			return 0;
	}
	return lua_error(L);
	/*const static char* indicies[] = {
		"Fullscreen",
		NULL
	}; 
	enum {
		LUASCREEN_FULLSCREEN
	};	
	switch(luaL_checkoption(L,2,NULL, indicies)){
		//case LUASCREEN_WIDTH:
		//	Engine::instance->width = luaL_checkint(L,2);
		//	return 0;
		//case LUASCREEN_HEIGHT:
		//	Engine::instance->height = luaL_checkint(L,2);
		//	return 0;
		case LUASCREEN_FULLSCREEN:
			if (!lua_isboolean(L,3)) return luaL_error(L,"bad argument to Fullscreen: bool expected, got (%s)",lua_typename(L,lua_type(L,2)));
			Engine::instance->SetFullscreen(lua_toboolean(L,2)!=0);
			return 0;
	}
	return lua_error(L);*/
}

void Engine::RegisterLuaScreen(lua_State* L){
	const static luaL_Reg lib[] = {
		{"__index",LuaScreen_Index},
		{"__newindex",LuaScreen_NewIndex},
		{NULL, NULL}
	};
	lua_newtable(L);
	lua_createtable(L,0,2);
	luaL_setfuncs(L,lib,0);
	lua_setmetatable(L,-2);
	lua_setglobal(L,"Screen");
}

Engine::Engine(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
		:hInstance(hInstance),
		nCmdShow(nCmdShow),
		windowTitle("SUPER COOL 2D GAME"),
		currentWindow(nullptr),
		fullscreen(false){
	if (instance != nullptr) throw logic_error("Engine is already running!");
	instance = this;
	L = luaL_newstate();
	luaL_openlibs(L);

	outputLog.open("lua_output.log",ios::out);
	//lua_pushcfunction(L, RedirectedPrint);
	//lua_setglobal(L, "print");
	lua_register(L,"print",RedirectedPrint);
	lua_register(L,"wait",Wait);
	lua_register(L,"tick",Tick);
	
	lua_newtable(L); //weak table
	lua_newtable(L); //weak table metatable
	lua_pushstring(L,"v");
	lua_setfield(L,-2,"__mode");
	lua_setmetatable(L,-2);
	lua_setfield(L,LUA_REGISTRYINDEX, "weak");
	
	luaL_requiref(L, "Sprite",GameObject::LuaLoadSprite,true);
	luaL_requiref(L, "Vector2",Vec::LuaLoadVec2,true);
	luaL_requiref(L, "Vector3",Vec::LuaLoadVec3,true);
	luaL_requiref(L, "Vector4",Vec::LuaLoadVec4,true);
	luaL_requiref(L, "Texture",Texture::LuaLoadTexture,true);

	lua_pop(L,5);
	GameObject::LuaLoadCamera(L);
	RegisterLuaScreen(L);
	input.RegisterLuaEventHandler(L);
	
	ReadConfig();

	renderer.Init(hInstance, nCmdShow, input.GetWndProc());

	
	currentWindow = renderer.OpenWindow(width,height, windowTitle, fullscreen, resizable);
	vsync = renderer.SetVsync(vsync);

	input.RegisterObserver(this,EVENT_ALL);

	running = true;
}

Engine::~Engine(){
	lua_close(L);
	instance = nullptr;
}

Engine* Engine::instance = nullptr;
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

void Engine::ToggleFullscreen(){
	renderer.CloseWindow();
	currentWindow = renderer.OpenWindow(width, height, windowTitle, fullscreen = !fullscreen);
}

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
		lua_pushnumber(threadObject.thread,currentTime);
		lua_pushnumber(threadObject.thread,deltaTime);
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
			threadObject.thread = nullptr;
			luaL_unref(L,LUA_REGISTRYINDEX, threadObject.threadHandle);
			if (msg != LUA_OK) MessageBox(NULL,lua_tostring(threadObject.thread,-1),"Script error!",MB_DEFAULT_DESKTOP_ONLY);
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

void Engine::BeginLoop(){
	double lastTime = currentTime = CalculateTime();
	//if (luaL_dofile(L,"main.lua") != LUA_OK)
	//	auto a = lua_tostring(L,1);
	//BeginScripts();
	{
		lua_State* thread = lua_newthread(L);
		luaL_loadfile(thread,"scripts/main.lua");
		StartScript(thread, 0);
	}

	while (running){
		input.GetInput(currentWindow);
		currentTime = CalculateTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		UpdatePhysics();
		UpdateLogic();
		renderer.Render(objects);
	}
	renderer.CloseWindow();
}

void Engine::OnWindowResize(int w, int h){
	width = w;
	height = h;
	renderer.OnWindowResize(w,h);
}

void Engine::ReadConfig(){
	width = 800;
	height = 600;
	fullscreen = false;
	vsync = true;
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
		if (s == NULL)
			return luaL_error(L,
				 LUA_QL("tostring") " must return a string to " LUA_QL("print"));
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

int test(lua_State*L){
	lua_getfield(L,LUA_REGISTRYINDEX, "weak");
	return 1;
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

	lua_register(L,"test",test);
	
	luaL_requiref(L, "Sprite",GameObject::LuaLoadSprite,true);
	luaL_requiref(L, "Vector2",Vec::LuaLoadVec2,true);
	luaL_requiref(L, "Vector3",Vec::LuaLoadVec3,true);
	luaL_requiref(L, "Vector4",Vec::LuaLoadVec4,true);
	luaL_requiref(L, "Texture",Texture::LuaLoadTexture,true);
	lua_pop(L,5);
	GameObject::LuaLoadCamera(L);
	input.RegisterLuaEventHandler(L);
	
	ReadConfig();

	renderer.Init(hInstance, nCmdShow, input.GetWndProc());

	
	currentWindow = renderer.OpenWindow(width,height, windowTitle, fullscreen);
	vsync = renderer.SetVsync(vsync);

	input.RegisterObserver(this,EVENT_ALL);

	running = true;
}

Engine::~Engine(){
	lua_close(L);
	instance = nullptr;
}

Engine* Engine::instance = nullptr;
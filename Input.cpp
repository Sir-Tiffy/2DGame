#include "StdAfx.h"
#include "Input.h"
#include "Engine.h"

using namespace std;

static Input* instance;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_SIZE:
			Engine::instance->renderer.OnWindowResize(LOWORD(lParam),HIWORD(lParam));
			break;
		case WM_KEYDOWN: case WM_SYSKEYDOWN:
			if (!(lParam&0x40000000)) instance->QueueEvent(unique_ptr<Event>(new KeyEvent(instance, EVENT_KEY_DOWN, wParam)));
			break;
		case WM_KEYUP: case WM_SYSKEYUP:
			instance->QueueEvent(unique_ptr<Event>(new KeyEvent(instance, EVENT_KEY_UP, wParam)));
			break;
		case WM_MOUSEMOVE:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_MOVE, LOWORD(lParam), HIWORD(lParam), wParam)));
			break;
		/*case WM_INPUT:{
			static RAWINPUT input;
			unsigned int inputSize = sizeof(input);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &input, (unsigned int*)&inputSize, sizeof(RAWINPUTHEADER));
			if (input.header.dwType == RIM_TYPEMOUSE){
				instance->ReceiveEvent(&MouseEvent(instance, EVENT_MOUSE_MOVE_HARDWARE, input.data.mouse.lLastX, input.data.mouse.lLastY, input.data.mouse.usButtonData));
				break;
			} else return DefWindowProc(hWnd,msg,wParam,lParam);
		}*/
		case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_BUTTON_DOWN, LOWORD(lParam), HIWORD(lParam), wParam)));
			break;
		case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: case WM_XBUTTONUP:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_BUTTON_UP, LOWORD(lParam), HIWORD(lParam), wParam)));
			break;
		case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK: case WM_MBUTTONDBLCLK: case WM_XBUTTONDBLCLK:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_BUTTON_DOUBLECLICK, LOWORD(lParam), HIWORD(lParam), wParam)));
			break;
		case WM_MOUSEWHEEL:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_WHEEL, LOWORD(lParam), HIWORD(lParam), wParam)));
			break;
		case WM_COPY:
			instance->QueueEvent(unique_ptr<Event>(new Event(instance, EVENT_WINDOW_COPY)));
			break;
		case WM_CUT:
			instance->QueueEvent(unique_ptr<Event>(new Event(instance, EVENT_WINDOW_CUT)));
			break;
		case WM_PASTE:
			instance->QueueEvent(unique_ptr<Event>(new Event(instance, EVENT_WINDOW_PASTE)));
			break;
		case WM_UNDO:
			instance->QueueEvent(unique_ptr<Event>(new Event(instance, EVENT_WINDOW_UNDO)));
			break;
		case WM_CLOSE:
			instance->QueueEvent(unique_ptr<Event>(new Event(instance,EVENT_WINDOW_CLOSE)));
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void Input::QueueEvent(unique_ptr<Event> event){
	eventQueue.push(std::move(event));
}

void Input::GetInput(WindowHandle currentWindow){
	MSG msg;
	while (PeekMessage(&msg, currentWindow, 0, 0, PM_REMOVE)){
		DispatchMessage(&msg);
	}
	while (!eventQueue.empty()){
		ReceiveEvent(eventQueue.front().get());
		eventQueue.pop();
	}
}

WNDPROC Input::GetWndProc(){
	return WndProc;
}

static int RegisterKeyDown(lua_State* L){
	{
		const int top = lua_gettop(L);
		if (top > 1) lua_pop(L,top-1);
	}
	if (lua_isnil(L,1)){
		if (instance->keyDownFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->keyDownFunc);
			instance->keyDownFunc = 0;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->keyDownFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}
static int RegisterKeyUp(lua_State* L){
	{
		const int top = lua_gettop(L);
		if (top > 1) lua_pop(L,top-1);
	}
	if (lua_isnil(L,1)){
		if (instance->keyUpFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->keyUpFunc);
			instance->keyUpFunc = 0;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->keyUpFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}

void Input::ReceiveEvent(const Event* event){
	switch(event->type){
		case EVENT_KEY_DOWN:
			if (keyDownFunc != 0){
				lua_checkstack(L,1);//CHECK
				lua_State* thread = lua_newthread(L);
				lua_checkstack(thread,2);//CHECK
				lua_rawgeti(thread,LUA_REGISTRYINDEX,keyDownFunc);
				lua_pushlstring(thread,(const char*)&((KeyEvent*)event)->key,1);

				Engine::instance->StartScript(thread,1);
			}
			break;
		case EVENT_KEY_UP:
			if (keyUpFunc != 0){
				lua_checkstack(L,1);//CHECK
				lua_State* thread = lua_newthread(L);
				lua_checkstack(thread,2);//CHECK
				lua_rawgeti(thread,LUA_REGISTRYINDEX,keyUpFunc);
				lua_pushlstring(thread,(const char*)&((KeyEvent*)event)->key,1);
				Engine::instance->StartScript(thread,1);

			}
			break;
	}
	BroadcastEvent(event);
}

static int LoadEventLib(lua_State* L){
	const static luaL_Reg inputLib[] = {
		{"KeyDown",RegisterKeyDown},
		{"KeyUp",RegisterKeyUp},
		{NULL, NULL}
	};
	luaL_newlib(L,inputLib);
	return 1;
}

void Input::RegisterLuaEventHandler(lua_State* L){
	this->L = L;
	luaL_requiref(L,"Input",LoadEventLib,true);
	lua_pushlightuserdata(L,this);
	lua_newtable(L);
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_pop(L,1);
}

Input::Input():
	L(nullptr),
	keyDownFunc(0),
	keyUpFunc(0)
{
	instance = this;
}

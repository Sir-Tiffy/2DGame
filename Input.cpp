#include "StdAfx.h"
#include "Input.h"
#include "Engine.h"

using namespace std;

static Input* instance;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_SIZE:
			Engine::instance->OnWindowResize(LOWORD(lParam),HIWORD(lParam));
			break;
		case WM_KEYDOWN: case WM_SYSKEYDOWN:
			if (!(lParam&0x40000000)) instance->QueueEvent(unique_ptr<Event>(new KeyEvent(instance, EVENT_KEY_DOWN, wParam)));
			break;
		case WM_KEYUP: case WM_SYSKEYUP:
			instance->QueueEvent(unique_ptr<Event>(new KeyEvent(instance, EVENT_KEY_UP, wParam)));
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
		case WM_MOUSEMOVE:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_MOVE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),LOWORD(wParam)^instance->prevMouseFlags)));
			instance->prevMouseFlags = LOWORD(wParam);
			break;
		case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN: case WM_XBUTTONDOWN:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_BUTTON_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), LOWORD(wParam)^instance->prevMouseFlags)));
			instance->prevMouseFlags = LOWORD(wParam);
			break;
		case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP: case WM_XBUTTONUP:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_BUTTON_UP, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), LOWORD(wParam)^instance->prevMouseFlags)));
			instance->prevMouseFlags = LOWORD(wParam);
			break;
		case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK: case WM_MBUTTONDBLCLK: case WM_XBUTTONDBLCLK:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_BUTTON_DOUBLECLICK, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), LOWORD(wParam)^instance->prevMouseFlags)));
			instance->prevMouseFlags = LOWORD(wParam);
			break;
		case WM_MOUSEWHEEL:
			instance->QueueEvent(unique_ptr<Event>(new MouseEvent(instance, EVENT_MOUSE_WHEEL, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_WHEEL_DELTA_WPARAM(wParam))));
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
	return eventQueue.emplace_back(std::move(event));
}

void Input::GetInput(WindowHandle currentWindow){
	MSG msg;
	while (PeekMessage(&msg, currentWindow, 0, 0, PM_REMOVE)){
		DispatchMessage(&msg);
	}
	for (auto& e:eventQueue){
		ReceiveEvent(e.get());
	}
	eventQueue.clear();
}

WNDPROC Input::GetWndProc(){
	return WndProc;
}

static int RegisterKeyDown(lua_State* L){
	lua_settop(L,1);
	if (lua_isnil(L,1)){
		if (instance->keyDownFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->keyDownFunc);
			instance->keyDownFunc = LUA_NOREF;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->keyDownFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}
static int RegisterKeyUp(lua_State* L){
	lua_settop(L,1);
	if (lua_isnil(L,1)){
		if (instance->keyUpFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->keyUpFunc);
			instance->keyUpFunc = LUA_NOREF;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->keyUpFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}
static int RegisterMouseMove(lua_State* L){
	lua_settop(L,1);
	if (lua_isnil(L,1)){
		if (instance->mouseMoveFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->mouseMoveFunc);
			instance->mouseMoveFunc = LUA_NOREF;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->mouseMoveFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}
static int RegisterMouseButtonDown(lua_State* L){
	lua_settop(L,1);
	if (lua_isnil(L,1)){
		if (instance->mouseDownFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->mouseDownFunc);
			instance->mouseDownFunc = LUA_NOREF;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->mouseDownFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}
static int RegisterMouseButtonUp(lua_State* L){
	lua_settop(L,1);
	if (lua_isnil(L,1)){
		if (instance->mouseUpFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->mouseUpFunc);
			instance->mouseUpFunc = LUA_NOREF;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->mouseUpFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}
static int RegisterMouseWheel(lua_State* L){
	lua_settop(L,1);
	if (lua_isnil(L,1)){
		if (instance->mouseWheelFunc){
			luaL_unref(L,LUA_REGISTRYINDEX,instance->mouseWheelFunc);
			instance->mouseWheelFunc = LUA_NOREF;
		}
	} else if (!lua_isfunction(L,1)) return luaL_argerror(L,1,"Function expected");
	else {
		instance->mouseWheelFunc = luaL_ref(L,LUA_REGISTRYINDEX);
	}
	return 0;
}

void Input::ReceiveEvent(const Event* event){
	switch(event->type){
		case EVENT_KEY_DOWN:
			if (keyDownFunc != LUA_NOREF){
				lua_State* thread = lua_newthread(L);
				lua_rawgeti(thread,LUA_REGISTRYINDEX,keyDownFunc);
				lua_pushlstring(thread,(const char*)&((KeyEvent*)event)->key,1);

				Engine::instance->StartScript(thread,1);
			}
			break;
		case EVENT_KEY_UP:
			if (keyUpFunc != LUA_NOREF){
				lua_State* thread = lua_newthread(L);
				lua_rawgeti(thread,LUA_REGISTRYINDEX,keyUpFunc);
				lua_pushlstring(thread,(const char*)&((KeyEvent*)event)->key,1);
				Engine::instance->StartScript(thread,1);

			}
			break;
		case EVENT_MOUSE_MOVE:
			if (mouseMoveFunc != LUA_NOREF){
				lua_State* thread = lua_newthread(L);
				lua_rawgeti(thread,LUA_REGISTRYINDEX,mouseMoveFunc);
				lua_pushnumber(thread,(float)((MouseEvent*)event)->x);
				lua_pushnumber(thread,(float)((MouseEvent*)event)->y);
				Engine::instance->StartScript(thread,2);
			}
			break;
		case EVENT_MOUSE_BUTTON_DOWN:
			if (mouseDownFunc != LUA_NOREF){
				lua_State* thread = lua_newthread(L);
				lua_rawgeti(thread,LUA_REGISTRYINDEX,mouseDownFunc);
				
				lua_pushnumber(thread,((MouseEvent*)event)->x);
				lua_pushnumber(thread,((MouseEvent*)event)->y);

				unsigned int button = ((MouseEvent*)event)->flags;
				if (button & MouseEvent::BUTTON_LEFT) button = 1;
				else if (button & MouseEvent::BUTTON_RIGHT) button = 2;
				else if (button & MouseEvent::BUTTON_MIDDLE) button = 3;
				else if (button & MouseEvent::BUTTON_4) button = 4;
				else if (button & MouseEvent::BUTTON_5) button = 5;
				lua_pushinteger(thread,button);

				Engine::instance->StartScript(thread,3);
			}
			break;
		case EVENT_MOUSE_BUTTON_UP:
			if (mouseUpFunc != LUA_NOREF){
				lua_State* thread = lua_newthread(L);
				lua_rawgeti(thread,LUA_REGISTRYINDEX,mouseUpFunc);
				
				lua_pushnumber(thread,((MouseEvent*)event)->x);
				lua_pushnumber(thread,((MouseEvent*)event)->y);

				unsigned int button = ((MouseEvent*)event)->flags;
				if (button & MouseEvent::BUTTON_LEFT) button = 1;
				else if (button & MouseEvent::BUTTON_RIGHT) button = 2;
				else if (button & MouseEvent::BUTTON_MIDDLE) button = 3;
				else if (button & MouseEvent::BUTTON_4) button = 4;
				else if (button & MouseEvent::BUTTON_5) button = 5;
				lua_pushinteger(thread,button);

				Engine::instance->StartScript(thread,3);
			}
			break;
		case EVENT_MOUSE_WHEEL:
			if (mouseWheelFunc != LUA_NOREF){
				lua_State* thread = lua_newthread(L);
				lua_rawgeti(thread,LUA_REGISTRYINDEX, mouseWheelFunc);
				lua_pushnumber(thread,((MouseEvent*)event)->x);
				lua_pushnumber(thread,((MouseEvent*)event)->y);
				lua_pushnumber(thread,((signed short)((MouseEvent*)event)->flags)/120.0);
				Engine::instance->StartScript(thread,3);
			}
	}
	BroadcastEvent(event);
}

static int LoadEventLib(lua_State* L){
	const static luaL_Reg inputLib[] = {
		{"KeyDown",RegisterKeyDown},
		{"KeyUp",RegisterKeyUp},
		{"MouseMove", RegisterMouseMove},
		{"MouseButtonDown", RegisterMouseButtonDown},
		{"MouseButtonUp", RegisterMouseButtonUp},
		{"MouseWheel", RegisterMouseWheel},
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
	keyDownFunc(LUA_NOREF),
	keyUpFunc(LUA_NOREF),
	mouseMoveFunc(LUA_NOREF),
	mouseDownFunc(LUA_NOREF),
	mouseUpFunc(LUA_NOREF),
	mouseWheelFunc(LUA_NOREF),
	prevMouseFlags(0)
{
	instance = this;
}

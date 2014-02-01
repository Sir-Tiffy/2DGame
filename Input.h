#pragma once
#include "Observer.h"

class Input:public EventSubject, public EventObserver{
friend LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
friend int RegisterKeyDown(lua_State* L);
friend int RegisterKeyUp(lua_State* L);
friend int RegisterMouseMove(lua_State* L);
friend int RegisterMouseButtonDown(lua_State* L);
friend int RegisterMouseButtonUp(lua_State* L);
friend int RegisterMouseWheel(lua_State* L);
private:
	std::vector<std::unique_ptr<Event>> eventQueue;
	lua_State* L;
	int keyDownFunc;
	int keyUpFunc;
	int mouseDownFunc;
	int mouseUpFunc;
	int mouseMoveFunc;
	int mouseWheelFunc;
	unsigned int prevMouseFlags;
	void QueueEvent(std::unique_ptr<Event> event);
public:
	virtual void ReceiveEvent(const Event* event) override;
	void GetInput(WindowHandle window);
	void RegisterLuaEventHandler(lua_State* L);
	WNDPROC GetWndProc();
	Input();
};
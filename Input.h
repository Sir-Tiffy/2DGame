#pragma once
#include "Observer.h"

class Input:public EventSubject, public EventObserver{
friend LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
friend int RegisterKeyDown(lua_State* L);
friend int RegisterKeyUp(lua_State* L);
private:
	std::queue<std::unique_ptr<Event>> eventQueue;
	lua_State* L;
	int keyDownFunc;
	int keyUpFunc;
	void QueueEvent(std::unique_ptr<Event> event);
public:
	virtual void ReceiveEvent(const Event* event) override;
	void GetInput(WindowHandle window);
	void RegisterLuaEventHandler(lua_State* L);
	WNDPROC GetWndProc();
	Input();
};
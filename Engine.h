#pragma once
#include "StdAfx.h"
#include "Renderer.h"
#include "Input.h"
#include "GameObject.h"

class Engine:public EventSubject, public EventObserver{ friend int GetTop(lua_State* L);
friend static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	lua_State* L;
	std::string windowTitle;
	int width, height;
	bool fullscreen;
	bool vsync;
	bool resizable;

	double currentTime;
	double deltaTime;

	Renderer renderer;
	Input input;
	
	struct ThreadObject{
		lua_State* thread;
		double waitTime;
		int threadHandle;
		ThreadObject(lua_State* thread, double waitTime, int stackIndex):thread(thread),waitTime(waitTime),threadHandle(threadHandle){}
	};

	std::vector<ThreadObject> waitingLuaThreads;

	const HINSTANCE hInstance;
	const int nCmdShow;
	WindowHandle currentWindow;
	//HGLRC hrc;
	//HDC hdc;


	std::vector<GameObject::Sprite*> objects;

	void OnWindowResize(int width, int height);

	void ReadConfig();
	/*void OpenWindow();
	void CloseWindow();
	void InitGL();*/

	double CalculateTime();

	void RunScripts();
	//void BeginScripts();

	void GetInput();
	void UpdatePhysics();
	void UpdateLogic();
	
	friend int LuaScreen_Index(lua_State* L);
	friend int LuaScreen_NewIndex(lua_State* L);
	void RegisterLuaScreen(lua_State* L);
	//void Render();
protected:
public:
	std::ofstream outputLog;

	bool running;
	double GetTime();
	
	virtual void ReceiveEvent(const Event* event) override;

	void SetWindowTitle(std::string s);
	
	void StartScript(lua_State* thread, unsigned int args);

	void RecalculateCamera(GameObject::Camera* camera);
	void AddGameObject(GameObject::Sprite* sprite);
	void RemoveGameObject(GameObject::Sprite* sprite);
	void SetFullscreen(bool setting);
	void ToggleFullscreen();
	void BeginLoop();

	//Engine(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//~Engine();
	Engine(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	~Engine();

	static Engine* instance;
};
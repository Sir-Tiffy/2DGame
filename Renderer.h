#pragma once
#include "GameObject.h"
#include "Matrix.h"
#include "Texture.h"

class Renderer{
private:
	int
		width,
		height;
	bool
		vsync,
		fullscreen,
		resizable;
	std::string windowTitle;
	WindowHandle currentWindow;

	mat4 projectionMatrix;
	mat4 viewMatrix;

	Vec::vec2 cameraPosition;
	float cameraScale;
	float cameraRotation;

	GLuint renderBufferObject;

	HDC hdc;
	HGLRC hrc;
	HINSTANCE hInstance;
	int nCmdShow;
	
	static const char* const WINDOW_CLASSNAME;
	static const unsigned int BITS_PER_PIXEL = 32;

	void InitGL();

public:
	//opens a window and returns the window handle
	WindowHandle OpenWindow(int width, int height, std::string title, bool fullscreen, bool resizable);


	static bool UploadTexture(lua_State* L, Texture::Texture* tex, std::vector<char>& data);

	enum TextureFilter{
		LINEAR = GL_LINEAR,
		NEAREST = GL_NEAREST
	};

	static void SetTextureFilter(Texture::Texture* tex, TextureFilter filter);
	static void DeleteTexture(Texture::Texture* tex);

	//close the current open window
	void CloseWindow();
	
	void RecalculateCamera(Vec::vec2 position, float scale, float rotation);
	//enables/disables vertical sync.  Returns new vsync state (false if failed)
	bool SetVsync(bool enabled);
	 //set the title of an open window
	void SetWindowTitle(std::string title);

	void OnWindowResize(int width, int height);

	void Render(std::vector<GameObject::Sprite*>& objects);

	void Init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc);

	Renderer();
	~Renderer();
};

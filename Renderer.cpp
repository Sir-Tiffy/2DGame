#include "StdAfx.h"
#include "Renderer.h"
#include "Shader.h"

using namespace std;
using namespace Vec;

static void (APIENTRY* glGetShaderiv)(GLuint shader, GLenum name, GLint* params);
static void (APIENTRY* glGetShaderInfoLog)(GLuint shader, int maxLength, int* length, char* log);
static void (APIENTRY* glGetProgramiv)(GLuint program, GLenum name, GLint* params);
static void (APIENTRY* glGetProgramInfoLog)(GLuint program, int maxLength, int* length, char* log);
static bool (APIENTRY* wglSwapIntervalExt)(int);
static void (APIENTRY* glGenBuffers)(GLsizei n, GLuint* buffers);
static void (APIENTRY* glBindBuffer)(GLenum target, GLuint buffer);
static void (APIENTRY* glBufferData)(GLenum target, int size, const void* data, GLenum usage);
static void (APIENTRY* glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalize, GLsizei stride, const void* pointer);
static void (APIENTRY* glEnableVertexAttribArray)(GLuint index);
static GLuint (APIENTRY* glCreateShader)(GLenum target);
static void (APIENTRY* glShaderSource)(GLuint shader, GLsizei count, const char** string, GLint length);
static void (APIENTRY* glCompileShader)(GLuint shader);
static GLuint (APIENTRY* glCreateProgram)();
static void (APIENTRY* glAttachShader)(GLuint program, GLuint shader);
static void (APIENTRY* glBindAttribLocation)(GLuint program, GLuint index, char* name);
static void (APIENTRY* glLinkProgram)(GLuint program);
static void (APIENTRY* glUseProgram)(GLuint program);
static GLint (APIENTRY* glGetUniformLocation)(GLuint program, const char* name);
static void (APIENTRY* glUniformMatrix4fv)(GLint location, int count, GLboolean transpose, const GLfloat* value);
static void (APIENTRY* glActiveTexture)(GLenum texture);
static void (APIENTRY* glUniform1i)(GLint location, GLint v0);

const static int GL_COMPILE_STATUS = 0x8b81;
const static int GL_LINK_STATUS = 0x8b82;
const static int GL_ARRAY_BUFFER = 0x8892;
const static int GL_STREAM_DRAW = 0x88e0;
const static int GL_STATIC_DRAW = 0x88e4;
const static int GL_VERTEX_SHADER = 0x8b31;
const static int GL_FRAGMENT_SHADER = 0x8b30;
const static int GL_INFO_LOG_LENGTH = 0x8b84;
const static int GL_TEXTURE0 = 0x84c0;

static void GetGLFuncs(){
	const char* const extensions = (const char*)glGetString(GL_EXTENSIONS);
	//vsyncEnabled = strstr(extensions, "WGL_EXT_swap_control")!=0;
	glGetShaderiv = (void (APIENTRY*)(GLuint shader, GLenum name, GLint* params))wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (void (APIENTRY*)(GLuint shader, int maxLength, int* length, char* log))wglGetProcAddress("glGetShaderInfoLog");
	glGetProgramiv = (void (APIENTRY*)(GLuint program, GLenum name, GLint* params))wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (void (APIENTRY*)(GLuint program, int maxLength, int* length, char* log))wglGetProcAddress("glGetProgramInfoLog");
	wglSwapIntervalExt = (bool (APIENTRY*)(int))wglGetProcAddress("wglSwapIntervalEXT");
	glGenBuffers = (void (APIENTRY*)(GLsizei n, GLuint* buffers))wglGetProcAddress("glGenBuffers");
	glBindBuffer = (void(APIENTRY*)(GLenum target, GLuint buffer))wglGetProcAddress("glBindBuffer");
	glBufferData = (void(APIENTRY*)(GLenum target, int size, const void* data, GLenum usage))wglGetProcAddress("glBufferData");
	glVertexAttribPointer = (void(APIENTRY*)(GLuint index, GLint size, GLenum type, GLboolean normalize, GLsizei stride, const void* pointer))wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (void(APIENTRY*)(GLuint index))wglGetProcAddress("glEnableVertexAttribArray");
	glCreateShader = (GLuint (APIENTRY*)(GLenum target))wglGetProcAddress("glCreateShader");
	glShaderSource = (void (APIENTRY*)(GLuint shader, GLsizei count, const char** string, GLint length))wglGetProcAddress("glShaderSource");
	glCompileShader = (void (APIENTRY*)(GLuint shader))wglGetProcAddress("glCompileShader");
	glCreateProgram = (GLuint (APIENTRY*)())wglGetProcAddress("glCreateProgram");
	glAttachShader = (void (APIENTRY*)(GLuint program, GLuint shader))wglGetProcAddress("glAttachShader");
	glBindAttribLocation = (void (APIENTRY*)(GLuint program, GLuint index, char* name))wglGetProcAddress("glBindAttribLocation");
	glLinkProgram = (void (APIENTRY*)(GLuint program))wglGetProcAddress("glLinkProgram");
	glUseProgram = (void (APIENTRY*)(GLuint program))wglGetProcAddress("glUseProgram");
	glGetUniformLocation = (GLint (APIENTRY*)(GLuint program, const char* name))wglGetProcAddress("glGetUniformLocation");
	glUniformMatrix4fv = (void (APIENTRY*)(GLint location, int count, GLboolean transpose, const GLfloat* value))wglGetProcAddress("glUniformMatrix4fv");
	glActiveTexture = (void (APIENTRY*)(GLenum texture))wglGetProcAddress("glActiveTexture");
	glUniform1i = (void ( APIENTRY*)(GLint location, GLint v0))wglGetProcAddress("glUniform1i");
}

static void CheckGLError(const char* msg){
	const auto code = glGetError();
	if (code == GL_NO_ERROR) return;
	throw runtime_error(msg);
}

#define CheckGLError(msg) CheckGLError(msg ERROR_INFO)

static void CheckShader(GLuint shader, const char* msg){
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_NO_ERROR) return;
	glGetShaderiv(shader,GL_INFO_LOG_LENGTH, &status);
	const auto errLength = strlen(msg);
	vector<char> log(status+errLength+1);
	for (unsigned int i = 0; i <= errLength; ++i) log[i] = msg[i];
	log[errLength] = '\n';
	glGetShaderInfoLog(shader, status, &status, log.data()+errLength+1);
	throw runtime_error(log.data());
}

static void CheckProgram(GLuint program, const char* msg){
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_NO_ERROR) return;
	glGetProgramiv(program,GL_INFO_LOG_LENGTH, &status);
	const auto errLength = strlen(msg);
	vector<char> log(status+errLength+1);
	for (unsigned int i = 0; i <= errLength; ++i) log[i] = msg[i];
	log[errLength] = '\n';
	glGetProgramInfoLog(program, status, &status, log.data()+errLength+1);
	throw runtime_error(log.data());
}

WindowHandle Renderer::OpenWindow(int width, int height, std::string windowTitle, bool fullscreen){
	this->width = width;
	this->height = height;
	this->windowTitle = windowTitle;
	if (currentWindow != nullptr) throw logic_error("Attempted to open a window with one already open!");

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	DWORD style;
	DWORD exStyle;

	if (fullscreen){
		DEVMODE settings = {};
		settings.dmSize = sizeof(DEVMODE);
		settings.dmPelsWidth = width;
		settings.dmPelsHeight = height;
		settings.dmBitsPerPel = BITS_PER_PIXEL;
		settings.dmFields = DM_PELSHEIGHT|DM_PELSWIDTH|DM_BITSPERPEL;
		if (ChangeDisplaySettings(&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) fullscreen = false;
	}
	style = fullscreen?WS_POPUP:(WS_OVERLAPPEDWINDOW/*^WS_THICKFRAME^WS_MAXIMIZEBOX*/);
	exStyle = WS_EX_APPWINDOW|(fullscreen*WS_EX_WINDOWEDGE);
	this->fullscreen = fullscreen;

	ShowCursor(!fullscreen);

	AdjustWindowRectEx(&rect, style, false, exStyle);

	try{

		currentWindow = CreateWindowEx(exStyle, WINDOW_CLASSNAME, windowTitle.c_str(), style|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, hInstance, NULL);
		if (currentWindow == NULL) throw runtime_error("Failed to open window!");

		static PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),	//size
			1,	//version
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,	//support
			PFD_TYPE_RGBA,	//format
			BITS_PER_PIXEL,	//colour depth
			0,0,0,0,0,0,	//bits ignored
			0,	//no alpha buffer
			0,	//shift bit ignored
			0,	//no accumulation buffer
			0,0,0,0,	//accumulation bits ignored
			16,	//z-buffer
			0,	//no stencil buffer
			0,	//no auxiliary buffer
			PFD_MAIN_PLANE,	//main drawing layer
			0,	//reserved
			0,0,0	//layer masks
		};

		hdc = GetDC(currentWindow);
		if (!hdc) throw runtime_error("Failed to create an OpenGL device context!");
		const unsigned int pixelFormat = ChoosePixelFormat(hdc, &pfd);
		if (!pixelFormat) throw runtime_error("Failed to find a suitable pixel format!");
		if (!SetPixelFormat(hdc, pixelFormat, &pfd)) throw runtime_error("Failed to apply the pixel format!");
		hrc = wglCreateContext(hdc);
		if (!hrc) throw runtime_error("Failed to create an OpenGL rendering context!");
		if (!wglMakeCurrent(hdc, hrc)) throw runtime_error("Failed to activate the OpenGL rendering context!");
		
		ShowWindow(currentWindow, nCmdShow);
		SetForegroundWindow(currentWindow);
		SetFocus(currentWindow);
		OnWindowResize(width,height);
		UpdateWindow(currentWindow);

		InitGL();
		return currentWindow;
	} catch (exception& e){
		CloseWindow();
		throw e;
	}
}

void Renderer::CloseWindow(){
	if (currentWindow == nullptr) return;
	if (fullscreen){
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(true);
	}

	bool err = false;
	string errMsg;

	if (hrc){
		if (!wglMakeCurrent(NULL, NULL)){
			err = true;
			errMsg = "Failed to release DC and RC!" ERROR_INFO;
		}
		if (!wglDeleteContext(hrc) &&!err){
			err = true;
			errMsg = "Failed to release rendering context!" ERROR_INFO;
		}
		hrc = nullptr;
	}
	if (hdc){
		if (!ReleaseDC(currentWindow, hdc) &&!err){
			const auto r = GetLastError();
			err = true;
			errMsg = "Failed to release device context!" ERROR_INFO;
		}
		hdc = nullptr;
	}
	if (!DestroyWindow(currentWindow) && !err){
		err = true;
		errMsg = "Failed to destroy the window!" ERROR_INFO;
	}
	currentWindow = nullptr;
	if (err) throw runtime_error(errMsg);
}

bool Renderer::SetVsync(bool enabled){
	vsync = enabled;
	if (currentWindow == nullptr) return enabled;
	wglSwapIntervalExt(enabled);
	return enabled;
}


void Renderer::RecalculateCamera(vec2 position, float scale, float rotation){
	cameraPosition = -position;
	cameraScale = scale;
	cameraRotation = rotation;
	scale = 1/scale;
	/*viewMatrix = mat4(
		scale,0,0,0,
		0,scale,0,0,
		0,0,1,0,
		0,0,0,1
	);*/
	/*const mat4 scaleMat = mat4(
	scale,0,0,0,
	0,scale,0,0,
	0,0,1,0,
	0,0,0,1);
	const mat4 translateMat = mat4(
	1,0,0,cameraPosition.x,
	0,1,0,cameraPosition.y,
	0,0,1,0,
	0,0,0,1);
	const float cosTheta = cos(rotation);
	const float sinTheta = sin(rotation);
	const mat4 rotMat = mat4(
	cosTheta,-sinTheta,0,0,
	sinTheta,cosTheta,0,0,
	0,0,1,0,
	0,0,0,1);

	viewMatrix = scaleMat*rotMat*translateMat;*/
	const float sc = scale*cos(rotation);
	const float st = scale*sin(rotation);
	viewMatrix = mat4(
		sc,-st,0,cameraPosition.x*sc-cameraPosition.y*st,
		st,sc,0,cameraPosition.x*st+cameraPosition.y*sc,
		0,0,1,0,
		0,0,0,1
	);
}

void Renderer::OnWindowResize(int width, int height){
	this->width = width;
	this->height = height;
	glViewport(0,0,width,height);
	projectionMatrix = mat4(
		(float)height/width*2,0,0,0,
		0,2,0,0,
		0,0,0,0,
		0,0,0,1);
	//RecalculateCamera(cameraPosition,cameraScale, nearZ, farZ);
}

void Renderer::InitGL(){
	GetGLFuncs();
	glClearColor(0,0,0,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//const float aspect = (float)width/height;
	//glOrtho(-aspect,aspect,-1,1,0,1);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//viewMatrix = mat4::identity;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	SetVsync(vsync);

	glGenBuffers(1,&renderBufferObject);
}

void Renderer::Render(vector<GameObject::Sprite*>& sprites){
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (sprites.size() > 0){

		glEnable(GL_TEXTURE_2D);
	//	glActiveTexture(GL_TEXTURE0);

		//load shader
		static GLuint vert = 0;
		static GLuint frag = 0;
		static GLuint prog = 0;
		static GLuint vpUniform = 0;
		static GLuint texUniform = 0;
		if (prog == 0){
			vert = glCreateShader(GL_VERTEX_SHADER);
			frag = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(vert, 1, &VERT, 0);
			glShaderSource(frag,1, &FRAG, 0);
			glCompileShader(vert);
				CheckShader(vert,"Failed to compile vertex shader!" ERROR_INFO);
			glCompileShader(frag);
				CheckShader(frag,"Failed to copmile fragment shader!" ERROR_INFO);

			prog = glCreateProgram();
			glAttachShader(prog,vert);
			glAttachShader(prog,frag);
			glBindAttribLocation(prog,0,"in_Position");
			glBindAttribLocation(prog,1,"in_Colour");
			glBindAttribLocation(prog,2,"in_UV");
			glLinkProgram(prog);
				CheckProgram(prog, "Failed to link shader program!" ERROR_INFO);

			vpUniform = glGetUniformLocation(prog, "vp");
			texUniform = glGetUniformLocation(prog,"teq");
		}
		glUseProgram(prog);
		const mat4 vp = projectionMatrix*viewMatrix;
		glUniformMatrix4fv(vpUniform, 1, true, (float*)&vp.data);

		std::sort(sprites.begin(), sprites.end(), [](GameObject::Sprite* left, GameObject::Sprite* right){return left->GetSortKey() < right->GetSortKey();});

		glBindBuffer(GL_ARRAY_BUFFER, renderBufferObject);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2+3+2)*sizeof(float), 0); //position
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (2+3+2)*sizeof(float), (void*)(2*sizeof(float))); //colour
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (2+3+2)*sizeof(float), (void*)(5*sizeof(float))); //uv

		vector<float> data;
		unsigned int numVerts = 0;	
		unsigned int key = sprites[0]->GetSortKey();
		if (sprites[0]->GetTexture() != nullptr){
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(texUniform,0);
			glBindTexture(GL_TEXTURE_2D,sprites[0]->GetTexture()->textureHandle);
		} else glBindTexture(GL_TEXTURE_2D, 0);
		for (GameObject::Sprite* s:sprites){
			if (!s->visible) continue;
			if (s->GetSortKey() != key){
				glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STREAM_DRAW);
				glDrawArrays(GL_TRIANGLES,0,numVerts);
				numVerts = 0;
				data.clear();
				key = s->GetSortKey();
				if (s->GetTexture() != nullptr){
					glActiveTexture(GL_TEXTURE0);
					glUniform1i(texUniform,0);
					glBindTexture(GL_TEXTURE_2D,s->GetTexture()->textureHandle);
				} else glBindTexture(GL_TEXTURE_2D, 0);
				
			}

			numVerts += 6;
			data.reserve(6*(2+3+2));

			const float x0 = s->position.x;
			const float y0 = s->position.y;
			const float x1 = x0 + s->size.x;
			const float y1 = y0 + s->size.y;
			const float r = s->colour.r;
			const float g = s->colour.g;
			const float b = s->colour.b;
			const float uvX0 = 0;//s->UV.x;
			const float uvY0 = 0;//s->UV.y;
			const float uvX1 = 1;//s->UV.z;
			const float uvY1 = 1;//s->UV.w;

			data.emplace_back(x0);
			data.emplace_back(y0);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(uvX0);
			data.emplace_back(uvY0);

			data.emplace_back(x1);
			data.emplace_back(y0);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(uvX1);
			data.emplace_back(uvY0);

			data.emplace_back(x0);
			data.emplace_back(y1);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(uvX0);
			data.emplace_back(uvY1);

			data.emplace_back(x0);
			data.emplace_back(y1);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(uvX0);
			data.emplace_back(uvY1);

			data.emplace_back(x1);
			data.emplace_back(y0);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(uvX1);
			data.emplace_back(uvY0);

			data.emplace_back(x1);
			data.emplace_back(y1);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(uvX1);
			data.emplace_back(uvY1);
		}
		glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STREAM_DRAW);
		glDrawArrays(GL_TRIANGLES,0,numVerts);

	}
	CheckGLError("Error rendering scene!");
	SwapBuffers(hdc);
}
/*
	vector<float> verts;
	unsigned int numVerts = 0;

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);


	for (GameObject::Sprite* o:sprites){
		if (!o->visible) continue;

		if (o->GetTexture() != nullptr){
			glBindTexture(GL_TEXTURE_2D, o->GetTexture()->textureHandle);
			CheckGLError("Failed to load texture");
		}

		const float x0 = o->position.x;
		const float y0 = o->position.y;
		const float x1 = x0 + o->size.x;
		const float y1 = y0 + o->size.y;

		verts.reserve(verts.capacity()+7*6);
		numVerts += 6;

		verts.push_back(x0);
		verts.push_back(y0);
		verts.push_back(o->colour.r);
		verts.push_back(o->colour.g);
		verts.push_back(o->colour.b);
		verts.push_back(o->UV.x);
		verts.push_back(o->UV.y);

		verts.push_back(x1);
		verts.push_back(y0);
		verts.push_back(o->colour.r);
		verts.push_back(o->colour.g);
		verts.push_back(o->colour.b);
		verts.push_back(o->UV.z);
		verts.push_back(o->UV.y);

		verts.push_back(x0);
		verts.push_back(y1);
		verts.push_back(o->colour.r);
		verts.push_back(o->colour.g);
		verts.push_back(o->colour.b);
		verts.push_back(o->UV.x);
		verts.push_back(o->UV.w);

		verts.push_back(x0);
		verts.push_back(y1);
		verts.push_back(o->colour.r);
		verts.push_back(o->colour.g);
		verts.push_back(o->colour.b);
		verts.push_back(o->UV.x);
		verts.push_back(o->UV.w);

		verts.push_back(x1);
		verts.push_back(y0);
		verts.push_back(o->colour.r);
		verts.push_back(o->colour.g);
		verts.push_back(o->colour.b);
		verts.push_back(o->UV.z);
		verts.push_back(o->UV.y);

		verts.push_back(x1);
		verts.push_back(y1);
		verts.push_back(o->colour.r);
		verts.push_back(o->colour.g);
		verts.push_back(o->colour.b);
		verts.push_back(o->UV.z);
		verts.push_back(o->UV.w);
	}


	glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2+3+2)*sizeof(float), 0); //position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (2+3+2)*sizeof(float), (void*)(2*sizeof(float))); //colour
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (2+3+2)*sizeof(float), (void*)((2+3)*sizeof(float))); //uv
	glEnableVertexAttribArray(2);


	glDrawArrays(GL_TRIANGLES, 0, numVerts);
	CheckGLError("Error rendering scene!");
	SwapBuffers(hdc);
}*/

void Renderer::Init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndProc){
	this->nCmdShow = nCmdShow;
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASSNAME;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) throw runtime_error("Failed to register window class!");
}

Renderer::Renderer():
	currentWindow(nullptr),
	hdc(nullptr),
	hrc(nullptr),
	nCmdShow(0),
	hInstance(nullptr),
	fullscreen(false),
	vsync(false),
	width(800),
	height(600),
	projectionMatrix(mat4::identity),
	cameraPosition(0,0),
	cameraScale(1),
	cameraRotation(0)
{
	RecalculateCamera(cameraPosition,cameraScale, cameraRotation);
}

Renderer::~Renderer(){
	try{
		CloseWindow();
	} catch (exception &e){
		MessageBox(NULL, e.what(), "Fatal error!", MB_ICONERROR|MB_DEFAULT_DESKTOP_ONLY);
	}
	UnregisterClass(WINDOW_CLASSNAME, hInstance);
}

void Renderer::UploadTexture(lua_State* L, Texture::Texture* tex, vector<char>& data, int width, int height, short bpp){
	GLuint& textureHandle = tex->textureHandle;
	glGenTextures(1,&textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	switch(bpp){
		case 24: bpp = GL_RGB; break;
		case 32: bpp = GL_RGBA; break;
		default: bpp = 0;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,height,0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	if (glGetError() != GL_NO_ERROR) luaL_error(L, "Failed to load texture '%s': Error uploading texture!",tex->filename.data());
}

void Renderer::DeleteTexture(Texture::Texture* tex){
	glDeleteTextures(1, &tex->textureHandle);
	tex->textureHandle = 0;
}

const char* const Renderer::WINDOW_CLASSNAME = "2DGAMEWINDOW";
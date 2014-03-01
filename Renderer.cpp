#include "StdAfx.h"
#include "Renderer.h"
#include "Shader.h"

using namespace std;
using namespace Vec;

WindowHandle Renderer::OpenWindow(int width, int height, std::string windowTitle, bool fullscreen, bool resizable){
	this->width = width;
	this->height = height;
	this->resizable = resizable;
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
	style = fullscreen?WS_POPUP:(WS_OVERLAPPEDWINDOW^((!resizable)*(WS_THICKFRAME|WS_MAXIMIZEBOX)));
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

Vec::vec2 Renderer::ScreenToWorld(float x1, float y1){
	x1 = 2*x1/width-1;
	y1 = 2*(1-y1/height-.5f);
	const float a = (float)height/width;
	const float sinTheta = sin(cameraRotation);
	const float cosTheta = cos(cameraRotation);
	const float as2 = 1/(a*cameraScale*2);
	return Vec::vec2((a*y1*sinTheta+x1*cosTheta)*as2 - cameraPosition.x, (a*y1*cosTheta - x1*sinTheta)*as2 - cameraPosition.y);
}

void Renderer::OnWindowResize(int width, int height){
	this->width = width;
	this->height = height;
	glViewport(0,0,width,height);
	projectionMatrix = mat4(
		2.0f*height/width,0,0,0,
		0,2,0,0,
		0,0,0,0,
		0,0,0,1);
	//RecalculateCamera(cameraPosition,cameraScale, nearZ, farZ);
}

void Renderer::ResizeWindow(int width, int height){
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRectEx(&rect, GetWindowLong(currentWindow,GWL_STYLE), false, GetWindowLong(currentWindow,GWL_EXSTYLE));

	SetWindowPos(currentWindow,HWND_TOP,0,0,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOMOVE);
	return OnWindowResize(width,height);
}

void Renderer::SetWindowTitle(string title){
	SetWindowText(currentWindow,title.c_str());
}

void Renderer::SetResizable(bool resizable){
	this->resizable = resizable;
	if (fullscreen) return;
	const auto style = GetWindowLong(currentWindow,GWL_STYLE);
	SetWindowLong(currentWindow, GWL_STYLE, (style&(~(WS_THICKFRAME|WS_MAXIMIZEBOX)))|(resizable*(WS_THICKFRAME|WS_MAXIMIZEBOX)));
	SetWindowPos(currentWindow,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED);
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
	
	texturedShader.CreateShader("shaders/texturedF.glsl","shaders/texturedV.glsl");
	untexturedShader.CreateShader("shaders/untexturedF.glsl","shaders/untexturedV.glsl");
}

void Renderer::Render(vector<GameObject::Sprite*>& sprites){
	CheckGLError("Unknown error before rendering");
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (sprites.size() > 0){

	//	glEnable(GL_TEXTURE_2D);
	//	glActiveTexture(GL_TEXTURE0);


		//upload view/projection matrix to shaders
		const mat4 vp = projectionMatrix*viewMatrix;
		untexturedShader.UseProgram();
		CheckGLError("Error using untextured shader");
		untexturedShader.UploadVPMatrix(false,vp.data);
		CheckGLError("Error uploading matrix to untextured shader");
		texturedShader.UseProgram();
		CheckGLError("Error using untextured shader");
		texturedShader.UploadVPMatrix(false,vp.data);
		CheckGLError("Error uploading matrix to untextured shader");

		Shader::UnloadProgram();

		std::sort(sprites.begin(), sprites.end(), [](GameObject::Sprite* left, GameObject::Sprite* right){return left->GetSortKey() < right->GetSortKey();});

		glBindBuffer(GL_ARRAY_BUFFER, renderBufferObject);
		/*glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), 0); //position
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), (void*)(2*sizeof(float))); //colour
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), (void*)((2+4)*sizeof(float))); //uv*/

		vector<float> data;
		unsigned int numVerts = 0;	
		unsigned int key = -1;

		unsigned int currentTextureHandle = 0;

		for (GameObject::Sprite* s:sprites){
			if (!s->visible) continue;
			const bool textured = s->GetTexture() != nullptr;
			if (s->GetSortKey() != key){
				if (numVerts){
					glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STREAM_DRAW);
					glDrawArrays(GL_TRIANGLES,0,numVerts);
					data.clear();
					numVerts = 0;
				}
				key = s->GetSortKey();
				if (textured){
					if (s->GetTexture()->textureHandle != currentTextureHandle){
						if (texturedShader.UseProgram()){
							glEnableVertexAttribArray(0);
							glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), 0); //position
							glEnableVertexAttribArray(1);
							glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), (void*)(2*sizeof(float))); //colour
							glEnableVertexAttribArray(2);
							glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), (void*)((2+4)*sizeof(float))); //uv
						}
						texturedShader.BindTexture(currentTextureHandle = s->GetTexture()->textureHandle);
						//glBindTexture(GL_TEXTURE_2D,currentTextureHandle = s->GetTexture()->textureHandle);
						//glEnableVertexAttribArray(2);
						//glActiveTexture(GL_TEXTURE0);
						///glUniform1i(texUniform,0);
						//glEnable(GL_TEXTURE_2D);
						//glBindTexture(GL_TEXTURE_2D,currentTextureHandle = s->GetTexture()->textureHandle); 
					}
				} else {
					if (untexturedShader.UseProgram()){
						glEnableVertexAttribArray(0);
						glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (2+4)*sizeof(float), 0); //position
						glEnableVertexAttribArray(1);
						glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (2+4)*sizeof(float), (void*)(2*sizeof(float))); //colour
						glDisableVertexAttribArray(2);
						//glEnableVertexAttribArray(2);
						//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (2+4+2)*sizeof(float), (void*)((2+4)*sizeof(float))); //uv
					}
					//glBindTexture(GL_TEXTURE_2D, 0);
					//glDisable(GL_TEXTURE_2D);
				}
				
			}

			numVerts += 6;
			data.reserve(data.size()+6*(2+4+textured*2));

			const float x0 = s->position.x;
			const float y0 = s->position.y;
			const float x1 = x0 + s->size.x;
			const float y1 = y0 + s->size.y;
			const float r = s->colour.r;
			const float g = s->colour.g;
			const float b = s->colour.b;
			const float a = s->colour.a;
			const float uvX0 = s->UV.x;
			const float uvY0 = s->UV.y;
			const float uvX1 = s->UV.z;
			const float uvY1 = s->UV.w;

			data.emplace_back(x0);
			data.emplace_back(y0);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(a);
			if (textured){
				data.emplace_back(uvX0);
				data.emplace_back(uvY0);
			}

			data.emplace_back(x1);
			data.emplace_back(y0);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(a);
			if (textured){
				data.emplace_back(uvX1);
				data.emplace_back(uvY0);
			}

			data.emplace_back(x0);
			data.emplace_back(y1);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(a);
			if (textured){
				data.emplace_back(uvX0);
				data.emplace_back(uvY1);
			}

			data.emplace_back(x0);
			data.emplace_back(y1);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(a);
			if (textured){
				data.emplace_back(uvX0);
				data.emplace_back(uvY1);
			}

			data.emplace_back(x1);
			data.emplace_back(y0);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(a);
			if (textured){
				data.emplace_back(uvX1);
				data.emplace_back(uvY0);
			}

			data.emplace_back(x1);
			data.emplace_back(y1);
			data.emplace_back(r);
			data.emplace_back(g);
			data.emplace_back(b);
			data.emplace_back(a);
			if (textured){
				data.emplace_back(uvX1);
				data.emplace_back(uvY1);
			}
		}
		if (numVerts){
			glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STREAM_DRAW);
			glDrawArrays(GL_TRIANGLES,0,numVerts);
		}

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
	resizable(false),
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

bool Renderer::UploadTexture(lua_State* L, Texture::Texture* tex, vector<char>& data){
	GLuint& textureHandle = tex->textureHandle;
	glGenTextures(1,&textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width,tex->height,0, tex->bitmapFormat, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	if (glGetError() != GL_NO_ERROR){
		lua_pushfstring(L, "Failed to load texture '%s': Error uploading texture!",tex->filename.data());
		return true;
	}
	return false;
}

void Renderer::SetTextureFilter(Texture::Texture* tex, TextureFilter filter){
	glBindTexture(GL_TEXTURE_2D, tex->textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Renderer::DeleteTexture(Texture::Texture* tex){
	glDeleteTextures(1, &tex->textureHandle);
	tex->textureHandle = 0;
}

const char* const Renderer::WINDOW_CLASSNAME = "2DGAMEWINDOW";
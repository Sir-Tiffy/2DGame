#include "StdAfx.h"
#include "GLExtensions.h"

void (APIENTRY* glGetShaderiv)(GLuint shader, GLenum name, GLint* params);
void (APIENTRY* glGetShaderInfoLog)(GLuint shader, int maxLength, int* length, char* log);
void (APIENTRY* glGetProgramiv)(GLuint program, GLenum name, GLint* params);
void (APIENTRY* glGetProgramInfoLog)(GLuint program, int maxLength, int* length, char* log);
bool (APIENTRY* wglSwapIntervalExt)(int);
void (APIENTRY* glGenBuffers)(GLsizei n, GLuint* buffers);
void (APIENTRY* glBindBuffer)(GLenum target, GLuint buffer);
void (APIENTRY* glBufferData)(GLenum target, int size, const void* data, GLenum usage);
void (APIENTRY* glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalize, GLsizei stride, const void* pointer);
void (APIENTRY* glEnableVertexAttribArray)(GLuint index);
void (APIENTRY* glDisableVertexAttribArray)(GLuint index);
GLuint (APIENTRY* glCreateShader)(GLenum target);
void (APIENTRY* glShaderSource)(GLuint shader, GLsizei count, const char** string, GLint length);
void (APIENTRY* glCompileShader)(GLuint shader);
GLuint (APIENTRY* glCreateProgram)();
void (APIENTRY* glAttachShader)(GLuint program, GLuint shader);
void (APIENTRY* glBindAttribLocation)(GLuint program, GLuint index, char* name);
void (APIENTRY* glLinkProgram)(GLuint program);
void (APIENTRY* glUseProgram)(GLuint program);
GLint (APIENTRY* glGetUniformLocation)(GLuint program, const char* name);
void (APIENTRY* glUniformMatrix4fv)(GLint location, int count, GLboolean transpose, const GLfloat* value);
void (APIENTRY* glActiveTexture)(GLenum texture);
void (APIENTRY* glUniform1i)(GLint location, GLint v0);

void GetGLFuncs(){
	//const char* const extensions = (const char*)glGetString(GL_EXTENSIONS);
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
	glDisableVertexAttribArray = (void(APIENTRY*)(GLuint index))wglGetProcAddress("glDisableVertexAttribArray");
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
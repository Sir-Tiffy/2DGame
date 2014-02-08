#pragma once

extern void (APIENTRY* glGetShaderiv)(GLuint shader, GLenum name, GLint* params);
extern void (APIENTRY* glGetShaderInfoLog)(GLuint shader, int maxLength, int* length, char* log);
extern void (APIENTRY* glGetProgramiv)(GLuint program, GLenum name, GLint* params);
extern void (APIENTRY* glGetProgramInfoLog)(GLuint program, int maxLength, int* length, char* log);
extern bool (APIENTRY* wglSwapIntervalExt)(int);
extern void (APIENTRY* glGenBuffers)(GLsizei n, GLuint* buffers);
extern void (APIENTRY* glBindBuffer)(GLenum target, GLuint buffer);
extern void (APIENTRY* glBufferData)(GLenum target, int size, const void* data, GLenum usage);
extern void (APIENTRY* glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalize, GLsizei stride, const void* pointer);
extern void (APIENTRY* glEnableVertexAttribArray)(GLuint index);
extern void (APIENTRY* glDisableVertexAttribArray)(GLuint index);
extern GLuint (APIENTRY* glCreateShader)(GLenum target);
extern void (APIENTRY* glShaderSource)(GLuint shader, GLsizei count, const char** string, GLint length);
extern void (APIENTRY* glCompileShader)(GLuint shader);
extern GLuint (APIENTRY* glCreateProgram)();
extern void (APIENTRY* glAttachShader)(GLuint program, GLuint shader);
extern void (APIENTRY* glBindAttribLocation)(GLuint program, GLuint index, char* name);
extern void (APIENTRY* glLinkProgram)(GLuint program);
extern void (APIENTRY* glUseProgram)(GLuint program);
extern GLint (APIENTRY* glGetUniformLocation)(GLuint program, const char* name);
extern void (APIENTRY* glUniformMatrix4fv)(GLint location, int count, GLboolean transpose, const GLfloat* value);
extern void (APIENTRY* glActiveTexture)(GLenum texture);
extern void (APIENTRY* glUniform1i)(GLint location, GLint v0);

#define GL_COMPILE_STATUS 0x8b81
#define GL_LINK_STATUS 0x8b82
#define GL_ARRAY_BUFFER 0x8892
#define GL_STREAM_DRAW 0x88e0
#define GL_STATIC_DRAW 0x88e4
#define GL_VERTEX_SHADER 0x8b31
#define GL_FRAGMENT_SHADER 0x8b30
#define GL_INFO_LOG_LENGTH 0x8b84
#define GL_TEXTURE0 0x84c0

static void CheckGLError(const char* msg){
	//const auto code = glGetError();
	if (glGetError() == GL_NO_ERROR) return;
	throw std::runtime_error(msg);
}
#define CheckGLError(msg) CheckGLError(msg ERROR_INFO)

void GetGLFuncs();
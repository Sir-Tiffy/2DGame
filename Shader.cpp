#include "StdAfx.h"
#include "Shader.h"
#include "GLExtensions.h"

using namespace std;

static void CheckShader(GLuint shader, string msg){
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_NO_ERROR) return;
	glGetShaderiv(shader,GL_INFO_LOG_LENGTH, &status);
	const auto errLength = msg.length();
	vector<char> log(status+errLength+1);
	memcpy(log.data(),msg.c_str(),errLength);
	//for (unsigned int i = 0; i <= errLength; ++i) log[i] = msg[i];
	log[errLength] = '\n';
	glGetShaderInfoLog(shader, status, &status, log.data()+errLength+1);
	throw runtime_error(log.data());
}

static void CheckProgram(GLuint program, string msg){
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_NO_ERROR) return;
	glGetProgramiv(program,GL_INFO_LOG_LENGTH, &status);
	const auto errLength = msg.length();
	vector<char> log(status+errLength+1);
	memcpy(log.data(),msg.c_str(),errLength);
	//std::copy(msg.begin(), msg.end(), log.begin());
	//for (unsigned int i = 0; i <= errLength; ++i) log[i] = msg[i];
	log[errLength] = '\n';
	glGetProgramInfoLog(program, status, &status, log.data()+errLength+1);
	throw runtime_error(log.data());
}


void Shader::CreateShader(string fragmentFilename, string vertexFilename){
	vert = glCreateShader(GL_VERTEX_SHADER);
	frag = glCreateShader(GL_FRAGMENT_SHADER);

			
	ifstream f(vertexFilename,ios::in);
	if (!f.good()) throw runtime_error("Failed to open "+vertexFilename);
	f.seekg(0,ios::end);
	unsigned int filesize = (unsigned int)f.tellg();
	if (filesize == 0) throw runtime_error("Failed to load shader '"+vertexFilename +"': File is empty!");
	f.seekg(0,ios::beg);
	vector<char> data(filesize);
	f.read(data.data(),filesize);
	f.close();
	const char* ptr = data.data();
	glShaderSource(vert,1,&(ptr),0);
	data.clear();

	f.open(fragmentFilename,ios::in);
	if (!f.good()) throw runtime_error("Failed to open "+fragmentFilename);
	f.seekg(0,ios::end);
	filesize = (unsigned int)f.tellg();
	if (filesize == 0) throw runtime_error("Failed to load shader '"+fragmentFilename +"': File is empty!");
	f.seekg(0,ios::beg);
	data.resize(filesize);
	f.read(data.data(),filesize);
	f.close();
	ptr = data.data();
	glShaderSource(frag,1,&ptr,0);



	//glShaderSource(vert, 1, &VERT, 0);
	//glShaderSource(frag,1, &FRAG, 0);
	glCompileShader(vert);
	CheckShader(vert,"Failed to compile vertex shader '"+vertexFilename+"'!");
	glCompileShader(frag);
	CheckShader(frag,"Failed to compile fragment shader '"+fragmentFilename+"'!");

	prog = glCreateProgram();
	glAttachShader(prog,vert);
	glAttachShader(prog,frag);

	BindAttribLocations();

	/*glBindAttribLocation(prog,0,"in_Position");
	glBindAttribLocation(prog,1,"in_Colour");
	glBindAttribLocation(prog,2,"in_UV");*/
	glLinkProgram(prog);
	CheckProgram(prog, "Failed to link shader program!");

	GetUniformLocations();
	/*vpUniform = glGetUniformLocation(prog, "vp");
	texUniform = glGetUniformLocation(prog,"teq");*/
	CheckGLError("Error creating shader");
}

bool Shader::UseProgram(){
	if (currentShader == this) return false;
	glUseProgram(prog);
	currentShader = this;
	return true;
}

void TexturedShader::BindAttribLocations(){
	glBindAttribLocation(prog,0,"in_Position");
	glBindAttribLocation(prog,1,"in_Colour");
	glBindAttribLocation(prog,2,"in_UV");
}
void TexturedShader::GetUniformLocations(){
	vpUniformLocation = glGetUniformLocation(prog, "vp");
	texUniformLocation = glGetUniformLocation(prog,"tex");
}
bool TexturedShader::UseProgram(){
	if (!Shader::UseProgram()) return false;
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texUniformLocation,0);
	glEnable(GL_TEXTURE_2D);
	CheckGLError("Failed to enable texture settings");
	return true;
}
void TexturedShader::UploadVPMatrix(bool transpose, const float* data){
	if (currentShader != this) throw logic_error("Tried to upload view/projection matrix to unloaded shader!");
	return glUniformMatrix4fv(vpUniformLocation, 1, transpose, data);
}
void TexturedShader::BindTexture(GLuint textureHandle){
	glBindTexture(GL_TEXTURE_2D, textureHandle);
}

void UntexturedShader::BindAttribLocations(){
	glBindAttribLocation(prog,0,"in_Position");
	glBindAttribLocation(prog,1,"in_Colour");
}
void UntexturedShader::GetUniformLocations(){
	vpUniformLocation = glGetUniformLocation(prog, "vp");
}
bool UntexturedShader::UseProgram(){
	if (!Shader::UseProgram()) return false;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGLError("Failed to disable texture settings");
	return true;
}
void UntexturedShader::UploadVPMatrix(bool transpose, const float* data){
	if (currentShader != this) throw logic_error("Tried to upload view/projection matrix to unloaded shader!");
	return glUniformMatrix4fv(vpUniformLocation, 1, transpose, data);
}

Shader* Shader::currentShader = nullptr;
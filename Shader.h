#pragma once

class Shader{
protected:
	static Shader* currentShader;
	GLuint frag, vert, prog;
	virtual void BindAttribLocations()=0;
	virtual void GetUniformLocations()=0;
public:
	virtual bool UseProgram();
	void CreateShader(std::string fragmentFilename, std::string vertexFilename);
	static inline void UnloadProgram(){
		currentShader = nullptr;
	}
};

class TexturedShader:public Shader{
protected:
	GLuint vpUniformLocation, texUniformLocation;
	virtual void BindAttribLocations() override;
	virtual void GetUniformLocations() override;
public:
	void UploadVPMatrix(bool transpose, const float* data);
	void BindTexture(GLuint textureHandle);
	virtual bool UseProgram() override;
};

class UntexturedShader:public Shader{
protected:
	GLuint vpUniformLocation;
	virtual void BindAttribLocations() override;
	virtual void GetUniformLocations() override;
public:
	void UploadVPMatrix(bool transpose, const float* data);
	virtual bool UseProgram() override;
};

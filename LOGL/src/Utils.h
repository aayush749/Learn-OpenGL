#pragma once
#include <fstream>
#include <string>

#include "GL/glew.h"

namespace Utils {
	void readShader(const char* filepath, std::string& string);
	void getShaderLog(GLuint shader);
	void getProgramLog(GLuint program);
	bool checkOpenGLError();
	GLuint CreateShaderProgram();
	GLuint LoadTexture(const char* texImagePath);
}
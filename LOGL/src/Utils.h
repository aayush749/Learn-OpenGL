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


	// GOLD material - ambient, diffuse, specular, and shininess
	float* goldAmbient();
	float* goldDiffuse();
	float* goldSpecular();
	float goldShininess();
	// SILVER material - ambient, diffuse, specular, and shininess
	float* silverAmbient();
	float* silverDiffuse();
	float* silverSpecular();
	float silverShininess();
	// BRONZE material - ambient, diffuse, specular, and shininess
	float* bronzeAmbient();
	float* bronzeDiffuse();
	float* bronzeSpecular();
	float bronzeShininess();
}
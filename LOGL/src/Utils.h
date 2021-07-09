#pragma once
#include <fstream>
#include <string>

#include "GL/glew.h"
#include <GLFW/glfw3.h>

namespace Utils {
	void readShader(const char* filepath, std::string& string);
	void getShaderLog(GLuint shader);
	void getProgramLog(GLuint program);
	bool checkOpenGLError();
	GLuint CreateShaderProgram(const char* vShaderFilePath, const char* fShaderFilePath);
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

	//For managing input
	void ManageInput(GLFWwindow* window, int key, int scancode, int action, int mods);
}
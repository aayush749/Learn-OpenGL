#include "Utils.h"
#include <SOIL2/SOIL2.h>

#include <iostream>
#include <vector>

enum class Input
{
	LEFT, RIGHT
};

extern GLuint currentShaderIndex, renderingProgram;
extern std::vector<GLuint> shaderPrograms;

namespace Utils {
	void readShader(const char* filepath, std::string& string)
	{
		std::ifstream shader_file(filepath);
		if (!shader_file)
		{
			printf("File not found: %s", filepath);
			return;
		}

		std::string line;
		while (getline(shader_file, line))
		{
			string += line + '\n';
		}
		shader_file.close();
	}

	void getShaderLog(GLuint shader)
	{
		int length;
		int chWritten;
		char* log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			log = (char*)alloca(sizeof(char) * length);
			glGetShaderInfoLog(shader, length, &chWritten, log);
			printf("[Error compiling shader]: %s\n", log);
		}
	}

	void getProgramLog(GLuint program)
	{
		int length;
		int chWritten;
		char* log;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			log = (char*)alloca(sizeof(char) * length);
			glGetProgramInfoLog(program, length, &chWritten, log);
			printf("[Error creating program]: %s\n", log);
		}
	}

	bool checkOpenGLError()
	{
		bool foundError = false;
		int glError = glGetError();
		while (glError != GL_NO_ERROR)
		{
			printf("glError: %d\n", glError);
			glError = glGetError();
			foundError = true;
		}

		return foundError;
	}

	GLuint CreateShaderProgram(const char* vertexShaderFilePath, const char* fragmentShaderFilePath)
	{
		std::string vShaderString, fShaderString;
		readShader(vertexShaderFilePath, vShaderString);
		readShader(fragmentShaderFilePath, fShaderString);

		GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
		getShaderLog(vShader);

		GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
		getShaderLog(fShader);

		const char* vShaderSource = vShaderString.c_str();
		const char* fShaderSource = fShaderString.c_str();

		glShaderSource(vShader, 1, &vShaderSource, NULL);

		glShaderSource(fShader, 1, &fShaderSource, NULL);

		glCompileShader(vShader);

		glCompileShader(fShader);


		GLuint vfProgram = glCreateProgram();

		glAttachShader(vfProgram, vShader);

		glAttachShader(vfProgram, fShader);
		getProgramLog(vfProgram);

		glLinkProgram(vfProgram);
		getProgramLog(vfProgram);


		return vfProgram;
	}

	GLuint LoadTexture(const char* texImagePath) 
	{
		GLuint textureID;
		textureID = SOIL_load_OGL_texture(texImagePath,
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
		if (textureID == 0) 
			std::cout << "Could not find texture file" << texImagePath << std::endl;

		return textureID;
	}

	// GOLD material - ambient, diffuse, specular, and shininess
	float* goldAmbient() { static float a[4] = { 0.2473f, 0.1995f, 0.0745f, 1 }; return (float*)a; }
	float* goldDiffuse() { static float a[4] = { 0.7516f, 0.6065f, 0.2265f, 1 }; return (float*)a; }
	float* goldSpecular() { static float a[4] = { 0.6283f, 0.5559f, 0.3661f, 1 }; return (float*)a; }
	float goldShininess() { return 51.2f; }
	// SILVER material - ambient, diffuse, specular, and shininess
	float* silverAmbient() { static float a[4] = { 0.1923f, 0.1923f, 0.1923f, 1 }; return (float*)a; }
	float* silverDiffuse() { static float a[4] = { 0.5075f, 0.5075f, 0.5075f, 1 }; return (float*)a; }
	float* silverSpecular() { static float a[4] = { 0.5083f, 0.5083f, 0.5083f, 1 }; return (float*)a; }
	float silverShininess() { return 51.2f; }
	// BRONZE material - ambient, diffuse, specular, and shininess
	float* bronzeAmbient() { static float a[4] = { 0.2125f, 0.1275f, 0.0540f, 1 }; return (float*)a; }
	float* bronzeDiffuse() { static float a[4] = { 0.7140f, 0.4284f, 0.1814f, 1 }; return (float*)a; }
	float* bronzeSpecular() { static float a[4] = { 0.3936f, 0.2719f, 0.1667f, 1 }; return (float*)a; }
	float bronzeShininess() { return 25.6f; }

	static void ReTargetShader(Input ip)
	{
		if (ip == Input::LEFT && currentShaderIndex != 0)
		{
			currentShaderIndex--;
		}

		else if (ip == Input::RIGHT && currentShaderIndex != shaderPrograms.size() - 1)
		{
			currentShaderIndex++;
		}

		//after setting the current shader model, use that shader
		renderingProgram = shaderPrograms[currentShaderIndex];
	}

	//For managing input
	void ManageInput(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
			ReTargetShader(Input::LEFT);

		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
			ReTargetShader(Input::RIGHT);
	}

}
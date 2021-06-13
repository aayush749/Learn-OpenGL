#include "Utils.h"
#include <SOIL2/SOIL2.h>

#include <iostream>

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

	GLuint CreateShaderProgram()
	{
		std::string vShaderString, fShaderString;
		readShader("shaders/vertexShader.shader", vShaderString);
		readShader("shaders/fragmentShader.shader", fShaderString);

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
}
#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utils.h"
#include "Torus.h"
#include "ImportedModel.h"

#define numVAOs 1
#define numVBOs 3

float cameraX, cameraY, cameraZ;
float cubeLocX, cubeLocY, cubeLocZ;
float torusLocX, torusLocY, torusLocZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
float x = 0.0f;
float increment = 0.01f;
float tf; //time factor
// allocate variables used in display() function, so that they won’t need to be allocated during rendering
GLuint mvLoc, projLoc, nLoc;
int width, height;
float aspect;

ImportedModel model("Models/Suzanne.obj");
Torus myTorus(0.5f, 0.2f, 48);

//Some initilizations to test the lighting (Gouraud)
//Location for shader uniform variables
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mAmbLoc, mDiffLoc, mSpecLoc, mShiLoc;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos, lightPosV; // light position as Vector3f, in both model and view space
float lightPos[3]; // light position as float array
// initial light location
glm::vec3 initialLightLoc = glm::vec3(5.0f, 2.0f, 2.0f);
// white light properties
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
// silver material properties
float* matAmb = Utils::silverAmbient();
float* matDif = Utils::silverDiffuse();
float* matSpe = Utils::silverSpecular();
float matShi = Utils::silverShininess();

void InstallLights(const glm::mat4& vMat);

void setupVertices(void) 
{
	std::vector<glm::vec3> vert = model.getVertices();
	std::vector<glm::vec2> tex = model.getTextureCoords();
	std::vector<glm::vec3> norm = model.getNormals();
	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	int numVertices = model.getNumVertices();
	//Push the location coordinates, texture coordinates and normal vector values into corresponding vectors
	for (int i = 0; i < numVertices; i++) 
	{
		pvalues.push_back(vert[i].x);
		pvalues.push_back(vert[i].y);
		pvalues.push_back(vert[i].z);
		tvalues.push_back(tex[i].s);
		tvalues.push_back(tex[i].t);
		nvalues.push_back(norm[i].x);
		nvalues.push_back(norm[i].y);
		nvalues.push_back(norm[i].z);
	}
	glGenVertexArrays(numVAOs, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo); // generate VBOs as before, plus one for indices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // vertex positions
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * sizeof(float), &pvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // texture coordinates
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * sizeof(float), &tvalues[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); // normal vectors
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * sizeof(float), &nvalues[0], GL_STATIC_DRAW);
}


void init(GLFWwindow* window) 
{
	renderingProgram = Utils::CreateShaderProgram();
	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 5.0f;
	torusLocX = 0.0f; torusLocY = 0.0f; torusLocZ = 0.0f;
	setupVertices();
}

void Display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);

	// get the uniform variables for the MV and projection matrices
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	// build perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// pass on the view matrix and the model matrix for the pyramid (after applying some rotation and scale transforms)
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(torusLocX, torusLocY, torusLocZ));
	
	//rotate the torus to make it easier to see
	mMat *= glm::rotate(glm::mat4(1.0f), float(currentTime), glm::vec3(1.0f, 1.0f, 1.0f));

	//Setup lights based on the current light positions
	currentLightPos = glm::vec3(initialLightLoc.x, initialLightLoc.y, initialLightLoc.z);
	InstallLights(vMat);

	//Now build the model view matrices by concatenating the view and the model matrices
	mvMat = vMat * mMat;
	
	//build the inverse transpose of the MV matrix for transforming normal vectors
	invTrMat = glm::transpose(glm::inverse(mvMat));
	
	//Put the mv, inverseTranspose (normal) and proj matrices into the corresponding uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// bind the vertices buffer (VBO #0) to vertex attribute #0 in the vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	// bind the normals buffer (in VBO #2) to vertex attribute #1 in the vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glDrawArrays(GL_TRIANGLES, 0, model.getNumVertices());
}


void InstallLights(const glm::mat4& vMat)
{
	//Convert light's position into viewspace and store it in a float array
	lightPosV = glm::vec3(vMat * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = lightPosV.x;
	lightPos[1] = lightPosV.y;
	lightPos[2] = lightPosV.z;

	//Get the location of the lights and the material fields in the shader
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "light.position");
	mAmbLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mDiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mSpecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mShiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

	// set the uniform light and material values in the shader
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
	glProgramUniform4fv(renderingProgram, mAmbLoc, 1, matAmb);
	glProgramUniform4fv(renderingProgram, mDiffLoc, 1, matDif);
	glProgramUniform4fv(renderingProgram, mSpecLoc, 1, matSpe);
	glProgramUniform1f(renderingProgram, mShiLoc, matShi);
}

int main()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello, World!", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Could not create window!\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);

	glfwSwapInterval(1);


	init(window);

	//Print Statements
	printf("Vendor Info:%s\n", glGetString(GL_VENDOR));
	printf("Version: %s\n", glGetString(GL_VERSION));

	while (!glfwWindowShouldClose(window))
	{
		Display(window, glfwGetTime());
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
}
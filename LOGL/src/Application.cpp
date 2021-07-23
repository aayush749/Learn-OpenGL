#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utils.h"
#include "Torus.h"
#include "ImportedModel.h"

#define numVAOs 1
#define numVBOs 5

float cameraX, cameraY, cameraZ;
float cubeLocX, cubeLocY, cubeLocZ;
float torusLocX, torusLocY, torusLocZ;
float pyramidLocX, pyramidLocY, pyramidLocZ;
GLuint renderingProgram, renderingProgram1, renderingProgram2;	//the current shader program used to render (is chosen per frame), modify:: using renderingProgram 1 and 2 for shadow mapping purposes
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

float x = 0.0f;
float increment = 0.01f;
float tf; //time factor
// allocate variables used in display() function, so that they won’t need to be allocated during rendering
GLuint mvLoc, projLoc, nLoc;
int width, height;
float aspect;

float deltaX = 0.1f;

Torus myTorus(0.5f, 0.2f, 48);
ImportedModel pyramid("Models/Pyramid.obj");

//Vector of all shader programs
std::vector<GLuint> shaderPrograms;

//Some initilizations to test the lighting
//Location for shader uniform variables
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mAmbLoc, mDiffLoc, mSpecLoc, mShiLoc;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos, lightPosV; // light position as Vector3f, in both model and view space
float lightPos[3]; // light position as float array
// initial light location
glm::vec3 initialLightLoc = glm::vec3(-3.8f, 2.2f, 1.1f);
// white light properties
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 0.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// gold material for the pyramid
float* goldMatAmb = Utils::goldAmbient();
float* goldMatDif = Utils::goldDiffuse();
float* goldMatSpe = Utils::goldSpecular();
float goldMatShi = Utils::goldShininess();
// bronze material for the torus
float* bronzeMatAmb = Utils::bronzeAmbient();
float* bronzeMatDif = Utils::bronzeDiffuse();
float* bronzeMatSpe = Utils::bronzeSpecular();
float bronzeMatShi = Utils::bronzeShininess();

// variables used in display() for transfering light to shaders
float curAmb[4], curDif[4], curSpe[4], matAmb[4], matDif[4], matSpe[4];
float curShi, matShi;

// shadow-related variables 
int screenSizeX, screenSizeY;
GLuint shadowTex, shadowBuffer;
glm::mat4 lightVmatrix;
glm::mat4 lightPmatrix;
glm::mat4 shadowMVP1;
glm::mat4 shadowMVP2;
glm::mat4 b;

void InstallLights(GLuint shaderID, const glm::mat4& vMat);

unsigned int currentShaderIndex;

void ManageInput(GLFWwindow*, int, int, int, int);

void ResizeWindowCallback(GLFWwindow*, int, int);

void setupShadowBuffers(GLFWwindow*);

void setupVertices(void) 
{
	std::vector<glm::vec3> vert = myTorus.getVertices();
	std::vector<glm::vec2> tex = myTorus.getTexCoords();
	std::vector<glm::vec3> norm = myTorus.getNormals();
	std::vector<int> indices = myTorus.getIndices();
	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	int numVertices = myTorus.getNumVertices();
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
	
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // normal vectors
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * sizeof(float), &nvalues[0], GL_STATIC_DRAW);

	//indices for the torus
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

	//Pyramid
	vert.clear();
	norm.clear();
	pvalues.clear();
	nvalues.clear();
	
	vert = pyramid.getVertices();
	norm = pyramid.getNormals();
	
	numVertices = pyramid.getNumVertices();
	//Push the location coordinates, texture coordinates and normal vector values into corresponding vectors
	for (int i = 0; i < numVertices; i++)
	{
		pvalues.push_back(vert[i].x);
		pvalues.push_back(vert[i].y);
		pvalues.push_back(vert[i].z);
		nvalues.push_back(norm[i].x);
		nvalues.push_back(norm[i].y);
		nvalues.push_back(norm[i].z);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]); //pyramid's vert buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pvalues.size(), &pvalues[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]); //pyramid's normal buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * nvalues.size(), &nvalues[0], GL_STATIC_DRAW);

}

void passOne();
void passTwo();

void init(GLFWwindow* window) 
{
	//Compile and load all the shaders in the shaderPrograms vector
	std::vector<std::string> filenames;
	for (auto& file : std::filesystem::directory_iterator("shaders"))
	{
		filenames.push_back("shaders/" + file.path().stem().string() + ".shader");
	}
	
	for (int i = 1; i < filenames.size(); i += 2)
	{
		GLuint shaderProgram = Utils::CreateShaderProgram(filenames[i].c_str(), filenames[i - 1].c_str());
		shaderPrograms.push_back(shaderProgram);
	}

	currentShaderIndex = 0;
	//renderingProgram = shaderPrograms[currentShaderIndex];
	renderingProgram1 = Utils::CreateShaderProgram("shaders/shadow_pass1_v.shader", "shaders/shadow_pass1_f.shader");
	renderingProgram2 = Utils::CreateShaderProgram("shaders/shadow_pass2_v.shader", "shaders/shadow_pass2_f.shader");
	cameraX = 0.0f; cameraY = 0.2f; cameraZ = 6.0f;
	torusLocX = 1.6f; torusLocY = 0.0f; torusLocZ = -0.3f;
	pyramidLocX = -1.0f ; pyramidLocY = -0.5f; pyramidLocZ = 0.3f;
	
	setupVertices();
	setupShadowBuffers(window);
	//The Bias Matrix to convert from light space to texture space { from [-1..1] to [0..1] }
	b = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f);

	currentLightPos = initialLightLoc;
}

void setupShadowBuffers(GLFWwindow* window) {
	glfwGetFramebufferSize(window, &width, &height);
	screenSizeX = width;
	screenSizeY = height;
	// create the custom frame buffer
	glGenFramebuffers(1, &shadowBuffer);
	// create the shadow texture and configure it to hold depth information.
	// these steps are similar to those in Program 5.2
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		screenSizeX, screenSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

// The display() function manages the setup of the custom frame buffer and the shadow texture
// in preparation for pass 1 and pass 2 respectively. 
void Display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	
	// set up view and perspective matrix from the light point of view, for pass 1
	//currentLightPos.z -= 0.1f;
	lightVmatrix = glm::lookAt(currentLightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // vector from light to origin
	lightPmatrix = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);

	// make the custom frame buffer current, and associate it with the shadow texture
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	// disable drawing colors, but enable the depth computation
	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);

	passOne();

	glDisable(GL_POLYGON_OFFSET_FILL);

	// restore the default display buffer, and re-enable drawing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glDrawBuffer(GL_FRONT);

	aspect = (float)width / (float)height;
	pMat = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);

	passTwo();

}

void passOne()
{
	//rendering Program 1 includes the pass one vertex and fragment shaders
	glUseProgram(renderingProgram1);

	//the following blocks of code render the torus to establish the depth buffer from the light point of view

	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(torusLocX, torusLocY, torusLocZ));

	//slight rotation for viewability
	mMat = glm::rotate(mMat, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//we are drawing from the light's point of view, so we use the light's Projection and View matrices
	shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
	GLuint sLoc = glGetUniformLocation(renderingProgram1, "shadowMVP");
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	//we only need to set up torus vertices buffer - we don't need its textures or normals for pass one.

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);

	//repeat for the pyramid without clearing the depth buffer
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(pyramidLocX, pyramidLocY, pyramidLocZ));
	mMat = glm::rotate(mMat, glm::radians(40.0f), glm::vec3(1.0f, 1.0f, 0.0f));

	mMat = glm::translate(mMat, glm::vec3(-glm::radians((float)glfwGetTime()), 0.0f, 0.0f)); //glm::radians is used in transforming just to slow down the translation, else it would be really fast
	

	shadowMVP1 = lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, pyramid.getNumVertices());
}

void passTwo()
{
	glUseProgram(renderingProgram2);

	//draw the torus, this time we need to include lighting, materials, normals etc.
	//We also need to provide MVP transforms for BOTH camera space and light space

	mvLoc = glGetUniformLocation(renderingProgram2, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram2, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram2, "norm_matrix");
	GLuint sLoc = glGetUniformLocation(renderingProgram2, "shadowMVP2");

	//the torus is bronze
	curAmb[0] = bronzeMatAmb[0]; curAmb[1] = bronzeMatAmb[1]; curAmb[2] = bronzeMatAmb[2];
	curDif[0] = bronzeMatDif[0]; curDif[1] = bronzeMatDif[1]; curDif[2] = bronzeMatDif[2];
	curSpe[0] = bronzeMatSpe[0]; curSpe[1] = bronzeMatSpe[1]; curSpe[2] = bronzeMatSpe[2];
	curShi = bronzeMatShi;
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));

	InstallLights(renderingProgram2, vMat);
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(torusLocX, torusLocY, torusLocZ));
	// slight rotation for viewability
	mMat = glm::rotate(mMat, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// build the MV matrix for the torus from the camera’s point of view
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	// build the MVP matrix for the torus from the light’s point of view
	shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
	// put the MV and PROJ matrices into the corresponding uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));
	
	// set up torus vertices and normals buffers (and texture coordinates buffer if used)
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // torus vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // torus normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]); 
	glDrawElements(GL_TRIANGLES, myTorus.getNumIndices(), GL_UNSIGNED_INT, 0);

	//drawing the pyramids
	//the pyramid is gold
	curAmb[0] = goldMatAmb[0]; curAmb[1] = goldMatAmb[1]; curAmb[2] = goldMatAmb[2];
	curDif[0] = goldMatDif[0]; curDif[1] = goldMatDif[1]; curDif[2] = goldMatDif[2];
	curSpe[0] = goldMatSpe[0]; curSpe[1] = goldMatSpe[1]; curSpe[2] = goldMatSpe[2];
	curShi = goldMatShi;

	InstallLights(renderingProgram2, vMat);
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(pyramidLocX, pyramidLocY, pyramidLocZ));
	// slight rotation for viewability
	mMat = glm::rotate(mMat, glm::radians(40.0f), glm::vec3(1.0f, 1.0f, 0.0f));
	mMat = glm::translate(mMat, glm::vec3(-glm::radians((float)glfwGetTime()), 0.0f, 0.0f));	//glm::radians is used in transforming just to slow down the translation, else it would be really fast
	

	// build the MV matrix for the pyramid from the camera’s point of view
	mvMat = vMat * mMat;

	invTrMat = glm::transpose(glm::inverse(mvMat));
	// build the MVP matrix for the torus from the light’s point of view
	shadowMVP2 = b * lightPmatrix * lightVmatrix * mMat;
	// put the MV and PROJ matrices into the corresponding uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glDrawArrays(GL_TRIANGLES, 0, pyramid.getNumVertices());
}


void InstallLights(GLuint shaderID, const glm::mat4& vMat)
{
	glUseProgram(shaderID);
	//Convert light's position into viewspace and store it in a float array
	lightPosV = glm::vec3(vMat * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = lightPosV.x;
	lightPos[1] = lightPosV.y;
	lightPos[2] = lightPosV.z;

	//Get the location of the lights and the material fields in the shader
	globalAmbLoc = glGetUniformLocation(shaderID, "globalAmbient");
	ambLoc = glGetUniformLocation(shaderID, "light.ambient");
	diffLoc = glGetUniformLocation(shaderID, "light.diffuse");
	specLoc = glGetUniformLocation(shaderID, "light.specular");
	posLoc = glGetUniformLocation(shaderID, "light.position");
	mAmbLoc = glGetUniformLocation(shaderID, "material.ambient");
	mDiffLoc = glGetUniformLocation(shaderID, "material.diffuse");
	mSpecLoc = glGetUniformLocation(shaderID, "material.specular");
	mShiLoc = glGetUniformLocation(shaderID, "material.shininess");

	// set the uniform light and material values in the shader
	glProgramUniform4fv(shaderID, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(shaderID, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(shaderID, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(shaderID, specLoc, 1, lightSpecular);
	glProgramUniform3fv(shaderID, posLoc, 1, lightPos);
	glProgramUniform4fv(shaderID, mAmbLoc, 1, curAmb);
	glProgramUniform4fv(shaderID, mDiffLoc, 1, curDif);
	glProgramUniform4fv(shaderID, mSpecLoc, 1, curSpe);
	glProgramUniform1f(shaderID, mShiLoc, curShi);
}

int main()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello, World!", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Could not create window!\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);

	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, ResizeWindowCallback);

	init(window);

	//Print Statements
	printf("Vendor Info:%s\n", glGetString(GL_VENDOR));
	printf("Version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

	//Set input callback for retargeting the current shader
	glfwSetKeyCallback(window, Utils::ManageInput);
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		Display(window, glfwGetTime());
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
}

void ResizeWindowCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees
}
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
#include "Sphere.h"
#include "Application.h"

#define numVAOs 1
#define numVBOs 9

float cameraX, cameraY, cameraZ;
float cubeLocX, cubeLocY, cubeLocZ;
float sphereLocX, sphereLocY, sphereLocZ;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
float x = 0.0f;
float increment = 0.01f;
float tf; //time factor
// allocate variables used in display() function, so that they won’t need to be allocated during rendering
GLuint mvLoc, projLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat;
GLuint sunTexture, earthTexture, moonTexture;


GLuint mLoc, vLoc, tfLoc; //The uniform location for the model and view matrix and the time factor
Sphere sun(48), earth(48), moon(48);

void FillVertsAndIndices(std::vector<float>& pvalues, std::vector<float>& tvalues, std::vector<float>& nvalues)
{
	std::vector<int> ind = earth.GetIndices();
	const std::vector<glm::vec3>& earthVert = earth.GetVertices();
	const std::vector<glm::vec2>& eartTex = earth.GetTexCoords();
	const std::vector<glm::vec3>& earthNorm = earth.GetNormals();

	int numIndices = earth.GetNumIndices();
	for (int i = 0; i < numIndices; i++) {
		pvalues.push_back((earthVert[ind[i]]).x);
		pvalues.push_back((earthVert[ind[i]]).y);
		pvalues.push_back((earthVert[ind[i]]).z);
		tvalues.push_back((eartTex[ind[i]]).s);
		tvalues.push_back((eartTex[ind[i]]).t);
		nvalues.push_back((earthNorm[ind[i]]).x);
		nvalues.push_back((earthNorm[ind[i]]).y);
		nvalues.push_back((earthNorm[ind[i]]).z);
	}
}

void setupVertices(void)
{
	std::vector<float> pvalues; // vertex positions
	std::vector<float> tvalues; // texture coordinates
	std::vector<float> nvalues; // normal vectors


	//Sun
	FillVertsAndIndices(pvalues, tvalues, nvalues);
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(3, vbo);
	// put the vertices into buffer #0
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * sizeof(float), &pvalues[0], GL_STATIC_DRAW);
	// put the texture coordinates into buffer #1
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * sizeof(float), &tvalues[0], GL_STATIC_DRAW);
	// put the normals into buffer #2
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * sizeof(float), &nvalues[0], GL_STATIC_DRAW);

}

void init(GLFWwindow* window) 
{
	renderingProgram = Utils::CreateShaderProgram();
	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 10.0f;
	sphereLocX = 0.0f; sphereLocY = 0.0f; sphereLocZ = 0.0f;
	setupVertices();
	sunTexture = Utils::LoadTexture("Textures/Sun.jpg");
	earthTexture = Utils::LoadTexture("Textures/Earth.jpg");
	moonTexture = Utils::LoadTexture("Textures/Moon.jpg");
}

void Display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(renderingProgram);
	
	// get the uniform variables for the MV and projection matrices
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

	// build perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// pass on the view matrix and the model matrix for the pyramid (after applying some rotation and scale transforms)
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	
	std::stack<glm::mat4> mvStack;
	mvStack.push(vMat);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//Sun
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sphereLocX, sphereLocX, sphereLocZ));
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindTexture(GL_TEXTURE_2D, sunTexture);
	glActiveTexture(GL_TEXTURE0);
	
	glDrawArrays(GL_TRIANGLES, 0, sun.GetNumIndices());
	mvStack.pop();	//pop the rotation transform applied matrix

	//Earth
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(6.0f * sin((float)currentTime), 0.0f, 6.0f * cos((float)currentTime)));
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(-1.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	glDrawArrays(GL_TRIANGLES, 0, earth.GetNumIndices());

	mvStack.pop();	//pop the rotation transform applied matrix

	//Moon
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f * cos((float)currentTime), 2.0f * sin((float)currentTime)));
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float) currentTime, glm::vec3(0.0f, 1.0f, 1.0f));
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, .25f, .25f));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, moonTexture);

	glDrawArrays(GL_TRIANGLES, 0, moon.GetNumIndices());

	mvStack.pop(); mvStack.pop(); mvStack.pop(); mvStack.pop();
}

int main()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Hello, World!", NULL, NULL);
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
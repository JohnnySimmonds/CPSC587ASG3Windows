// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdlib>

#include <glm\glm\glm.hpp>
#include <glm\glm\gtc\matrix_transform.hpp>

// specify that we want the OpenGL core profile before including GLFW headers
#include "glad/glad.h"
#include <GLFW/glfw3.h>


#include "camera.h"
#include "Mass.h"
#include "Spring.h"

#define PI 3.14159265359

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
//string LoadSource(const char *filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void createJelloCubeSprings(vector<Spring*> *springs, vector<vec3> *cube, vec3 pos, float sizeOfCube, vector<unsigned int> *indices, vector<Mass*> *massObj);

vec2 mousePos;
bool leftmousePressed = false;
bool rightmousePressed = false;
bool play = false;
bool springChain = false;
int scene = 0;
bool initSpringSys = false;
bool isCloth = false;

bool isCube = false;
float sizeOfCube = 4.0f;
float sizeOfCloth = 20.0f;
int numSprings = 6;

camera* activeCamera;

GLFWwindow* window = 0;

mat4 winRatio = mat4(1.f);

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		play = !play;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
		activeCamera->moveCameraPositionForward();
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
		activeCamera->moveCameraPositionBackwards();
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
		activeCamera->moveCameraPositionLeft();
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
		activeCamera->moveCameraPositionRight();

	activeCamera->updateCameraView();

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{

		scene += 1;
		if (scene == 4)
			scene = 0;

		if (!springChain)
			springChain = true;
		else
			springChain = false;

		initSpringSys = false;
		//initSpringSys = false;
	}

}
bool mouseButtonOnePressed = false;
bool isFirstMousePosition = true;
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		mouseButtonOnePressed = true;
	}
	else
		mouseButtonOnePressed = false;

}
float lastX, lastY, yaw, pitch;
void mousePosCallback(GLFWwindow* window, double xPosition, double yPosition)
{

	if (isFirstMousePosition)
	{
		lastX = xPosition;
		lastY = yPosition;
		isFirstMousePosition = false;
	}
	if (mouseButtonOnePressed)
	{
		float xOffset = xPosition - lastX;
		float yOffset = lastY - yPosition;
		lastX = xPosition;
		lastY = yPosition;

		float sensitivity = 0.5f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		vec3 target = vec3(0.0f, 0.0f, -3.0f);
		target.x = cos(radians(yaw)) * cos(radians(pitch));
		target.y = sin(radians(pitch));
		target.z = sin(radians(yaw)) * cos(radians(pitch));

		activeCamera->updateCameraTarget(normalize(target));
		activeCamera->updateCameraView();
	}


}
float fov = 45.0f;
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 90.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 90.0f)
		fov = 90.0f;
}
void resizeCallback(GLFWwindow* window, int width, int height)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, width, height);

	float minDim = float(std::min(width, height));

	winRatio[0][0] = minDim/float(width);
	winRatio[1][1] = minDim/float(height);
}





//==========================================================================
// TUTORIAL STUFF


//vec2 and vec3 are part of the glm math library. 
//Include in your own project by putting the glm directory in your project, 
//and including glm/glm.hpp as I have at the top of the file.
//"using namespace glm;" will allow you to avoid writing everyting as glm::vec2

struct VertexBuffers{
	enum{ VERTICES=0, NORMALS, INDICES, COUNT};

	GLuint id[COUNT];
};
void deleteStuff(GLuint vao, VertexBuffers vbo, GLuint program);
//Describe the setup of the Vertex Array Object
bool initVAO(GLuint vao, const VertexBuffers& vbo)
{
	glBindVertexArray(vao);		//Set the active Vertex Array

	glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
	glBindBuffer( GL_ARRAY_BUFFER, vbo.id[VertexBuffers::VERTICES] );		//Set the active Vertex Buffer
	glVertexAttribPointer(
		0,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::NORMALS]);
	glVertexAttribPointer(
		1,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[VertexBuffers::INDICES]);

	return !CheckGLErrors("initVAO");		//Check for errors in initialize
}


//Loads buffers with data
bool loadBuffer(const VertexBuffers& vbo, 
				const vector<vec3>& points, 
				const vector<vec3> normals, 
				const vector<unsigned int>& indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::VERTICES]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*points.size(),		//Size of data in array (in bytes)
		&points[0],						//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW					//GL_DYNAMIC_DRAW if you're changing the data often
										//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::NORMALS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*normals.size(),	//Size of data in array (in bytes)
		&normals[0],					//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW					//GL_DYNAMIC_DRAW if you're changing the data often
										//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[VertexBuffers::INDICES]);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int)*indices.size(),
		&indices[0],
		GL_STATIC_DRAW
		);

	return !CheckGLErrors("loadBuffer");	
}

//Compile and link shaders, storing the program ID in shader array
GLuint initShader(string vertexName, string fragmentName)
{	
	string vertexSource = LoadSource(vertexName);		//Put vertex file text into string
	string fragmentSource = LoadSource(fragmentName);		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	return LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array
}

//Initialization
void initGL()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
}

bool loadUniforms(GLuint program, mat4 perspective, mat4 modelview)
{
	glUseProgram(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "modelviewMatrix"),
						1,
						false,
						&modelview[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(program, "perspectiveMatrix"),
						1,
						false,
						&perspective[0][0]);

	return !CheckGLErrors("loadUniforms");
}

//Draws buffers to screen
void renderLine(GLuint vao, int startElement, int numElements, GLuint program, VertexBuffers vbo, vector<vec3> points, vector<vec3>normals, vector<unsigned int> indices)
{
	
	
	glBindVertexArray(vao);		//Use the LINES vertex array
	glUseProgram(program);
	
	loadBuffer(vbo, points, normals, indices);
	
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.25f);
	
	glDrawElements(
			GL_LINES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);
	glDisable(GL_LINE_SMOOTH);
 
	CheckGLErrors("render");
	glUseProgram(0);
	glBindVertexArray(0);


}
void renderPoints(GLuint vao, int startElement, int numElements, GLuint program, VertexBuffers vbo, vector<vec3> points, vector<vec3>normals, vector<unsigned int> indices)
{
	
	
	glBindVertexArray(vao);		//Use the LINES vertex array
	glUseProgram(program);
	
	loadBuffer(vbo, points, normals, indices);
	
	//glEnable(GL_LINE_SMOOTH);
	glPointSize(5.0f);
	
	glDrawElements(
			GL_POINTS,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);
	//glDisable(GL_LINE_SMOOTH);
 
	CheckGLErrors("render");
	glUseProgram(0);
	glBindVertexArray(0);


}
void renderPlane(GLuint vao, int startElement, int numElements, GLuint program, VertexBuffers vbo, vector<vec3> points, vector<vec3>normals, vector<unsigned int> indices)
{
	
	
	glBindVertexArray(vao);		//Use the LINES vertex array
	glUseProgram(program);
	
	loadBuffer(vbo, points, normals, indices);
	
	//glEnable(GL_LINE_SMOOTH);
	//glPointSize(10.0f);
	
	glDrawElements(
			GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);
	//glDisable(GL_LINE_SMOOTH);
 
	CheckGLErrors("render");
	glUseProgram(0);
	glBindVertexArray(0);


}

void generateSquare(vector<vec3>* vertices, vector<vec3>* normals, 
					vector<unsigned int>* indices, float width)
{
	vertices->push_back(vec3(-width*0.5f, -width*0.5f, 0.f));
	vertices->push_back(vec3(width*0.5f, -width*0.5f, 0.f));
	vertices->push_back(vec3(width*0.5f, width*0.5f, 0.f));
	vertices->push_back(vec3(-width*0.5f, width*0.5f, 0.f));

	normals->push_back(vec3(0.f, 0.f, 1.f));
	normals->push_back(vec3(0.f, 0.f, 1.f));
	normals->push_back(vec3(0.f, 0.f, 1.f));
	normals->push_back(vec3(0.f, 0.f, 1.f));

	//First triangle
	indices->push_back(0);
	indices->push_back(1);
	indices->push_back(2);
	//Second triangle
	indices->push_back(2);
	indices->push_back(3);
	indices->push_back(0);
}

GLFWwindow* createGLFWWindow()
{
	// initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
        return NULL;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 1024, "OpenGL Example", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return NULL;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwMakeContextCurrent(window);

    return window;
}



// ==========================================================================
// PROGRAM ENTRY POINT
void printVec3(vec3 toPrint)
{
	cout << "X: " << toPrint.x << endl;
	cout << "Y: " << toPrint.y << endl;
	cout << "Z: " << toPrint.z << endl;
}


void createSpringChain(vector<Spring*> *springs, int numSprings)
{

	Spring *springRoot = new Spring(vec3(0.0f, 2.0f, 0.0f), vec3(0.0f, -2.0f, 0.0f), true, false);
	springs->push_back(springRoot);


	float setXMassB = 4.0f;
	float setYMassB = 2.0f;
	bool zigZag = false;
	Spring *springPrev = springRoot;
	for(int i = 0; i < numSprings-1; i++)
	{
		Spring *springNew = new Spring(springPrev->getMassB()->getPosition(), vec3(setXMassB , setYMassB, 0.0f), false, false);
		springNew->setMassA(springPrev->getMassB());
		springs->push_back(springNew);
		springPrev = springNew;
		setXMassB += 2.0f;
		if(zigZag)
		{
			setYMassB += 1.0f;
			zigZag = false;
		}
		else
		{
			setYMassB -= 1.0f;
			zigZag = true;
		}
	}

}
 
void createCube(vector<Spring*> *springs)
{
	//Spring *springRoot = new Spring(vec3(-1.0f,1.0f,1.0f), vec3(-1.0f,-1.0f,1.0f), false,false);
	//springs->push_back(springRoot);
	
	Spring *springRoot = new Spring(vec3(1.0f,1.0f,1.0f), vec3(-1.0f,1.0f,1.0f), false,false);
	springs->push_back(springRoot);
	Spring *springTest = new Spring(springRoot->getMassA()->getPosition(), vec3(1.0f,-1.0f,1.0f), false,false);
	springTest->setMassA(springRoot->getMassA());
	springs->push_back(springTest);
	Spring *springTest2 = new Spring(vec3(-1.0f,-1.0f,1.0f), springTest->getMassB()->getPosition(), false,false);
	springTest2->setMassB(springTest->getMassB());
	springs->push_back(springTest2);
	Spring *springTest3 = new Spring(springTest2->getMassA()->getPosition(), springRoot->getMassB()->getPosition(), false,false);
	springTest3->setMassA(springTest2->getMassA());
	springTest3->setMassB(springRoot->getMassB());
	springs->push_back(springTest3);
	
	Spring *springCross = new Spring (springRoot->getMassA()->getPosition(), springTest2->getMassA()->getPosition(), false, false);
	springCross->setMassA(springRoot->getMassA());
	springCross->setMassB(springTest2->getMassA());
	springs->push_back(springCross);
	
	Spring *springCross2 = new Spring (springRoot->getMassB()->getPosition(), springTest2->getMassB()->getPosition(), false, false);
	springCross2->setMassA(springRoot->getMassB());
	springCross2->setMassB(springTest2->getMassB());
	springs->push_back(springCross2);
	
} 


void setupDraw(vector<vec3> *mass, vector<unsigned int> *massInds, vector<vec3> *massColor, 
vector<vec3> *spring, vector<unsigned int> *springsInd, vector<vec3> *springColor, Spring *springOne)
{
		vec3 posA = springOne->getMassB()->getPosition();
		
		spring->push_back(springOne->getMassA()->getPosition());
		springsInd->push_back(springsInd->size());
		springColor->push_back(vec3(posA.x/sizeOfCube,posA.y/sizeOfCube,posA.z/sizeOfCube));



		mass->push_back(springOne->getMassA()->getPosition());
		massInds->push_back(massInds->size());
		massColor->push_back(vec3(posA.x/sizeOfCube,posA.y/sizeOfCube,posA.z/sizeOfCube));
		
		
		
		springOne->getMassA()->setIsDrawn(true);

		vec3 posB = springOne->getMassB()->getPosition();
		spring->push_back(springOne->getMassB()->getPosition());
		springsInd->push_back(springsInd->size());

		springColor->push_back(vec3(posB.x/sizeOfCube,posB.y/sizeOfCube,posB.z/sizeOfCube));
		
		
		
		mass->push_back(springOne->getMassB()->getPosition());
		massInds->push_back(massInds->size());

		massColor->push_back(vec3(posB.x/sizeOfCube,posB.y/sizeOfCube,posB.z/sizeOfCube));
		springOne->getMassB()->setIsDrawn(true);

}


void createSpringBox(vector<vec3> box, vector<unsigned int> indices, vector<Spring*> *springs, vector<Mass*> *masses)
{
	//vector<Mass*> masses;
	for(int i = 0; i < box.size(); i++)
	{
		Mass *massNew = new Mass();
		massNew->setPosition(box[i]);
		masses->push_back(massNew);
	}
	int ind1, ind2;
	for(unsigned int i = 0; i < indices.size(); i+=2)
	{
		ind1 = indices[i];
		ind2 = indices[i+1];
		
		Spring* springNew = new Spring(box[ind1], box[ind2], false, false);
		springNew->setMassA(masses->at(ind1));
		springNew->setMassB(masses->at(ind2));
		springs->push_back(springNew);
	}
}

bool isInBoundsCloth(vec3 pos, float sizeOfCloth)
{
	if(pos.y >= 0 && pos.y < sizeOfCloth)
		if(pos.x >= 0 && pos.x < sizeOfCloth)
			return true;
				
	return false;
}
void createClothSprings(vector<Spring*> *springs, vector<vec3> *cube, vec3 pos, float sizeOfCloth, vector<unsigned int> *indices, vector<Mass*> *massObj, float z)
{
	vec3 newBPos;
	unsigned int ind1, ind2;

	for(float y = -1; y <= 1.0f; y++)
	{
		for(float x = -1; x <= 1.0f; x++)
		{
			if(x==0 && y == 0)
			{
				continue;
			}
			if(isInBoundsCloth(pos+vec3(x,y,z), sizeOfCloth))
			{
				newBPos = pos+vec3(x,y,0.0f);
				ind1 = (unsigned int)(pos.x+(pos.y*sizeOfCloth));
				ind2 = (unsigned int)(newBPos.x+(newBPos.y*sizeOfCloth));
				Spring* springNew = new Spring(pos, newBPos, false, false);
		
				springNew->setMassA(massObj->at(ind1));
				springNew->setMassB(massObj->at(ind2));
				springNew->getMassA()->setPosition(pos);
				springNew->getMassB()->setPosition(newBPos);

				springs->push_back(springNew);
				
				
				indices->push_back(ind1);
				indices->push_back(ind2);

			}
		}
				
	}
}
void createCloth(vector<vec3> *cloth, vector<vec3> *normals, vector<unsigned int> *indices, float sizeOfCloth, vector<Spring*> *springs, vector<Mass*> *masses)
{
	for(int i = 0; i < sizeOfCloth*sizeOfCloth; i++)
	{
		Mass *massNew = new Mass();
		masses->push_back(massNew);
	}
	
	float z = 0.0f;
	
	unsigned int currInd;

	float scale = 1.0f;

	for(float y = 0; y < sizeOfCloth; y++)
	{
		for(float x = 0; x < sizeOfCloth; x++)
		{
			currInd = (unsigned int)(x+(y*sizeOfCloth));
			cloth->push_back(vec3(x*scale,y*scale,z*scale));
			normals->push_back(vec3(x/sizeOfCloth,y/sizeOfCloth,z/sizeOfCloth));
			indices->push_back(currInd);

			createClothSprings(springs, cloth, vec3(x,y,z), sizeOfCloth, indices, masses, z);
		}
		
	}
	for(int i = 0; i < springs->size(); i+=2)
	{
		if(springs->at(i)->getMassA()->getPosition().y == sizeOfCloth-1.0f)
			springs->at(i)->getMassA()->setIsFixed(!springs->at(i)->getMassA()->getIsFixed());
	}

}


void createJelloCube(vector<vec3> *cube, vector<vec3> *normals, vector<unsigned int> *indices, float sizeOfCube, vector<Spring*> *springs, vector<Mass*> *masses)
{
	
	for(int i = 0; i < sizeOfCube*sizeOfCube*sizeOfCube; i++)
	{
		Mass *massNew = new Mass();
		masses->push_back(massNew);
	}
	
	unsigned int currInd;
	//float sizeOfCube = 3.0f;
	float scale = 0.5f;
	for(float z = 0; z < sizeOfCube; z++)
	{
		for(float y = 0; y < sizeOfCube; y++)
		{
			for(float x = 0; x < sizeOfCube; x++)
			{
				currInd = (unsigned int)(x+(y*sizeOfCube)+(z*sizeOfCube*sizeOfCube));
				cube->push_back(vec3(x*scale,y*scale,z*scale));
				normals->push_back(vec3(x/sizeOfCube,y/sizeOfCube,z/sizeOfCube));
				indices->push_back(currInd);
				
				
				createJelloCubeSprings(springs, cube, vec3(x,y,z), sizeOfCube, indices, masses);
			}
			
		}
	}

	

}

bool isInBounds(vec3 pos, float sizeOfCube)
{
	if(pos.z >= 0 && pos.z < sizeOfCube)
		if(pos.y >= 0 && pos.y < sizeOfCube)
			if(pos.x >= 0 && pos.x < sizeOfCube)
				return true;
				
	return false;
}

void createJelloCubeSprings(vector<Spring*> *springs, vector<vec3> *cube, vec3 pos, float sizeOfCube, vector<unsigned int> *indices, vector<Mass*> *massObj)
{
	vec3 newBPos;
	unsigned int ind1, ind2;
	
	for(float z = -1; z <= 1.0f; z++)
	{
		for(float y = -1; y <= 1.0f; y++)
		{
			for(float x = -1; x <= 1.0f; x++)
			{
				if(x==0 && y == 0 && z==0)
				{
					continue;
				}
				if(isInBounds(pos+vec3(x,y,z), sizeOfCube))
				{
					newBPos = pos+vec3(x,y,z);
					ind1 = (unsigned int)(pos.x+(pos.y*sizeOfCube)+(pos.z*sizeOfCube*sizeOfCube));
					ind2 = (unsigned int)(newBPos.x+(newBPos.y*sizeOfCube)+(newBPos.z*sizeOfCube*sizeOfCube));
					Spring* springNew = new Spring(pos, newBPos, false, false);
			
					springNew->setMassA(massObj->at(ind1));
					springNew->setMassB(massObj->at(ind2));
					springNew->getMassA()->setPosition(pos);
					springNew->getMassB()->setPosition(newBPos);
					

					
					springs->push_back(springNew);
					
					indices->push_back(ind1);
					indices->push_back(ind2);

				}
			}
					
		}
	}
	
}

void createBox(vector<vec3> *box, vector<vec3> *normals, vector<unsigned int> *indices)
{
	box->push_back(vec3(1.0f, 1.0f, 1.f)); //0
	box->push_back(vec3(1.0f, -1.0f, 1.f)); //1
	box->push_back(vec3(-1.0f, -1.0f, 1.f)); //2
	box->push_back(vec3(-1.0f, 1.0f, 1.f)); //3
	box->push_back(vec3(-1.0f, -1.0f, -1.f)); //4
	box->push_back(vec3(-1.0f, 1.0f, -1.f)); //5
	box->push_back(vec3(1.0f, 1.0f, -1.f)); //6
	box->push_back(vec3(1.0f, -1.0f, -1.f)); //7
	
	indices->push_back(0);
	indices->push_back(1);
	
	indices->push_back(0);
	indices->push_back(6);
	
	indices->push_back(0);
	indices->push_back(7);
	
	indices->push_back(0);
	indices->push_back(3);
	
	indices->push_back(0);
	indices->push_back(5);
	
	indices->push_back(0);
	indices->push_back(2);
	
	indices->push_back(1);
	indices->push_back(2);
	
	indices->push_back(1);
	indices->push_back(4);
	
	indices->push_back(1);
	indices->push_back(7);
	
	indices->push_back(1);
	indices->push_back(6);
	
	indices->push_back(1);
	indices->push_back(3);
	
	indices->push_back(2);
	indices->push_back(3);
	
	indices->push_back(2);
	indices->push_back(4);
	
	indices->push_back(2);
	indices->push_back(5);
	
	indices->push_back(2);
	indices->push_back(7);
	
	indices->push_back(3);
	indices->push_back(5);
	
	indices->push_back(3);
	indices->push_back(4);
	
	indices->push_back(4);
	indices->push_back(5);
	
	indices->push_back(4);
	indices->push_back(6);
	
	indices->push_back(4);
	indices->push_back(7);
	
	indices->push_back(5);
	indices->push_back(7);
	
	indices->push_back(5);
	indices->push_back(6);
	
	indices->push_back(6);
	indices->push_back(7);
	
	

	/*colors of each point*/

	normals->push_back(vec3(1.0f, 0.0f, 0.f));
	normals->push_back(vec3(0.0f, 1.0f, 0.f));
	normals->push_back(vec3(0.0f, 0.0f, 1.f));
	normals->push_back(vec3(1.0f, 1.0f, 0.f));

	normals->push_back(vec3(1.0f, 0.0f, 1.f));
	normals->push_back(vec3(0.0f, 1.0f, 1.f));
	normals->push_back(vec3(0.5f, 0.5f, 0.5f));
	normals->push_back(vec3(0.0f, 0.0f, 0.f));
}
void createPlane(vector<vec3> *plane, vector<vec3> *planeColors, vector<unsigned int> *planeInds)
{
	plane->push_back(vec3(-10.0f, -10.0f, -10.0f));
	plane->push_back(vec3(-10.0f, -10.0f, 10.0f));
	plane->push_back(vec3(10.0f, -10.0f, -10.0f));
	plane->push_back(vec3(10.0f, -10.0f, 10.0f));
	
	planeInds->push_back(3);
	planeInds->push_back(0);
	planeInds->push_back(2);
	
	planeInds->push_back(0);
	planeInds->push_back(1);
	planeInds->push_back(3);
	
	planeColors->push_back(vec3(1.0f, 1.0f, 1.0f));
	planeColors->push_back(vec3(1.0f, 1.0f, 1.0f));
	planeColors->push_back(vec3(1.0f, 1.0f, 1.0f));
	planeColors->push_back(vec3(1.0f, 1.0f, 1.0f));
	
}
int main(int argc, char *argv[])
{   
    window = createGLFWWindow();
    if(window == NULL)
    	return -1;

    //Initialize glad
    if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}

    // query and print out information about our OpenGL environment
    QueryGLVersion();

	initGL();

	//Initialize shader
	GLuint program = initShader("Shaders/vertex.glsl", "Shaders/fragment.glsl");

	GLuint vao;
	VertexBuffers vbo;

	//Generate object ids
	glGenVertexArrays(1, &vao);
	glGenBuffers(VertexBuffers::COUNT, vbo.id);

	initVAO(vao, vbo);

	//Geometry information
	vector<vec3> points, normals;
	vector<unsigned int> indices;

	generateSquare(&points, &normals, &indices, 1.f);

	vector<Spring*> multipleSprings;

	camera cam;
	activeCamera = &cam;
	mat4 perspectiveMatrix = perspective(radians(fov), 1.f, 0.1f, 300.f);
	mat4 V;
	//float fovy, float aspect, float zNear, float zFar
	//mat4 perspectiveMatrix = perspective(radians(80.f), 1.f, 0.1f, 400.f);



	float dt = 0.01f;
	float time = 0.0f;

	vec3 gravity = vec3(0.0f, -2.81f, 0.0f);
    vec3 force;
    mat4 moveObj;
    
    vector<vec3> masses;
    vector<unsigned int> massInd;
    vector<vec3> colorMass;

    
    vector<vec3> springs;
    vector<unsigned int> springInd;
    vector<vec3> colorSpring;
    
    
    vector<vec3> massFixed;
    vector<unsigned int> massFixedInd;
    vector<vec3> colorMassFixed;
    
    vector<vec3> box;
    vector<vec3> boxColor;
    vector<unsigned int> boxInds;

    vector<vec3> cloth;
    vector<vec3> clothColor;
    vector<unsigned int> clothInds;
    
    vector<Mass*> massObjs;
    float minHeight = -20.0f;
	float extraTime = 0.0f;

    // run an event-triggered main loop
    vector<vec3> cube;
    vector<vec3> cubeColors;
    vector<unsigned int> cubeInds;
    
	vector<vec3> plane;
	vector<vec3> planeColor;
	vector<unsigned int> planeInd;
	
	createPlane(&plane, &planeColor, &planeInd);

	float kWave = 6.0f;
	float kInc = 1.0f;
	vec3 airForce = vec3(0.0f,0.0f,0.0f);
	//createJelloCubeSprings(&multipleSprings, cube, vec3(0.0f,0.0f,0.0f), sizeOfCube);
    while (!glfwWindowShouldClose(window))
    {
		if(!initSpringSys)
		{
			play = false;
			masses.clear();
			massInd.clear();
			colorMass.clear();
	
				
			springs.clear();
			springInd.clear();
			colorSpring.clear();
			
			
			massFixed.clear();
			massFixedInd.clear();
			colorMassFixed.clear();
			
			multipleSprings.clear();	
			massObjs.clear();	
			
			cloth.clear();
			clothInds.clear();
			isCube = false;
			isCloth = false;
			switch(scene)
			{
				case 0:
					createSpringChain(&multipleSprings, 1);
				break;
				
				case 1:
					createSpringChain(&multipleSprings, numSprings);
				break;
				
				case 2:
				{
					createJelloCube(&cube, &cubeColors, &cubeInds, sizeOfCube, &multipleSprings, &massObjs);
					isCube = true;
					for(int i = 0; i < multipleSprings.size(); i++)
					{
						multipleSprings[i]->getMassA()->setIsCube(true);
						multipleSprings[i]->getMassB()->setIsCube(true);
					}

				}
				break;
				case 3:
				{
					isCloth = true;
					createCloth(&cloth, &clothColor, &clothInds, sizeOfCloth, &multipleSprings, &massObjs);

				}
				break;
				
				default:
				break;
				
			}

			initSpringSys = true;
			for(int i = 0; i < multipleSprings.size(); i++)
			{
				setupDraw(&masses, &massInd, &colorMass, &springs, &springInd, &colorSpring, multipleSprings[i]);

			}
		}

		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//Clear color and depth buffers (Haven't covered yet)
		
		dt = 0.004f;
		dt += extraTime;

		V = cam.getCameraView();
		perspectiveMatrix = perspective(radians(fov), 1.f, 0.1f, 300.f);

		if(play)
			{
				float timeStep = 1.0f / 1000.0f;
				
				
				while (dt >= timeStep)
					{
						masses.clear();
						massInd.clear();
						colorMass.clear();

						
						springs.clear();
						springInd.clear();
						colorSpring.clear();
						
	
						

						for( int i = 0; i < multipleSprings.size(); i++)
						{
							multipleSprings[i]->zeroForce();

						}
						for(int j = 0; j < multipleSprings.size(); j++)
						{
							multipleSprings[j]->applyForce(dt);
						}
						for(int k = 0; k < multipleSprings.size(); k++)
						{
	

							
							if(isCloth)
							{
								
								airForce = vec3(0.0f,0.0f,-3.0f);
								multipleSprings[k]->setStiffness(kWave);
		
							}
								//test = vec3(5.0f, 0.0f, -5.0f);
							if(!multipleSprings[k]->getMassA()->getCalced())
							{
								
								multipleSprings[k]->getMassA()->setForce((multipleSprings[k]->getMassA()->getForce() + gravity*multipleSprings[k]->getMassA()->getMass()+airForce));
								multipleSprings[k]->getMassA()->resolveForces(dt);
								multipleSprings[k]->getMassA()->setCalced(true);	
							}
							if(!multipleSprings[k]->getMassB()->getCalced())
							{
								multipleSprings[k]->getMassB()->setForce((multipleSprings[k]->getMassB()->getForce() + gravity*multipleSprings[k]->getMassB()->getMass()+ airForce));
								multipleSprings[k]->getMassB()->resolveForces(dt);
								multipleSprings[k]->getMassB()->setCalced(true);
							}
								
						}
						if(kWave == 200.0f || kWave == 5.0f)
						{
							kInc *= -1.0f;

						}
						
						kWave += kInc;
						for( int i = 0; i < multipleSprings.size(); i++)
						{
							if(isCloth)
							{
								
								if(!multipleSprings[i]->getMassA()->getIsFixed())
								{
									multipleSprings[i]->getMassA()->setPosition(multipleSprings[i]->getMassA()->getNewPos());
									multipleSprings[i]->getMassA()->setNewVel();
								}
								if(!multipleSprings[i]->getMassB()->getIsFixed())
								{
									multipleSprings[i]->getMassB()->setPosition(multipleSprings[i]->getMassB()->getNewPos());
									multipleSprings[i]->getMassB()->setNewVel();
								}
							}
							else
							{
								multipleSprings[i]->getMassB()->setPosition(multipleSprings[i]->getMassB()->getNewPos());
								multipleSprings[i]->getMassA()->setPosition(multipleSprings[i]->getMassA()->getNewPos());
								multipleSprings[i]->getMassA()->setNewVel();
								multipleSprings[i]->getMassB()->setNewVel();
							}
						
							multipleSprings[i]->unCalced(); 
						}
					
						extraTime = dt;
						dt -= timeStep;
					}
			}
			for(int i = 0; i < multipleSprings.size(); i++)
			{
				setupDraw(&masses, &massInd, &colorMass, &springs, &springInd, &colorSpring, multipleSprings[i]);
			
			}

			loadUniforms(program, winRatio*perspectiveMatrix*V, mat4(1.0f));
			renderLine(vao, 0, springInd.size(), program, vbo, springs, colorSpring, springInd); 

			loadUniforms(program, winRatio*perspectiveMatrix*V, mat4(1.0f));
			renderPoints(vao, 0, massInd.size(), program, vbo, masses, colorMass, massInd);
			
			if(isCube)
			{
				loadUniforms(program, winRatio*perspectiveMatrix*V, mat4(1.0f));
				renderPlane(vao, 0, planeInd.size(), program, vbo, plane, planeColor, planeInd);
			}
			

			glfwSwapBuffers(window);// scene is rendered to the back buffer, so swap to front for display
			glfwPollEvents(); // sleep until next event before drawing again

			masses.clear();
			massInd.clear();
			colorMass.clear();

			
			springs.clear();
			springInd.clear();
			colorSpring.clear();


		
	}

	// clean up allocated resources before exit
	
	deleteStuff(vao, vbo, program);
	
	return 0;
}

/*
        */

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void deleteStuff(GLuint vao, VertexBuffers vbo, GLuint program)
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(VertexBuffers::COUNT, vbo.id);
	glDeleteProgram(program);
	

	glfwDestroyWindow(window);
	glfwTerminate();
}
void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string

string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}/*
string LoadSource(const char *filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);

	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		return "";
	}

	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}*/

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();

    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}


// ==========================================================================

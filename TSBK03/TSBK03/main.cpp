// TSBK03, Teknik för avancerade datorspel
// Mycket av nedanstående kod är tagen från labbfiler i TSBK07 och TSBK03, skrivna av Ingemar Ragnemalm.
// Lorem ipsum, etc.

// Nedanstående bör lösas. Så småningom.
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include <iostream>

#ifdef _WIN32
// MS
//#include <windows.h>
#include <gl/glew.h>
// Glew initialization... thing.
GLenum err;
#else
// Linux
#include <gl/gl.h>
#endif
#include <gl/freeglut.h>

#include "common/VectorUtils3.h"
#include "common/GL_utilities.h"
#include "common/loadobj.h"
#include "common/zpr.h"

#define PI 3.14159265358979323846

// initial width and heights
#define W 512
#define H 512

#define NUM_LIGHTS 4

#define FILTER_PASSES 10

void OnTimer(int value);

mat4 projectionMatrix;
mat4 viewMatrix;


GLfloat square[] = {
	-1, -1, 0,
	-1, 1, 0,
	1, 1, 0,
	1, -1, 0 };
GLfloat squareTexCoord[] = {
	0, 0,
	0, 1,
	1, 1,
	1, 0 };
GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };

Model* squareModel;

//----------------------Globals-------------------------------------------------
vec3 cam = vec3(0.0, 0.0, 0.0);
vec3 point = vec3(0.0, 0.0, 0.0);
Model *model1;
FBOstruct *fbo1, *fbo2, *fbo_orig;
GLuint phongshader = 0, plaintextureshader = 0, lowpasshader = 0, thresholdshader = 0, bloomshader = 0;
GLfloat t = 0;

//-------------------------------------------------------------------------------------

// -------------------Light source(s)-------------------
const int numberOfLightSources = 4;	// Change here and in the fragment shader when setting the number of light sources.
Point3D lightSourcesColorsArr[] = { { 1.0f, 1.0f, 1.0f },	// White light.
{ 0.0f, 0.0f, 0.0f },	// No light.
{ 0.0f, 0.0f, 0.0f },	// No light.
{ 0.0f, 0.0f, 0.0f } };	// No light.
GLfloat specularExponent[] = { 150.0, 0.0, 0.0, 0.0 };
GLint isDirectional[] = { 1, 0, 0, 0 };
Point3D lightSourcesDirectionsPositions[] = { { 0.58f, 0.58f, 0.58f },	// White light, directional.
{ 0.0f, 0.0f, 0.0f },			// No light.
{ 0.0f, 0.0f, 0.0f },			// No light.
{ 0.0f, 0.0f, 0.0f } };			// No light.
// -----------------------------------------------------

mat4 bunnyTrans;

void init(void)
{
	#ifdef _WIN32
	// Nedan verkar krävas för att GLEW inte ska gnälla. Bah.
	glewExperimental = TRUE;
	err = glewInit();
	#endif
	
	dumpInfo();  // shader info

	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// Load and compile shaders
	plaintextureshader = loadShaders("shaders/plaintextureshader.vert", "shaders/plaintextureshader.frag");  // puts texture on teapot
	phongshader = loadShaders("shaders/phong.vert", "shaders/phong.frag");  // renders with light (used for initial renderin of teapot)
	lowpasshader = loadShaders("shaders/lowpass.vert", "shaders/lowpass.frag");  // filters the values in texture and outputs
	thresholdshader = loadShaders("shaders/threshold.vert", "shaders/threshold.frag");
	bloomshader = loadShaders("shaders/bloom.vert", "shaders/bloom.frag");

	printError("init shader");

	fbo1 = initFBO(W, H, 0);
	fbo2 = initFBO(W, H, 0);
	fbo_orig = initFBO(W, H, 0);

	// load the model
	// model1 = LoadModelPlus("teapot.obj");
	// model1 = LoadModelPlus("objects/stanford-bunny.obj");
	model1 = LoadModelPlus("objects/bunnyplus.obj");

	// Scale and place bunny since it is too small
	bunnyTrans = T(0, 0, 0);
	bunnyTrans = Mult(S(8, 8, 8), bunnyTrans);

	// Lighting stuff.
	glUseProgram(phongshader);
	glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesDirPosArr"), numberOfLightSources, &lightSourcesDirectionsPositions[0].x);
	glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesColorArr"), numberOfLightSources, &lightSourcesColorsArr[0].x);
	glUniform1fv(glGetUniformLocation(phongshader, "specularExponent"), numberOfLightSources, specularExponent);
	glUniform1iv(glGetUniformLocation(phongshader, "isDirectional"), numberOfLightSources, isDirectional);
	glActiveTexture(GL_TEXTURE0);
	// -----------------------------------

	squareModel = LoadDataToModel(
		square, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);

	cam = SetVector(0, 5, 15);
	point = SetVector(0, 1, 0);

	glutTimerFunc(5, &OnTimer, 0);

	zprInit(&viewMatrix, cam, point);
	//viewMatrix = lookAtv(cam, point, vec3(0.0, 1.0, 0.0));
}

void OnTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(5, &OnTimer, value);
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
	// This function is called whenever it is time to render
	//  a new frame; due to the idle()-function below, this
	//  function will get called several times per second

	// render to fbo1!
	useFBO(fbo_orig, 0L, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1, 0.1, 0.3, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(phongshader);
	
	// Increase "time"
	t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);


	glUniformMatrix4fv(glGetUniformLocation(phongshader, "VTPMatrix"), 1, GL_TRUE, projectionMatrix.m);
	//glUniformMatrix4fv(glGetUniformLocation(phongshader, "modelviewMatrix"), 1, GL_TRUE, bunnyTrans.m);
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "WTVMatrix"), 1, GL_TRUE, viewMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "MTWMatrix"), 1, GL_TRUE, bunnyTrans.m);
	glUniform3fv(glGetUniformLocation(phongshader, "camPos"), 1, &cam.x);
	glUniform1i(glGetUniformLocation(phongshader, "texUnit"), 0);

	// Enable Z-buffering
	glEnable(GL_DEPTH_TEST);
	// Enable backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	DrawModel(model1, phongshader, "in_Position", "in_Normal", NULL);
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	
	// THRESHOLD

	glUseProgram(thresholdshader);
	useFBO(fbo2, fbo_orig, 0L);
	DrawModel(squareModel, thresholdshader, "in_Position", NULL, "in_TexCoord");

	// FILTERING

	// glUniform1f(glGetUniformLocation(lowpasshader, "t"), t); 


	glUseProgram(lowpasshader);

	for (int i = 0; i<FILTER_PASSES; i++){
		useFBO(fbo1, fbo2, 0L);
		DrawModel(squareModel, lowpasshader, "in_Position", NULL, "in_TexCoord");

		useFBO(fbo2, fbo1, 0L);

		DrawModel(squareModel, lowpasshader, "in_Position", NULL, "in_TexCoord");
	}

	// BLOOMING

	glUseProgram(bloomshader);
	glUniform1i(glGetUniformLocation(bloomshader, "texUnit2"), 1);

	useFBO(fbo1, fbo2, fbo_orig);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawModel(squareModel, bloomshader, "in_Position", NULL, "in_TexCoord");

	// Activate second shader program
	useFBO(0L, fbo1, 0L);
	glUseProgram(plaintextureshader);

	DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");

	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	GLfloat ratio = (GLfloat)w / (GLfloat)h;
	projectionMatrix = perspective(90, ratio, 1.0, 1000);
}


// This function is called whenever the computer is idle
// As soon as the machine is idle, ask GLUT to trigger rendering of a new
// frame
void idle()
{
	glutPostRedisplay();
}

//-----------------------------main-----------------------------------------------
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(W, H);

	glutInitContextVersion(3, 2);
	glutCreateWindow("Render to texture with FBO");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	initKeymapManager();

	init();
	glutMainLoop();
}
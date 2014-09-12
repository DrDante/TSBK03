// TSBK03, Teknik för avancerade datorspel
// Mycket av nedanstående kod är tagen från labbfiler i TSBK07 och TSBK03, skrivna av Ingemar Ragnemalm.
// Lorem ipsum, etc.

// Nedanstående bör lösas. Så småningom.
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
#endif

// --------------------------Includes---------------------------
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
#include "common/LoadTGA.h"
#include "common/zpr.h"
// -------------------------------------------------------------

// ---------------------------Defines---------------------------
#define PI 3.14159265358979323846
// Bredd och höjd.
#define W 512
#define H 512
// Antal ljuskällor.
#define NUM_LIGHTS 4
// Antal filter
#define FILTER_PASSES 10
// -------------------------------------------------------------

// Kvadratmodellen, som resultat från filter ritas upp på.
GLfloat square[] = { -1, -1, 0,
					 -1, 1, 0,
					 1, 1, 0,
					 1, -1, 0 };
GLfloat squareTexCoord[] = { 0, 0,
							 0, 1,
							 1, 1,
							 1, 0 };
GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };
Model* squareModel;

// ---------------------------Globals---------------------------
// Modeller.
Model *model1;
// Vektorer.
vec3 cam = vec3(0.0, 0.0, 0.0);
vec3 point = vec3(0.0, 0.0, 0.0);
// Matriser.
mat4 projectionMatrix;
mat4 viewMatrix;
mat4 bunnyTrans;
// Shaders.
GLuint phongshader = 0, plaintextureshader = 0, lowpasshader = 0, thresholdshader = 0, addtexshader = 0;
// FBOs.
FBOstruct *fbo1, *fbo2, *fbo_orig;
// Övrigt.
//GLfloat t = 0;	// Tidsvariabel.

// ------------------------Ljuskällor------------------------
Point3D lightSourcesColorsArr[] = { { 1.0f, 1.0f, 1.0f },	// Vitt ljus.
									{ 0.0f, 0.0f, 0.0f },	// Inget ljus.
									{ 0.0f, 0.0f, 0.0f },	// Inget ljus.
									{ 0.0f, 0.0f, 0.0f } };	// Inget ljus.
GLfloat specularExponent[] = { 150.0, 0.0, 0.0, 0.0 };		// Spekulär exponent, bör ändras till objektberoende.
GLint isDirectional[] = { 1, 0, 0, 0 };						// För punktkällor (0) är positionen en position.
Point3D lightSourcesDirectionsPositions[] = { { 0.58f, 0.58f, 0.58f },	// Vitt ljus (riktat).
											  { 0.0f, 0.0f, 0.0f },		// Inget ljus.
											  { 0.0f, 0.0f, 0.0f },		// Inget ljus.
											  { 0.0f, 0.0f, 0.0f } };	// Inget ljus.
// ----------------------------------------------------------
// -------------------------------------------------------------

// --------------------Function declarations--------------------
void OnTimer(int value);
void reshape(GLsizei w, GLsizei h);
void idle();
// -------------------------------------------------------------


void init(void)
{
	#ifdef _WIN32
		// Raden nedan verkar krävas för att GLEW inte ska gnälla. Bah.
		glewExperimental = TRUE;
		err = glewInit();
	#endif
	
	dumpInfo();  // Shader info.

	// GL inits.
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// Ladda och kompilera shaders.
	plaintextureshader = loadShaders("shaders/plaintextureshader.vert", "shaders/plaintextureshader.frag");	// Sätter en textur på ett texturobjekt.
	phongshader = loadShaders("shaders/phong.vert", "shaders/phong.frag");									// Renderar ljus (enligt Phong-modellen).
	lowpasshader = loadShaders("shaders/lowpass.vert", "shaders/lowpass.frag");								// Lågpassfiltrerar input. BÖR FIXAS (AMD, samt "riktig" Gauss-filtrering).
	thresholdshader = loadShaders("shaders/threshold.vert", "shaders/threshold.frag");						// Sparar undan det överflödiga ljuset ett objekt kan ha.
	addtexshader = loadShaders("shaders/addtextures.vert", "shaders/addtextures.frag");						// Adderar två texturer till varandra.

	printError("init shader");

	// FBO inits.
	fbo1 = initFBO(W, H, 0);
	fbo2 = initFBO(W, H, 0);
	fbo_orig = initFBO(W, H, 0);

	// Laddning av modeller.
	model1 = LoadModelPlus("objects/bunnyplus.obj");
	squareModel = LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);

	// Initiell placering och skalning av modeller.
	bunnyTrans = T(0, 0, 0);
	bunnyTrans = Mult(S(8, 8, 8), bunnyTrans);

	// ------Ladda upp info om ljuskällorna till phongshadern-------
	glUseProgram(phongshader);
	glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesDirPosArr"), NUM_LIGHTS, &lightSourcesDirectionsPositions[0].x);
	glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesColorArr"), NUM_LIGHTS, &lightSourcesColorsArr[0].x);
	glUniform1fv(glGetUniformLocation(phongshader, "specularExponent"), NUM_LIGHTS, specularExponent);
	glUniform1iv(glGetUniformLocation(phongshader, "isDirectional"), NUM_LIGHTS, isDirectional);
	glActiveTexture(GL_TEXTURE0);
	// -------------------------------------------------------------

	// Initiell placering och orientering av kamera.
	cam = SetVector(0, 5, 15);
	point = SetVector(0, 1, 0);
	zprInit(&viewMatrix, cam, point);
	//viewMatrix = lookAtv(cam, point, vec3(0.0, 1.0, 0.0));

	glutTimerFunc(5, &OnTimer, 0);
}


void display(void)
{
	// Först renderas till fbo_orig (inte till skärmen).
	useFBO(fbo_orig, 0L, 0L);

	// Rensa framebuffer & z-buffer.
	glClearColor(0.1, 0.1, 0.3, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Aktivera shaderprogrammet.
	glUseProgram(phongshader);
	
	// Öka tidsvariabeln t.
	//t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

	// Uppladdning av matriser och annan data till phongshadern.
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "VTPMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "WTVMatrix"), 1, GL_TRUE, viewMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "MTWMatrix"), 1, GL_TRUE, bunnyTrans.m);
	glUniform3fv(glGetUniformLocation(phongshader, "camPos"), 1, &cam.x);
	glUniform1i(glGetUniformLocation(phongshader, "texUnit"), 0);

	// Aktivera z-buffering (inför Phong shading).
	glEnable(GL_DEPTH_TEST);
	// Aktivera backface culling.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// ----------------Scenen renderas till fbo_orig----------------
	DrawModel(model1, phongshader, "in_Position", "in_Normal", NULL);
	// -------------------------------------------------------------
	
	// Avaktivering av z-buffering och backface culling (allt nedan är en platt bild).
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	// ----------------------------Bloom----------------------------
	// 1. Threshold.
	// Allt ljus över 1.0 sparas undan i fbo2, m.h.a. thresholdshadern.
	glUseProgram(thresholdshader);
	useFBO(fbo2, fbo_orig, 0L);
	DrawModel(squareModel, thresholdshader, "in_Position", NULL, "in_TexCoord");

	// 2. Filtrering.
	// Det undansparade ljuset (1) filtreras rekursivt FILTER_PASSES * 2 gånger.
	glUseProgram(lowpasshader);
	for (int i = 0; i < FILTER_PASSES; i++)
	{
		useFBO(fbo1, fbo2, 0L);
		DrawModel(squareModel, lowpasshader, "in_Position", NULL, "in_TexCoord");

		useFBO(fbo2, fbo1, 0L);
		DrawModel(squareModel, lowpasshader, "in_Position", NULL, "in_TexCoord");
	}

	// 3. Blooming.
	// Det filtrerade överskottet (2) av ursprungsbildens ljus adderas till ursprungsbilden.
	glUseProgram(addtexshader);
	useFBO(fbo1, fbo2, fbo_orig);
	glUniform1i(glGetUniformLocation(addtexshader, "texUnit2"), 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawModel(squareModel, addtexshader, "in_Position", NULL, "in_TexCoord");
	// -------------------------------------------------------------

	// Uppritning av bilden, m.h.a. plaintextureshadern.
	glUseProgram(plaintextureshader);
	useFBO(0L, fbo1, 0L);
	DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");

	glutSwapBuffers();
}


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

// --------------------Function definitions---------------------
// -----------------Ingemars hjälpfunktioner-----------------
void OnTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(5, &OnTimer, value);
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
// ----------------------------------------------------------
// -------------------------------------------------------------
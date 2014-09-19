// TSBK03, Teknik f�r avancerade datorspel
// Mycket av nedanst�ende kod �r tagen fr�n labbfiler i TSBK07 och TSBK03, skrivna av Ingemar Ragnemalm.
// Lorem ipsum, etc.

// Nedanst�ende b�r l�sas. S� sm�ningom.
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

// --------------------------Includes---------------------------
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <iostream>

#ifdef _WIN32
// MS
//#include <windows.h>
#include <gl/glew.h>
// Glew initialization... thing.
#include <gl/freeglut.h>
GLenum err;
#else
// Linux
#include <GL/gl.h>
/* #include <GL/freeglut.h> */
#endif

#include "common/VectorUtils3.h"
#include "common/GL_utilities.h"
#include "common/loadobj.h"
#include "common/LoadTGA.h"
#include "camera.h"
#include "common/SDL_util.h"
// -------------------------------------------------------------

// ---------------------------Defines---------------------------
#define PI 3.14159265358979323846
// Bredd och h�jd.
#define W 512
#define H 512
// Antal ljusk�llor.
#define NUM_LIGHTS 4
// Antal l�gpassfilter
#define FILTER_PASSES 30
// Antal ljusspridningar p� diagonalerna
#define DIAG_PASSES 10
// Antal l�gpassfilter p� diagonalerna
#define DIAG_FILTER_PASSES 2

#define DISPLAY_TIMER 0
#define UPDATE_TIMER 1

#define SPEED 2

// -------------------------------------------------------------

// Kvadratmodellen, som resultat fr�n filter ritas upp p�.
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
// Kamera (ers�tter zpr)
Camera cam;
// Matriser.
mat4 projectionMatrix;
mat4 viewMatrix;
mat4 bunnyTrans;
// Shaders.
GLuint phongshader = 0, plaintextureshader = 0, lowpassxshader = 0,
       lowpassyshader, thresholdshader = 0, addtexshader = 0, diag1shader = 0,
       diag2shader = 0;
// FBOs.
FBOstruct *fbo1, *fbo2, *fbo3, *fbo4, *fbo_orig;
// Sk�rmstorlek
int width; 
int height; 
// �vrigt.
//GLfloat t = 0;	// Tidsvariabel.

// ------------------------Ljusk�llor------------------------
Point3D lightSourcesColorsArr[] = { { 1.0f, 1.0f, 1.0f },	// Vitt ljus.
    { 0.0f, 0.0f, 0.0f },	// Inget ljus.
    { 0.0f, 0.0f, 0.0f },	// Inget ljus.
    { 0.0f, 0.0f, 0.0f } };	// Inget ljus.
GLfloat specularExponent[] = { 150.0, 0.0, 0.0, 0.0 };		// Spekul�r exponent, b�r �ndras till objektberoende.
GLint isDirectional[] = { 1, 0, 0, 0 };						// F�r punktk�llor (0) �r positionen en position.
Point3D lightSourcesDirectionsPositions[] = { { 0.58f, 0.58f, 0.58f },	// Vitt ljus (riktat).
    { 0.0f, 0.0f, 0.0f },		// Inget ljus.
    { 0.0f, 0.0f, 0.0f },		// Inget ljus.
    { 0.0f, 0.0f, 0.0f } };	// Inget ljus.
// ----------------------------------------------------------
// -------------------------------------------------------------

// --------------------Function declarations--------------------
void OnTimer(int value);
void reshape(int w, int h, mat4 *projectionMatrix);
void idle();


// SDL functions
void handle_keypress(SDL_Event event);
void handle_mouse(SDL_Event event);
static void event_handler(SDL_Event event);
void handle_userevent(SDL_Event event);
void check_keys();
// -------------------------------------------------------------


void init(void)
{
#ifdef _WIN32
    // Raden nedan verkar kr�vas f�r att GLEW inte ska gn�lla. Bah.
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
    plaintextureshader = loadShaders("shaders/passthrough.vert", "shaders/plaintextureshader.frag");	// S�tter en textur p� ett texturobjekt.
    phongshader = loadShaders("shaders/phong.vert", "shaders/phong.frag");					// Renderar ljus (enligt Phong-modellen).
    lowpassxshader = loadShaders("shaders/passthrough.vert", "shaders/lowpassx.frag");				// L�gpassfiltrerar input i x-led.
    lowpassyshader = loadShaders("shaders/passthrough.vert", "shaders/lowpassy.frag");				// L�gpassfiltrerar input i y-led.
    thresholdshader = loadShaders("shaders/passthrough.vert", "shaders/threshold.frag");			// Sparar undan det �verfl�diga ljuset ett objekt kan ha.
    addtexshader = loadShaders("shaders/passthrough.vert", "shaders/addtextures.frag");			// Adderar tv� texturer till varandra.
    diag1shader = loadShaders("shaders/passthrough.vert", "shaders/diag1.frag");					// Sprider ut ljus i \ diagonalen
    diag2shader = loadShaders("shaders/passthrough.vert", "shaders/diag2.frag");					// Sprider ut ljus i / diagonalen

    printError("init shader");

    // FBO inits.
    fbo1 = initFBO(W, H, 0);
    fbo2 = initFBO(W, H, 0);
    fbo3 = initFBO(W, H, 0);
    fbo4 = initFBO(W, H, 0);
    fbo_orig = initFBO(W, H, 0);

    // Clamp texturkoordinater ist�llet f�r repeat s� inte ljus l�cker �ver
    // kanterna
    glBindTexture(GL_TEXTURE_2D, fbo1->texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, fbo2->texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, fbo3->texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, fbo4->texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Laddning av modeller.
    model1 = LoadModelPlus((char*)"objects/bunnyplus.obj");
    squareModel = LoadDataToModel(square, NULL, squareTexCoord, NULL, squareIndices, 4, 6);

    // Initiell placering och skalning av modeller.
    bunnyTrans = T(0, 0, 0);
    bunnyTrans = Mult(S(8, 8, 8), bunnyTrans);

    // ------Ladda upp info om ljusk�llorna till phongshadern-------
    glUseProgram(phongshader);
    glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesDirPosArr"), NUM_LIGHTS, &lightSourcesDirectionsPositions[0].x);
    glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesColorArr"), NUM_LIGHTS, &lightSourcesColorsArr[0].x);
    glUniform1fv(glGetUniformLocation(phongshader, "specularExponent"), NUM_LIGHTS, specularExponent);
    glUniform1iv(glGetUniformLocation(phongshader, "isDirectional"), NUM_LIGHTS, isDirectional);
    glActiveTexture(GL_TEXTURE0);
    // -------------------------------------------------------------

    // Initiell placering och orientering av kamera.
    cam = Camera(phongshader, &viewMatrix);

    // SDL f�r att d�lja mus och l�sa den till f�nstret
    SDL_SetRelativeMouseMode(SDL_TRUE);
}


void display(void)
{
    // F�rst renderas till fbo_orig (inte till sk�rmen).
    useFBO(fbo_orig, 0L, 0L);

    // Rensa framebuffer & z-buffer.
    glClearColor(0.1, 0.1, 0.3, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Aktivera shaderprogrammet.
    glUseProgram(phongshader);

    // �ka tidsvariabeln t.
    //t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
    mat4 bunnyTotal = bunnyTrans;

    // Uppladdning av matriser och annan data till phongshadern.
    glUniformMatrix4fv(glGetUniformLocation(phongshader, "VTPMatrix"), 1, GL_TRUE, projectionMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(phongshader, "WTVMatrix"), 1, GL_TRUE, viewMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(phongshader, "MTWMatrix"), 1, GL_TRUE, bunnyTotal.m);
    /* glUniform3fv(glGetUniformLocation(phongshader, "camPos"), 1, &cam.x); */
    glUniform1i(glGetUniformLocation(phongshader, "texUnit"), 0);

    // Aktivera z-buffering (inf�r Phong shading).
    glEnable(GL_DEPTH_TEST);
    // Aktivera backface culling.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // ----------------Scenen renderas till fbo_orig----------------
    DrawModel(model1, phongshader, "in_Position", "in_Normal", NULL);
    // -------------------------------------------------------------

    // Avaktivering av z-buffering och backface culling (allt nedan �r en platt bild).
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // ----------------------------Bloom----------------------------
    // 1. Threshold.
    // Allt ljus �ver 1.0 sparas undan i fbo2, m.h.a. thresholdshadern.
    //
    // Efter Threshold:
    // Tr�sklat ljus i fbo2
    glUseProgram(thresholdshader);
    useFBO(fbo2, fbo_orig, 0L);
    DrawModel(squareModel, thresholdshader, "in_Position", NULL, "in_TexCoord");

    // 2. Sprid ut ljus i diagonalerna
    //
    // Efter ljusspridning:
    // \ Diagonaler i fbo3
    // / Diagonaler i fbo4
    glUseProgram(diag1shader);

    useFBO(fbo3, fbo2, 0L);
    DrawModel(squareModel, diag1shader, "In_Position", NULL, "In_TexCoord");
    for (int i = 0; i < DIAG_PASSES; i++){
	useFBO(fbo1, fbo3, 0L);
	DrawModel(squareModel, diag1shader, "In_Position", NULL, "In_TexCoord");
	useFBO(fbo3, fbo1, 0L);
	DrawModel(squareModel, diag1shader, "In_Position", NULL, "In_TexCoord");
    }

    glUseProgram(diag2shader);

    useFBO(fbo4, fbo2, 0L);
    DrawModel(squareModel, diag1shader, "In_Position", NULL, "In_TexCoord");
    for (int i = 0; i < DIAG_PASSES; i++){
	useFBO(fbo1, fbo4, 0L);
	DrawModel(squareModel, diag2shader, "In_Position", NULL, "In_TexCoord");
	useFBO(fbo4, fbo1, 0L);
	DrawModel(squareModel, diag2shader, "In_Position", NULL, "In_TexCoord");
    }

    // 3. Addera ljuset p� diagonalerna
    //
    // Efter addering:
    // Ljusdiagonaler i fbo1
    glUseProgram(addtexshader);
    useFBO(fbo1, fbo3, fbo4);
    glUniform1i(glGetUniformLocation(addtexshader, "texUnit2"), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawModel(squareModel, addtexshader, "in_Position", NULL, "in_TexCoord");

    // 4. Filtrering.
    // Det undansparade ljuset (1) filtreras rekursivt FILTER_PASSES g�nger.
    //
    // Efter filtrering:
    // Utsmetat ljus i fbo2

    for (int i = 0; i < FILTER_PASSES; i++)
    {
	glUseProgram(lowpassxshader);
	useFBO(fbo3, fbo2, 0L);
	DrawModel(squareModel, lowpassxshader, "in_Position", NULL, "in_TexCoord");
	glUseProgram(lowpassyshader);
	useFBO(fbo2, fbo3, 0L);
	DrawModel(squareModel, lowpassyshader, "in_Position", NULL, "in_TexCoord");
    }

    // 4. Filtera ljuset p� diagonalerna
    // 
    // Efter filtrering:
    // Utsmetade diagonaler i fbo1
    for (int i = 0; i < DIAG_FILTER_PASSES; i++){
	glUseProgram(lowpassxshader);
	useFBO(fbo3, fbo1, 0L);
	DrawModel(squareModel, lowpassxshader, "in_Position", NULL, "in_TexCoord");
	glUseProgram(lowpassyshader);
	useFBO(fbo1, fbo3, 0L);
	DrawModel(squareModel, lowpassyshader, "in_Position", NULL, "in_TexCoord");
    }

    // 5. Blooming.
    // Det filtrerade �verskottet (2) av ursprungsbildens ljus adderas till ursprungsbilden och ljusdiagonalerna.
    glUseProgram(addtexshader);
    useFBO(fbo3, fbo2, fbo_orig);
    glUniform1i(glGetUniformLocation(addtexshader, "texUnit2"), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawModel(squareModel, addtexshader, "in_Position", NULL, "in_TexCoord");
    useFBO(fbo2, fbo3, fbo1);
    DrawModel(squareModel, addtexshader, "in_Position", NULL, "in_TexCoord");
    // -------------------------------------------------------------

    // Uppritning av bilden, m.h.a. plaintextureshadern.
    glUseProgram(plaintextureshader);
    useFBO(0L, fbo2, 0L);
    DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");

    swap_buffers();
}

// Timer f�r uppritande av sk�rm. Man f�r inte kalla funktioner h�r inne
Uint32 display_timer(Uint32 interval, void* param)
{
	SDL_Event event;
	
	event.type = SDL_USEREVENT;
	event.user.code = DISPLAY_TIMER;
	event.user.data1 = 0;
	event.user.data2 = 0;

	SDL_PushEvent(&event);
	return interval;
}

Uint32 update_timer(Uint32 interval, void* param)
{
    SDL_Event event;

    event.type = SDL_USEREVENT;
    event.user.code = UPDATE_TIMER;
    event.user.data1 = (void*) (intptr_t) interval;
    event.user.data2 = 0;

    SDL_PushEvent(&event);
    return interval;
}

// Hantera event
void event_handler(SDL_Event event)
{
	switch(event.type){
		case SDL_USEREVENT:
			handle_userevent(event);
			break;
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYDOWN:
			handle_keypress(event);
			break;
		case SDL_WINDOWEVENT:
			switch(event.window.event){
			    case SDL_WINDOWEVENT_RESIZED:
				get_window_size(&width, &height);
				resize_window(event);
				reshape(width, height, &projectionMatrix);
				break;
			} 
			break;
		case SDL_MOUSEMOTION:
			handle_mouse(event);
			break;
		default:
			break;
	}
}

// Hantera av�ndardefinierade event
void handle_userevent(SDL_Event event)
{
    switch(event.user.code){
        case (int)DISPLAY_TIMER:
            display();
            break;
	case (int)UPDATE_TIMER:
	    check_keys();
	    break;
        default:
            break;
    }
}

// Hantera knapptryckningar
void handle_keypress(SDL_Event event){
    switch(event.key.keysym.sym){
	case SDLK_ESCAPE:
	case SDLK_q:
	    exit(0);
	    break;
	case SDLK_g:
	    SDL_SetRelativeMouseMode(SDL_FALSE);
	    break;
	case SDLK_h:
	    SDL_SetRelativeMouseMode(SDL_TRUE);
	    break;
	default: 
	    break;
    }
}

void handle_mouse(SDL_Event event){
    get_window_size(&width, &height);
    cam.change_look_at_pos(event.motion.xrel,event.motion.y,width,height);
}

void check_keys()
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if(keystate[SDL_SCANCODE_W]) {
        cam.forward(0.1*SPEED);
    } else if(keystate[SDL_SCANCODE_S]) {
        cam.forward(-0.1*SPEED);
    }
    if(keystate[SDL_SCANCODE_A]) {
        cam.strafe(0.1*SPEED);
    } else if(keystate[SDL_SCANCODE_D]) {
        cam.strafe(-0.1*SPEED);
    }
}

int main(int argc, char *argv[])
{
    init_SDL((const char*) "TSBK03 Projekt SDL", 640, 480);
    reshape(640, 480, &projectionMatrix);
    init();
    SDL_TimerID timer_id;
    timer_id = SDL_AddTimer(30, &display_timer, NULL);
    timer_id = SDL_AddTimer(10, &update_timer, NULL);
    if(timer_id == 0){
	std::cerr << "Error setting timer function: " << SDL_GetError() << std::endl;
    }
    set_event_handler(&event_handler);
    inf_loop();
    return 0;
}

// --------------------Function definitions---------------------
// -----------------Ingemars hj�lpfunktioner-----------------
void reshape(int w, int h, mat4 *projectionMatrix)
{
    glViewport(0, 0, w, h);
    GLfloat ratio = (GLfloat)w / (GLfloat)h;
    *projectionMatrix = perspective(90, ratio, 1.0, 1000);
}
// ----------------------------------------------------------
// -------------------------------------------------------------

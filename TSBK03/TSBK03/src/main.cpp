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
#include <GL/freeglut.h>
#endif

// F�r att glm inte ska skrika i kompilering
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common/GL_utilities.h"
#include "common/loadobj.h"
#include "common/LoadTGA.h"
#include "camera.h"
#include "common/SDL_util.h"
#include "shadow_map.hpp"
// -------------------------------------------------------------

// ---------------------------Defines---------------------------
#define PI 3.14159265358979323846f
// Bredd och h�jd.
#define W 512
#define H 512
// Antal ljusk�llor.
#define NUM_LIGHTS 4

#define DISPLAY_TIMER 0
#define UPDATE_TIMER 1

#define SPEED 2

// ---------------------------Globals---------------------------
// Modeller.
Model *bunnyModel, *squareModel;
GLfloat square[] = 
{ 
    -1, -1, 0,
    -1,  1, 0,
     1,  1, 0,
     1, -1, 0
};

GLfloat squareTexCoord[] = 
{ 
    0, 0,
    0, 1,
    1, 1,
    1, 0
};

GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };

GLfloat squareNormals[] =
{
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0
};
    

// Kamera (ers�tter zpr)
Camera cam;
// Matriser.
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 bunnyTrans;
glm::mat4 squareTrans;
glm::mat4 scale_bias;

// FBO
FBOstruct *z_fbo;

// Shaders.
GLuint shadowshader = 0, zshader = 0, ptshader = 0; // passthrough shader;
// Sk�rmstorlek
int width = 640; 
int height = 480; 
// �vrigt.
//GLfloat t = 0;	// Tidsvariabel.

// ------------------------Ljusk�llor------------------------
/* Point3D lightSourcesColorsArr[] = */ 
/* {   { 1.0f, 1.0f, 1.0f },	// Vitt ljus. */
/*     { 0.0f, 0.0f, 0.0f },	// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f },	// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f } */
/* };	// Inget ljus. */
/* GLfloat specularExponent[] = { 150.0, 0.0, 0.0, 0.0 };		// Spekul�r exponent, b�r �ndras till objektberoende. */
/* GLint isDirectional[] = { 1, 0, 0, 0 };						// F�r punktk�llor (0) �r positionen en position. */
/* Point3D lightSourcesDirectionsPositions[] = */ 
/* {   { 0.58f, 0.58f, 0.58f },	// Vitt ljus (riktat). */
/*     { 0.0f, 0.0f, 0.0f },		// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f },		// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f } */
/* };		// Inget ljus. */
// ----------------------------------------------------------
// -------------------------------------------------------------

// --------------------Ljusk�lla-----------------------------
class lightSource 
{
    public:
	glm::vec3 pos;
	bool positional;
	glm::vec3 look_at;

	lightSource(glm::vec3 pos, bool positional, glm::vec3 look_at):
	    pos(pos), positional(positional), look_at(look_at) {}
	lightSource() = delete;

	glm::vec3 move(glm::vec3 v){
	    pos += v;
	    return pos;
	}
};

lightSource spotlight(glm::vec3(0,10,10), false, glm::vec3(0,0,0));

// --------------------Function declarations--------------------
void OnTimer(int value);
void reshape(int w, int h, glm::mat4 &projectionMatrix);
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
    zshader = loadShaders("shaders/depthbuffer.vert", "shaders/depthbuffer.frag");	// Renderar djupv�rden till z-buffer
    shadowshader = loadShaders("shaders/shadow_1.vert", "shaders/shadow_1.frag");	// Renderar skuggor till sk�rm
    ptshader = loadShaders("shaders/passthrough.vert", "shaders/passthrough.frag");	// Renderar djuptextur till sk�rm

    // Init z_fbo
    z_fbo = init_z_fbo(width, height);

    printError("init shader");

    // Laddning av modeller.
    bunnyModel = LoadModelPlus((char*)"objects/bunnyplus.obj");
    squareModel = LoadDataToModel(square, squareNormals, squareTexCoord, NULL, squareIndices, 4, 6);

    // Initiell placering och skalning av modeller.
    bunnyTrans = glm::scale(glm::mat4(), glm::vec3(3,3,3));

    squareTrans = glm::scale(glm::mat4(), glm::vec3(20,20,20));
    squareTrans = glm::translate(glm::vec3(0,-10,0)) * squareTrans;
    squareTrans = glm::rotate(squareTrans, float(PI/2.0), glm::vec3(1,0,0));

    // Scale and bias f�r shadow map
    scale_bias = glm::scale(glm::translate(glm::vec3(1, 1, 0)), glm::vec3(0.5, 0.5, 1));
    
    // Initiell placering och orientering av kamera.
    cam = Camera(zshader, &viewMatrix);

    // SDL f�r att d�lja mus och l�sa den till f�nstret
    SDL_SetRelativeMouseMode(SDL_TRUE);
}


void display(void)
{
    // Rensa framebuffer & z-buffer.
    glClearColor(0.1, 0.1, 0.3, 0);

    // Aktivera shaderprogrammet.
    glUseProgram(zshader);
    useFBO(z_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // �ka tidsvariabeln t.
    //t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
    glm::mat4 bunnyTotal = bunnyTrans;

    // Aktivera z-buffering
    glEnable(GL_DEPTH_TEST);
    /* glEnable(GL_CULL_FACE); */
    /* glCullFace(GL_BACK); */

    // Flytta kamera till ljusk�llan
    glm::vec3 tmp_cam_pos = cam.position;
    glm::vec3 tmp_cam_look_at = cam.look_at_pos;

    cam.position = spotlight.pos;
    cam.look_at_pos = spotlight.look_at;
    cam.update();

    // Uppladdning av matriser och annan data till shadern.
    glUniformMatrix4fv(glGetUniformLocation(zshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(zshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(zshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyTotal));

    // ----------------Scenen renderas till z-buffern ----------------
    DrawModel(bunnyModel, zshader, "in_Position", "in_Normal", NULL);
    glUniformMatrix4fv(glGetUniformLocation(zshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(squareTrans));
    DrawModel(squareModel, zshader, "in_Position", "in_Normal", NULL);
    // -------------------------------------------------------------

    glm::mat4 textureMatrix = scale_bias * projectionMatrix * viewMatrix;
    
    // �terst�ll kameran till ursprungsposition
    cam.position = tmp_cam_pos;
    cam.look_at_pos = tmp_cam_look_at;
    cam.update();
    
    glUseProgram(shadowshader);
    useFBO(0L, z_fbo, 0L);
    glDisable(GL_CULL_FACE);

    glm::mat4 textureMatrixTot = textureMatrix * bunnyTotal;

    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyTotal));
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ----------------Scenen renderas till sk�rmen  ----------------
    glUniform1i(glGetUniformLocation(shadowshader, "texUnit"), 0);

    DrawModel(bunnyModel, shadowshader, "in_Position", "in_Normal", NULL);


    textureMatrixTot = textureMatrix * squareTrans; 
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
    glUniformMatrix4fv(glGetUniformLocation(zshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(squareTrans));
    DrawModel(squareModel, shadowshader, "in_Position", "in_Normal", NULL);
    // -------------------------------------------------------------

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
		    reshape(width, height, projectionMatrix);
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
void handle_keypress(SDL_Event event)
{
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
	case SDLK_UP:
	    spotlight.move(glm::vec3(0,0,-1));
	    break;
	case SDLK_DOWN:
	    spotlight.move(glm::vec3(0,0,1));
	    break;
	case SDLK_LEFT:
	    spotlight.move(glm::vec3(-1,0,0));
	    break;
	case SDLK_RIGHT:
	    spotlight.move(glm::vec3(1,0,0));
	    break;
	default: 
	    break;
    }
}

void handle_mouse(SDL_Event event)
{
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
    init_SDL((const char*) "TSBK03 Projekt SDL", width, height);
    reshape(width, height, projectionMatrix);
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
void reshape(int w, int h, glm::mat4 &projectionMatrix)
{
    glViewport(0, 0, w, h);
    float ratio = (GLfloat)w / (GLfloat)h;
    projectionMatrix = glm::perspective(PI/2, ratio, 1.0f, 1000.0f);
}
// ----------------------------------------------------------

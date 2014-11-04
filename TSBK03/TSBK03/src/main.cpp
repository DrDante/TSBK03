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

// För att glm inte ska skrika i kompilering
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "common/VectorUtils3.h"
#include "common/GL_utilities.h"
#include "common/loadobj.h"
#include "common/LoadTGA.h"
#include "camera.h"
#include "common/SDL_util.h"
#include "shadow_map.hpp"
// -------------------------------------------------------------

// ---------------------------Defines---------------------------
#define PI 3.14159265358979323846f
// Bredd och höjd.
#define W 512
#define H 512
// Antal ljuskällor.
#define NUM_LIGHTS 4

#define DISPLAY_TIMER 0
#define UPDATE_TIMER 1

#define SPEED 2

// ---------------------------Globals---------------------------
// Modeller.
Model *model1, *squareModel;
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
    

// Kamera (ersätter zpr)
Camera cam;
// Matriser.
mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 bunnyTrans;
glm::mat4 squareTrans;
glm::mat4 scale_bias;

// FBO
FBOstruct *z_fbo;

// Shaders.
GLuint phongshader = 0, zshader = 0, ptshader = 0; // passthrough shader;
// Skärmstorlek
int width = 640; 
int height = 480; 
// Övrigt.
//GLfloat t = 0;	// Tidsvariabel.

// ------------------------Ljuskällor------------------------
/* Point3D lightSourcesColorsArr[] = */ 
/* {   { 1.0f, 1.0f, 1.0f },	// Vitt ljus. */
/*     { 0.0f, 0.0f, 0.0f },	// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f },	// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f } */
/* };	// Inget ljus. */
/* GLfloat specularExponent[] = { 150.0, 0.0, 0.0, 0.0 };		// Spekulär exponent, bör ändras till objektberoende. */
/* GLint isDirectional[] = { 1, 0, 0, 0 };						// För punktkällor (0) är positionen en position. */
/* Point3D lightSourcesDirectionsPositions[] = */ 
/* {   { 0.58f, 0.58f, 0.58f },	// Vitt ljus (riktat). */
/*     { 0.0f, 0.0f, 0.0f },		// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f },		// Inget ljus. */
/*     { 0.0f, 0.0f, 0.0f } */
/* };		// Inget ljus. */
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
    phongshader = loadShaders("shaders/phong.vert", "shaders/phong.frag");      // Renderar ljus (enligt Phong-modellen).
    zshader = loadShaders("shaders/shadow_1.vert", "shaders/shadow_1.frag");	// Renderar djupvärden till z-buffer
    ptshader = loadShaders("shaders/passthrough.vert", "shaders/passthrough.frag");	// Renderar djuptextur till skärm

    // Init z_fbo
    z_fbo = initFBO2(width, height, 0, 1);

    printError("init shader");

    // Laddning av modeller.
    model1 = LoadModelPlus((char*)"objects/bunnyplus.obj");
    squareModel = LoadDataToModel(square, squareNormals, squareTexCoord, NULL, squareIndices, 4, 6);

    // Initiell placering och skalning av modeller.
    bunnyTrans = glm::scale(glm::mat4(), glm::vec3(3,3,3));

    squareTrans = glm::scale(glm::mat4(), glm::vec3(20,20,20));
    squareTrans = glm::translate(glm::vec3(0,-10,0)) * squareTrans;
    squareTrans = glm::rotate(squareTrans, float(PI/2.0), glm::vec3(1,0,0));

    // Scale and bias för shadow map
    scale_bias = glm::scale(glm::translate(glm::vec3(1, 1, 0)), glm::vec3(0.5, 0.5, 1));
    

    /* // ------Ladda upp info om ljuskällorna till phongshadern------- */
    /* glUseProgram(phongshader); */
    /* glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesDirPosArr"), NUM_LIGHTS, &lightSourcesDirectionsPositions[0].x); */
    /* glUniform3fv(glGetUniformLocation(phongshader, "lightSourcesColorArr"), NUM_LIGHTS, &lightSourcesColorsArr[0].x); */
    /* glUniform1fv(glGetUniformLocation(phongshader, "specularExponent"), NUM_LIGHTS, specularExponent); */
    /* glUniform1iv(glGetUniformLocation(phongshader, "isDirectional"), NUM_LIGHTS, isDirectional); */
    /* glActiveTexture(GL_TEXTURE0); */
    /* // ------------------------------------------------------------- */
    
    glUseProgram(zshader);
    glUniformMatrix4fv(glGetUniformLocation(zshader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(scale_bias));

    // Initiell placering och orientering av kamera.
    cam = Camera(zshader, &viewMatrix);

    // SDL för att dölja mus och låsa den till fönstret
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

    // Öka tidsvariabeln t.
    //t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
    glm::mat4 bunnyTotal = bunnyTrans;

    // Uppladdning av matriser och annan data till shadern.
    glUniformMatrix4fv(glGetUniformLocation(zshader, "VTPMatrix"), 1, GL_TRUE, projectionMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(zshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(zshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyTotal));

    // Aktivera z-buffering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // ----------------Scenen renderas till z-buffern ----------------
    DrawModel(model1, zshader, "in_Position", "in_Normal", NULL);
    glDisable(GL_CULL_FACE);
    glUniformMatrix4fv(glGetUniformLocation(zshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(squareTrans));
    DrawModel(squareModel, zshader, "in_Position", "in_Normal", NULL);
    // -------------------------------------------------------------
    
    glUseProgram(ptshader);
    useFBO(0L, z_fbo, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ----------------Scenen renderas till skärmen ----------------
    glUniform1i(glGetUniformLocation(ptshader, "texUnit"), 0);
    DrawModel(squareModel, ptshader, "in_Position", NULL, "in_TexCoord");
    // -------------------------------------------------------------

    swap_buffers();
}

// Timer för uppritande av skärm. Man får inte kalla funktioner här inne
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

// Hantera avändardefinierade event
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
    reshape(width, height, &projectionMatrix);
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
// -----------------Ingemars hjälpfunktioner-----------------
void reshape(int w, int h, mat4 *projectionMatrix)
{
    glViewport(0, 0, w, h);
    GLfloat ratio = (GLfloat)w / (GLfloat)h;
    *projectionMatrix = perspective(90, ratio, 1.0, 1000);
}
// ----------------------------------------------------------

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
#include <iostream>

#ifdef _WIN32
// MS
//#include <windows.h>
#include <gl/glew.h>
// Glew initialization... thing.
GLenum err;
#else
// Linux
#include <GL/gl.h>
#endif
#include <SDL2/SDL.h>

// För att glm inte ska skrika i kompilering
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
    

// Kamera (ersätter zpr)
Camera cam;
// Matriser.
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 scaleBiasMatrix;
glm::mat4 bunnyTrans;
glm::mat4 squareTrans;

// FBO
FBOstruct *z_fbo;

// Shaders.
GLuint shadowshader = 0, zshader = 0; // passthrough shader;
// Skärmstorlek
int width = 640; 
int height = 480; 
// Övrigt.

// Bias som används för att undvika skuggacne, används i shadow_1.frag
float bias = 0.0015;

//GLfloat t = 0;	// Tidsvariabel.

// --------------------Ljuskälla-----------------------------
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
    zshader = loadShaders("shaders/depthbuffer.vert", "shaders/depthbuffer.frag");	// Renderar djupvärden till z-buffer
    shadowshader = loadShaders("shaders/shadow_1.vert", "shaders/shadow_1.frag");	// Renderar skuggor till skärm

    // Init z_fbo
    // Ändra width och height för bättre upplösning på skuggor!
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

    // Scale and bias för shadow map
    scaleBiasMatrix = glm::translate(glm::scale(glm::mat4(), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(1,1,1));

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

    // Aktivera z-buffering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Flytta kamera till ljuskällan
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

    glm::mat4 textureMatrix = scaleBiasMatrix * projectionMatrix * viewMatrix;
    
    // Återställ kameran till ursprungsposition
    cam.position = tmp_cam_pos;
    cam.look_at_pos = tmp_cam_look_at;
    cam.update();
    
    glUseProgram(shadowshader);
    useFBO(0L, z_fbo, 0L);
    glCullFace(GL_BACK);


    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyTotal));
    glUniform1f(glGetUniformLocation(shadowshader, "bias"), bias);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ----------------Scenen renderas till skärmen  ----------------
   
    glUniform1i(glGetUniformLocation(shadowshader, "texUnit"), 0);

    glm::mat4 textureMatrixTot = textureMatrix * bunnyTotal;
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
    DrawModel(bunnyModel, shadowshader, "in_Position", "in_Normal", NULL);

    textureMatrixTot = textureMatrix * squareTrans; 
    glUniformMatrix4fv(glGetUniformLocation(shadowshader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
    glUniformMatrix4fv(glGetUniformLocation(zshader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(squareTrans));
    DrawModel(squareModel, shadowshader, "in_Position", "in_Normal", NULL);
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
	case SDLK_KP_PLUS:
	    spotlight.move(glm::vec3(0,1,0));
	    break;
	case SDLK_KP_MINUS:
	    spotlight.move(glm::vec3(0,-1,0));
	    break;
	// Print camera position for debugging
	case SDLK_p:
	    std::cout << "Camera position: " << cam.position.x << ", " << cam.position.y << ", " << cam.position.z << std::endl;
	    break;
	// Print spotlight position for debugging
	case SDLK_l:
	    std::cout << "Spotlight position: " << spotlight.pos.x << ", " << spotlight.pos.y << ", " << spotlight.pos.z << std::endl;
	    break;
	// Öka bias
	case SDLK_b:
	    bias += 0.0001;
	    std::cout << bias << std::endl;
	    break;
	// Minska bias
	case SDLK_n:
	    bias -= 0.0001;
	    std::cout << bias << std::endl;
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
// -----------------Ingemars hjälpfunktioner-----------------
void reshape(int w, int h, glm::mat4 &projectionMatrix)
{
    glViewport(0, 0, w, h);
    float ratio = (GLfloat)w / (GLfloat)h;
    projectionMatrix = glm::perspective(PI/2, ratio, 1.0f, 1000.0f);
}
// ----------------------------------------------------------

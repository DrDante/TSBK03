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

#include "objects.hpp"
#include "common/GL_utilities.h"
#include "common/loadobj.h"
#include "common/LoadTGA.h"
#include "camera.h"
#include "common/SDL_util.h"
#include "shadow_map.hpp"
// -------------------------------------------------------------

// ---------------------------Defines---------------------------
#define PI 3.14159265358979323846f

// Bredd och höjd på djuptextur
#define SHADOW_W 2048
#define SHADOW_H 2048
// Antal ljuskällor.
#define NUM_LIGHTS 4

#define DISPLAY_TIMER 0
#define UPDATE_TIMER 1

#define SPEED 2

// ---------------------------Globals---------------------------
// Saker
Thing bunny, ground, sidewall, backwall, sphere, torus, lightsrc, blend;

// Model
Model *wallModel;

//---
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
//---

// Kamera (ersätter zpr)
Camera cam;
// Matriser.
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 scaleBiasMatrix;
glm::mat4 bunnyTrans;
glm::mat4 squareTrans;
glm::mat4 groundTrans;
glm::mat4 wallTrans;
glm::mat4 blendTrans;

// FBO
FBOstruct *z_fbo, *tmp_fbo, *res_fbo;

// Shaders.
GLuint shadowphongshader = 0, zshader = 0, addshader = 0, plainshader = 0; // passthrough shader;
// Skärmstorlek
int width = 1024; 
int height = 768; 
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

	glm::mat4 projectionMatrix;
};

lightSource spotlight1(glm::vec3(17, 0, 3), false, glm::vec3(1, 1, 1));
bool draw1 = 1;
lightSource spotlight2(glm::vec3(15, -2, 13), false, glm::vec3(1, 1, 1));
bool draw2 = 1;
bool debugmode = 0;

// --------------------Function declarations--------------------
void OnTimer(int value);
void reshape(int w, int h, glm::mat4 &projectionMatrix);
void idle();
void fbo_add_tmp_to_res();
void draw_scene(lightSource light);
void draw_lights(lightSource light);

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
    zshader = loadShaders("shaders/depthbuffer.vert", "shaders/depthbuffer.frag");				// Renderar djupvärden till z-buffer
    shadowphongshader = loadShaders("shaders/shadowphong.vert", "shaders/shadowphong.frag");	// Renderar skuggor till skärm
	addshader = loadShaders("shaders/passthrough.vert", "shaders/addtextures.frag");			// Adderar texturer från två fbos
	plainshader = loadShaders("shaders/passthrough.vert", "shaders/plaintextureshader.frag");	// Ritar ut 
	tmp_fbo = initFBO(width, height, 0);
	res_fbo = initFBO(width, height, 0);

    // Init z_fbo
    // Ändra width och height för bättre upplösning på skuggor!
    z_fbo = init_z_fbo(SHADOW_W, SHADOW_H);

    // Ladda upp hur står ändring i texturen en pixel är
    glUseProgram(shadowphongshader);
    glm::vec2 pixelDiff = glm::vec2(1.0/SHADOW_W, 1.0/SHADOW_H);
    glUniform2f(glGetUniformLocation(shadowphongshader, "pixelDiff"), pixelDiff.x, pixelDiff.y);
    

    printError("init shader");

    // Laddning av modeller.
    wallModel = LoadModelPlus((char*)"objects/ground.obj");
    bunny = Thing("objects/bunnyplus.obj");
    sphere = Thing("objects/sphere.obj");
    torus = Thing("objects/torus.obj");

	squareModel = LoadDataToModel(
		square, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);

	lightsrc = Thing("objects/sphere.obj");

	blend = Thing("objects/blend.obj");

    // Initiell placering och skalning av modeller.
    bunny.MTWmatrix = glm::scale(glm::mat4(), glm::vec3(3,3,3));

    torus.MTWmatrix = glm::translate(glm::vec3(-5,-2,-4));

    sphere.MTWmatrix = glm::translate(glm::vec3(5,0,-2));

    ground.MTWmatrix = glm::scale(glm::mat4(), glm::vec3(20,20,20));
    ground.MTWmatrix= glm::translate(glm::vec3(0,-10,0)) * ground.MTWmatrix;

    backwall.MTWmatrix = glm::scale(glm::mat4(), glm::vec3(20,20,20));
    backwall.MTWmatrix = glm::rotate(backwall.MTWmatrix, float(PI/2.0), glm::vec3(1,0,0));
    backwall.MTWmatrix = glm::translate(glm::vec3(0,9.9,-20)) * backwall.MTWmatrix;

    sidewall.MTWmatrix = glm::scale(glm::mat4(), glm::vec3(20,20,20));
    sidewall.MTWmatrix = glm::rotate(sidewall.MTWmatrix, float(PI/2.0), glm::vec3(0,0,1));
    sidewall.MTWmatrix = glm::translate(glm::vec3(20,9.9,0)) * sidewall.MTWmatrix;


	blend.MTWmatrix = glm::scale(glm::mat4(), glm::vec3(10, 10, 10));
	blend.MTWmatrix = glm::translate(glm::vec3(0, -5, 0)) * blend.MTWmatrix;

    // Scale and bias för shadow map
    scaleBiasMatrix = glm::translate(glm::scale(glm::mat4(), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(1,1,1));

    // Initiell placering och orientering av kamera.
    cam = Camera(zshader, &viewMatrix);

    // SDL för att dölja mus och låsa den till fönstret
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Sätt ljuskällornas projektionsmatrix
	glm::mat4 lightpersp = glm::perspective(float(PI / 2), float(width) / height, 1.0f, 1000.0f);
    spotlight1.projectionMatrix = lightpersp;
	spotlight2.projectionMatrix = lightpersp;
	//spotlight.projectionMatrix = glm::ortho(0.0f, float(width), 0.0f, float(height), 1.0f, 1000.0f);

}

void display(void)
{
	// TODO
	// 1 Bygg scen (Julius, blender!)
	// 2 Bygg system för ljuskällegeometri + penumbra
	// 2 I arbete ovan, orda objektberoende phongparametrar
	// 3 Med planerade mängd ljuskällor, gör processen som görs per ljuskälla
	// 4 Animering, någon slags rörelsesystem (inkl kollisionsdetection?), knappar för att stänga av ljuskällor
	// * Lägg på bloom (och kanske motion blur?)
	// * Putsa

	// Rensa framebuffer & z-buffer.
	glClearColor(0.0, 0.0, 0.0, 0);
	useFBO(res_fbo, 0L, 0L);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Aktivera shaderprogrammet.
	//glUseProgram(zshader);
	glClearColor(0.0, 0.0, 0.0, 0);
	useFBO(tmp_fbo, 0L, 0L);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    useFBO(z_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Öka tidsvariabeln t.
    //t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
	
	// Rita ut scenen till z-buffern, sedan med phong till tmp_fbo, och addera till res_fbo
	if (draw1)
	{
		draw_scene(spotlight1);
	}
	if (draw2)
	{
		draw_scene(spotlight2);
	}

	// Rita ut ljuskällor till res_fbo
	if (debugmode)
	{
		if (draw1)
		{
			draw_lights(spotlight1);
		}
		if (draw2)
		{
			draw_lights(spotlight2);
		}
	}

	// Rita ut res_fbo till skärmen	
	glUseProgram(plainshader);
	useFBO(0L, res_fbo, 0L);
	DrawModel(squareModel, plainshader, "in_Position", NULL, "in_TexCoord");

    swap_buffers();
}

void fbo_add_tmp_to_res()
{
	// Adds two fbos (tmp_fbo and res_fbo) to res_fbo
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(addshader);
	useFBO(res_fbo, tmp_fbo, 0L);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tmp_fbo->texid);
	glUniform1i(glGetUniformLocation(addshader, "texUnit"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, res_fbo->texid);
	glUniform1i(glGetUniformLocation(addshader, "texUnit2"), 1);
	DrawModel(squareModel, addshader, "in_Position", NULL, "in_TexCoord");
	glActiveTexture(GL_TEXTURE0);
}

void draw_scene(lightSource light)
{
	glUseProgram(zshader);
	useFBO(z_fbo, 0L, 0L);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Aktivera z-buffering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Flytta kamera till ljuskällan
	glm::vec3 tmp_cam_pos = cam.position;
	glm::vec3 tmp_cam_look_at = cam.look_at_pos;

	cam.position = light.pos;
	cam.look_at_pos = light.look_at;
	cam.update();

	// Uppladdning av matriser och annan data till shadern.
	glUniformMatrix4fv(glGetUniformLocation(zshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(light.projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(zshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// ----------------Scenen renderas till z-buffern ----------------

	// Rita kanin
	bunny.draw(zshader);

	// Rita sfär
	sphere.draw(zshader);

	// Rita torus
	torus.draw(zshader);

	// Rita golv
	//ground.draw(zshader, wallModel);

	// Rita bakre vägg
	//backwall.draw(zshader, wallModel);

	// Rita sidovägg
	//sidewall.draw(zshader, wallModel);

	//
	blend.draw(zshader);

	// -------------------------------------------------------------

	glm::mat4 textureMatrix = scaleBiasMatrix * projectionMatrix * viewMatrix;

	// Återställ kameran till ursprungsposition
	cam.position = tmp_cam_pos;
	cam.look_at_pos = tmp_cam_look_at;
	cam.update();

	glUseProgram(shadowphongshader);
	useFBO(tmp_fbo, z_fbo, 0L);
	

	glUniformMatrix4fv(glGetUniformLocation(shadowphongshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shadowphongshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3fv(glGetUniformLocation(shadowphongshader, "lPos"), 1, glm::value_ptr(light.pos));
	glUniform1f(glGetUniformLocation(shadowphongshader, "bias"), bias);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ---------------- Scenen renderas till tmp_fbo ----------------

	glUniform1i(glGetUniformLocation(shadowphongshader, "texUnit"), 0);
	GLfloat camPos[3] = { cam.position.x, cam.position.y, cam.position.z };
	glUniform3fv(glGetUniformLocation(shadowphongshader, "camPos"), 1, camPos);

	// Rita kanin
	bunny.draw_with_depthinfo(shadowphongshader, textureMatrix);

	// Rita torus
	torus.draw_with_depthinfo(shadowphongshader, textureMatrix);

	// Rita sfär
	sphere.draw_with_depthinfo(shadowphongshader, textureMatrix);

	// Rita golv
	//ground.draw_with_depthinfo(shadowphongshader, textureMatrix, wallModel);

	// Rita bakre vägg
	//backwall.draw_with_depthinfo(shadowphongshader, textureMatrix, wallModel);

	// Rita sidovägg
	//sidewall.draw_with_depthinfo(shadowphongshader, textureMatrix, wallModel);

	//
	blend.draw_with_depthinfo(shadowphongshader, textureMatrix);

	// -------------------------------------------------------------

	// Scenen adderas till res_fbo
	fbo_add_tmp_to_res();
}

void draw_lights(lightSource light)
{
	// Rita ut ljuskällor
	glUseProgram(shadowphongshader);
	useFBO(tmp_fbo, 0L, 0L);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	lightsrc.MTWmatrix = glm::translate(light.pos);
	lightsrc.draw(shadowphongshader);

	// Addera till res_fbo
	fbo_add_tmp_to_res();
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
	case SDLK_1:
		draw1 = !draw1;
		break;
	case SDLK_2:
		draw2 = !draw2;
		break;
	case SDLK_z:
		debugmode = !debugmode;
		break;
	case SDLK_e:
		spotlight2.pos = spotlight1.pos;
		spotlight2.look_at = spotlight2.pos - (spotlight1.pos - spotlight1.look_at);
		break;
	case SDLK_r:
		spotlight2.look_at = spotlight1.look_at;
		break;
	case SDLK_UP:
	    spotlight1.move(glm::vec3(0, 0, -1));
	    break;
	case SDLK_DOWN:
	    spotlight1.move(glm::vec3(0, 0, 1));
	    break;
	case SDLK_LEFT:
	    spotlight1.move(glm::vec3(-1, 0, 0));
	    break;
	case SDLK_RIGHT:
	    spotlight1.move(glm::vec3(1, 0, 0));
	    break;
	case SDLK_KP_PLUS:
	    spotlight1.move(glm::vec3(0, 1, 0));
	    break;
	case SDLK_KP_MINUS:
	    spotlight1.move(glm::vec3(0, -1, 0));
	    break;
	case SDLK_KP_8:
		spotlight2.move(glm::vec3(0, 0, -1));
		break;
	case SDLK_KP_2:
		spotlight2.move(glm::vec3(0, 0, 1));
		break;
	case SDLK_KP_4:
		spotlight2.move(glm::vec3(-1, 0, 0));
		break;
	case SDLK_KP_6:
		spotlight2.move(glm::vec3(1, 0, 0));
		break;
	case SDLK_KP_7:
		spotlight2.move(glm::vec3(0, 1, 0));
		break;
	case SDLK_KP_1:
		spotlight2.move(glm::vec3(0, -1, 0));
		break;
	// Print camera position for debugging
	case SDLK_p:
	    std::cout << "Camera position: " << cam.position.x << ", " << cam.position.y << ", " << cam.position.z << std::endl;
	    break;
	// Print spotlight positions for debugging
	case SDLK_l:
	    std::cout << "Spotlight1 position: " << spotlight1.pos.x << ", " << spotlight1.pos.y << ", " << spotlight1.pos.z << std::endl;
	    break;
	case SDLK_k:
		std::cout << "Spotlight2 position: " << spotlight2.pos.x << ", " << spotlight2.pos.y << ", " << spotlight2.pos.z << std::endl;
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

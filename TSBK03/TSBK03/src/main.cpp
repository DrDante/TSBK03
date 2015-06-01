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
#include <vector>

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
unsigned int lightbulbs = 6;																			// Antalet glödlampor (samma som ovan)

Thing lightsrc;																							// Ljuskälla (debug endast)
Thing m_bedside_lamp_lightbulb, m_bedside_lamp_lightbulb2;												// Glödlampor.
Thing m_desk_lamp_lightbulb, m_Hektar_light_bulb, m_ceiling_lamp_lightbulb1, m_ceiling_lamp_lightbulb2;	// Glödlampor.
Thing m_bamboo1, m_bamboo2, m_bamboo3, m_bamboo4, m_bamboo5, m_bamboo6;									// Bamboo, etc.
Thing m_bamboo_leaf, m_earth, m_flower_pot;																// Bamboo, etc.
Thing m_bed, m_mattress, m_sheet_pillow;																// Bed, etc.
Thing m_bedside_lamp, m_bedside_lamp_shade;																// Bedside table 1 lamp.
Thing m_bedside_table, m_bedside_table_handle, m_bedside_table_doors;									// Bedside table 1, etc.
Thing m_bedside_lamp2, m_bedside_lamp_shade2;															// Bedside table 2 lamp.
Thing m_bedside_table2, m_bedside_table_handle2, m_bedside_table_doors2;								// Bedside table 2, etc.
Thing m_books, m_bookshelf;																				// Bookshelf, etc.
Thing m_ceiling_lamp1, m_ceiling_lamp2;																	// Taklampor.
Thing m_chair_back, m_chair_legs, m_chair_lower_part, m_chair_seat, m_chair_wheels;						// Chair. (LOTS OF POLYGONS)
Thing m_desk, m_desk_cap, m_desk_cap_ring, m_desk_handle;												// Desk, etc.
Thing m_desk_lamp, m_desk_lamp_bulb_holder, m_desk_lamp_shade;											// Desk lamp.
Thing m_door, m_door_frame, m_door_handle, m_door_keyhole;												// Door.
Thing m_floor, m_ceiling, m_walls;																		// Golv, väggar.
Thing m_floor2, m_ceiling2, m_walls2;																	// Rum 2.
Thing m_TV, m_TV_granite, m_TV_screen, m_TV_table;														// TV, etc.
Thing m_window_handle, m_windows;																		// Windows.
Thing m_Hektar_lamp_shade, m_Hektar_lamp_stand, m_Hektar_light_bulb_stand;								// Hektar Ikea lampa.
Thing m_laptop, m_laptop_screen;																		// Laptop
float scl = 6;
std::vector<Thing> objlist;

// Texturer
GLuint t_bedside_lamp_lightbulb, t_bedside_lamp_lightbulb2;
GLuint t_desk_lamp_lightbulb, t_Hektar_light_bulb, t_ceiling_lamp_lightbulb, t_ceiling_lamp_lightbulb2;
GLuint t_bamboo1, t_bamboo2, t_bamboo3, t_bamboo4, t_bamboo5, t_bamboo6;
GLuint t_bamboo_leaf, t_earth, t_flower_pot;
GLuint t_bed, t_mattress, t_sheet_pillow;
GLuint t_bedside_lamp, t_bedside_lamp_shade;
GLuint t_bedside_table, t_bedside_table_handle, t_bedside_table_doors;
GLuint t_bedside_lamp2, t_bedside_lamp_shade2;
GLuint t_bedside_table2, t_bedside_table_handle2, t_bedside_table_doors2;
GLuint t_books, t_bookshelf;
GLuint t_ceiling_lamp1, t_ceiling_lamp2;
GLuint t_chair_back, t_chair_legs, t_chair_lower_part, t_chair_seat, t_chair_wheels;
GLuint t_desk, t_desk_cap, t_desk_cap_ring, t_desk_handle;
GLuint t_desk_lamp, t_desk_lamp_bulb_holder, t_desk_lamp_shade;
GLuint t_door, t_door_frame, t_door_handle, t_door_keyhole;
GLuint t_floor, t_ceiling, t_walls;
GLuint t_floor2, t_ceiling2, t_walls2;
GLuint t_TV, t_TV_granite, t_TV_screen, t_TV_table;
GLuint t_window_handle, t_windows;
GLuint t_Hektar_lamp_shade, t_Hektar_lamp_stand, t_Hektar_light_bulb_stand;
GLuint t_laptop, t_laptop_screen;
std::vector<GLuint> texlist;

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

// FBO
FBOstruct *z_fbo, *tmp_fbo, *res_fbo;

// Shaders.
GLuint shadowphongshader = 0, zshader = 0, addshader = 0, plainshader = 0;
// Skärmstorlek
int width = 1024;
int height = 768;

// Bias som används för att undvika skuggacne, används i shadowphong.frag
float bias = 0.0001;

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

lightSource sunlight(glm::vec3(-28.6608 * scl, 14.3395 * scl, 13.0906 * scl), true, glm::vec3(-5.28038 * scl, -6.90153 * scl, -15.4778 * scl));
bool draw1 = true;
lightSource bedlight1(glm::vec3(-6.39886 * scl, -3.91051 * scl, 18.4094 * scl), true, glm::vec3(-6.39886 * scl, -7.15032 * scl, 18.4093 * scl));
lightSource bedlight2(glm::vec3(-6.39886 * scl, -3.91051 * scl, 18.4094 * scl), true, glm::vec3(-6.39886 * scl, 7.15032 * scl, 18.4093 * scl));
bool draw2 = true;
float bedlightsXdist = 20.77;
lightSource bedlight21(glm::vec3((-6.39886 + bedlightsXdist) * scl, -3.91051 * scl, 18.4094 * scl), true, glm::vec3((-6.39886 + bedlightsXdist) * scl, -7.15032 * scl, 18.4093 * scl));
lightSource bedlight22(glm::vec3((-6.39886 + bedlightsXdist) * scl, -3.91051 * scl, 18.4094 * scl), true, glm::vec3((-6.39886 + bedlightsXdist) * scl, 7.15032 * scl, 18.4093 * scl));
bool draw3 = true;
lightSource desklamp(glm::vec3(21.5796 * scl, 1.4167 * scl, -17.2829 * scl), true, glm::vec3(22.8042 * scl, -2.43597 * scl, -17.2353 * scl));
bool draw4 = true;
lightSource hektarlight(glm::vec3(35.9581 * scl, 4.17864 * scl, 17.8685 * scl), true, glm::vec3(22.7725 * scl, -6.70588 * scl, 1.33307 * scl));
bool draw5 = true;
lightSource ceillight11(glm::vec3(19.9485 * scl, 14.0732 * scl, 1.07951 * scl), true, glm::vec3(27.4963 * scl, 6.00726 * scl, 8.9098 * scl));
lightSource ceillight12(glm::vec3(22.1733 * scl, 14.0158 * scl, 0.248075 * scl), true, glm::vec3(24.9946 * scl, 2.9704 * scl, -12.6847 * scl));
lightSource ceillight13(glm::vec3(20.1561 * scl, 14.0178 * scl, -0.999484 * scl), true, glm::vec3(14.1301 * scl, 1.02795 * scl, 8.77931 * scl));
bool draw6 = true;

lightSource ceillight21(glm::vec3(1.40355 * scl, 14.4348 * scl, 1.05177 * scl), true, glm::vec3(7.80346 * scl, 12.7173 * scl, -0.180466 * scl));
bool draw7 = true;
lightSource ceillight22(glm::vec3(0.916115 * scl, 13.9485 * scl, -1.07603 * scl), true, glm::vec3(0.819954 * scl, 6.30125 * scl, -8.91277 * scl));
bool draw8 = true;
lightSource ceillight23(glm::vec3(-0.925582 * scl, 14.0328 * scl, 0.92627 * scl), true, glm::vec3(2.81801 * scl, 10.0247 * scl, 6.17313 * scl));
bool draw9 = true;

lightSource flashlight(glm::vec3(-16.405 * scl, 12.6004 * scl, -17.4967 * scl), true, glm::vec3(3.89842 * scl, -1.50879 * scl, 4.35974 * scl));
bool drawf = false;
bool debugmode = false;

bool isDoorRotating = false;
bool isDoorClosed = true;
int doorObjPos;
unsigned int leafObjPos;
glm::vec3 totalDoorTrans;
int doorAngleMult = 0;
const float doorAngle = float(-2.0 * PI / 3.0);
const float doorAngleIncr = float(-PI / 90.0);
const float doorAngleMultLim = doorAngle / doorAngleIncr;

// --------------------Function declarations--------------------
void OnTimer(int value);
void reshape(int w, int h, glm::mat4 &projectionMatrix);
void idle();
void RotateDoor();
void fbo_add_tmp_to_res();
void draw_scene(lightSource light);
void draw_order(lightSource light);
void draw_order(lightSource light, glm::mat4 textureMatrix);
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
    glClearColor(0.0, 0.0, 0.0, 0);
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

    // Ladda upp hur stor ändring i texturen en pixel är
    glUseProgram(shadowphongshader);
    glm::vec2 pixelDiff = glm::vec2(1.0/SHADOW_W, 1.0/SHADOW_H);
    glUniform2f(glGetUniformLocation(shadowphongshader, "pixelDiff"), pixelDiff.x, pixelDiff.y);

    printError("init shader");


    // Texturer
    LoadTGATextureSimple("objects/textures/tga/bedside_lamp_lightbulb.tga", &t_bedside_lamp_lightbulb);
    LoadTGATextureSimple("objects/textures/tga/bedside_lamp_lightbulb2.tga", &t_bedside_lamp_lightbulb2);
    LoadTGATextureSimple("objects/textures/tga/white.tga", &t_desk_lamp_lightbulb);								// WHITE
    LoadTGATextureSimple("objects/textures/tga/Hektar_light_bulb.tga", &t_Hektar_light_bulb);
    LoadTGATextureSimple("objects/textures/tga/ceiling_lamp_lightbulb.tga", &t_ceiling_lamp_lightbulb);
    LoadTGATextureSimple("objects/textures/tga/ceiling_lamp_lightbulb2.tga", &t_ceiling_lamp_lightbulb2);

    LoadTGATextureSimple("objects/textures/tga/bed.tga", &t_bed);
    LoadTGATextureSimple("objects/textures/tga/bedside_table.tga", &t_bedside_table);
    LoadTGATextureSimple("objects/textures/tga/bedside_table_handle.tga", &t_bedside_table_handle);
    LoadTGATextureSimple("objects/textures/tga/bedside_table_doors.tga", &t_bedside_table_doors);
    LoadTGATextureSimple("objects/textures/tga/bedside_lamp.tga", &t_bedside_lamp);
    LoadTGATextureSimple("objects/textures/tga/bedside_lamp_shade.tga", &t_bedside_lamp_shade);
    LoadTGATextureSimple("objects/textures/tga/bedside_table2.tga", &t_bedside_table2);
    LoadTGATextureSimple("objects/textures/tga/bedside_table_handle2.tga", &t_bedside_table_handle2);
    LoadTGATextureSimple("objects/textures/tga/bedside_table_doors2.tga", &t_bedside_table_doors2);
    LoadTGATextureSimple("objects/textures/tga/bedside_lamp2.tga", &t_bedside_lamp2);
    LoadTGATextureSimple("objects/textures/tga/bedside_lamp_shade2.tga", &t_bedside_lamp_shade2);
    LoadTGATextureSimple("objects/textures/tga/mattress.tga", &t_mattress);
    LoadTGATextureSimple("objects/textures/tga/sheet_pillow.tga", &t_sheet_pillow);
    LoadTGATextureSimple("objects/textures/tga/bamboo1.tga", &t_bamboo1);
    LoadTGATextureSimple("objects/textures/tga/bamboo2.tga", &t_bamboo2);
    LoadTGATextureSimple("objects/textures/tga/bamboo3.tga", &t_bamboo3);
    LoadTGATextureSimple("objects/textures/tga/bamboo4.tga", &t_bamboo4);
    LoadTGATextureSimple("objects/textures/tga/bamboo5.tga", &t_bamboo5);
    LoadTGATextureSimple("objects/textures/tga/bamboo6.tga", &t_bamboo6);
    LoadTGATextureSimple("objects/textures/tga/bamboo_leaf.tga", &t_bamboo_leaf);
    LoadTGATextureSimple("objects/textures/tga/flower_pot.tga", &t_flower_pot);
    LoadTGATextureSimple("objects/textures/tga/earth.tga", &t_earth);
    LoadTGATextureSimple("objects/textures/tga/books.tga", &t_books);
    LoadTGATextureSimple("objects/textures/tga/bookshelf.tga", &t_bookshelf);
    LoadTGATextureSimple("objects/textures/tga/ceiling_lamp1.tga", &t_ceiling_lamp1);
    LoadTGATextureSimple("objects/textures/tga/ceiling_lamp2.tga", &t_ceiling_lamp2);
    LoadTGATextureSimple("objects/textures/tga/chair_back.tga", &t_chair_back);
    LoadTGATextureSimple("objects/textures/tga/chair_legs.tga", &t_chair_legs);
    LoadTGATextureSimple("objects/textures/tga/chair_lower_part.tga", &t_chair_lower_part);
    LoadTGATextureSimple("objects/textures/tga/chair_seat.tga", &t_chair_seat);
    LoadTGATextureSimple("objects/textures/tga/chair_wheels.tga", &t_chair_wheels);
    LoadTGATextureSimple("objects/textures/tga/desk.tga", &t_desk);
    LoadTGATextureSimple("objects/textures/tga/desk_cap.tga", &t_desk_cap);
    LoadTGATextureSimple("objects/textures/tga/desk_cap_ring.tga", &t_desk_cap_ring);
    LoadTGATextureSimple("objects/textures/tga/desk_handle.tga", &t_desk_handle);
    LoadTGATextureSimple("objects/textures/tga/desk_lamp.tga", &t_desk_lamp);
    LoadTGATextureSimple("objects/textures/tga/desk_lamp_bulb_holder.tga", &t_desk_lamp_bulb_holder);
    LoadTGATextureSimple("objects/textures/tga/desk_lamp_shade.tga", &t_desk_lamp_shade);
    LoadTGATextureSimple("objects/textures/tga/door.tga", &t_door);
    LoadTGATextureSimple("objects/textures/tga/door_frame.tga", &t_door_frame);
    LoadTGATextureSimple("objects/textures/tga/door_handle.tga", &t_door_handle);
    LoadTGATextureSimple("objects/textures/tga/door_keyhole.tga", &t_door_keyhole);
    LoadTGATextureSimple("objects/textures/tga/floor.tga", &t_floor);
    LoadTGATextureSimple("objects/textures/tga/ceiling.tga", &t_ceiling);
    LoadTGATextureSimple("objects/textures/tga/white.tga", &t_walls);											// WHITE
    LoadTGATextureSimple("objects/textures/tga/floor2.tga", &t_floor2);
    LoadTGATextureSimple("objects/textures/tga/ceiling2.tga", &t_ceiling2);
    LoadTGATextureSimple("objects/textures/tga/walls2.tga", &t_walls2);
    LoadTGATextureSimple("objects/textures/tga/Hektar_lamp_shade.tga", &t_Hektar_lamp_shade);
    LoadTGATextureSimple("objects/textures/tga/Hektar_lamp_stand.tga", &t_Hektar_lamp_stand);
    LoadTGATextureSimple("objects/textures/tga/Hektar_light_bulb_stand.tga", &t_Hektar_light_bulb_stand);
    LoadTGATextureSimple("objects/textures/tga/TV.tga", &t_TV);
    LoadTGATextureSimple("objects/textures/tga/TV_granite.tga", &t_TV_granite);
    LoadTGATextureSimple("objects/textures/tga/dark_gray.tga", &t_TV_screen);									// DARK GRAY
    LoadTGATextureSimple("objects/textures/tga/TV_table.tga", &t_TV_table);
    LoadTGATextureSimple("objects/textures/tga/white.tga", &t_windows);											// WHITE
    LoadTGATextureSimple("objects/textures/tga/white.tga", &t_window_handle);									// WHITE
    LoadTGATextureSimple("objects/textures/tga/laptop.tga", &t_laptop);
    LoadTGATextureSimple("objects/textures/tga/screen.tga", &t_laptop_screen);
    // --------

    // Laddning av modeller.

    squareModel = LoadDataToModel(
            square, NULL, squareTexCoord, NULL,
            squareIndices, 4, 6);

    lightsrc = Thing("objects/sphere.obj");
    // --- Julius modeller ---
    glm::vec3 sceneSize = glm::vec3(10 * scl, 10 * scl, 10 * scl);
    glm::vec3 sceneTrans = glm::vec3(0 * scl, -10 * scl, 0 * scl);
    glm::vec3 bambooTrans = glm::vec3(-16.2246 * scl, 0 * scl, -17.2567 * scl);
    glm::vec3 doorTrans = glm::vec3(28.0168 * scl, 0 * scl, 20.2156 * scl);
    totalDoorTrans = sceneTrans + doorTrans;

    lightsrc.MTWmatrix = glm::scale(glm::mat4(), sceneSize * scl);

    m_bedside_lamp_lightbulb = Thing("objects/light_bulb.obj");
    m_bedside_lamp_lightbulb2 = Thing("objects/light_bulb2.obj");
    m_desk_lamp_lightbulb = Thing("objects/desk_lamp_light_bulb.obj");
    m_Hektar_light_bulb = Thing("objects/Hektar_light_bulb.obj");
    m_ceiling_lamp_lightbulb1 = Thing("objects/ceiling_lamp_lightbulbs.obj");
    m_ceiling_lamp_lightbulb2 = Thing("objects/ceiling_lamp_lightbulbs2.obj");

    m_bed = Thing("objects/bed.obj");
    m_bedside_table = Thing("objects/bedside_table.obj");
    m_bedside_table_handle = Thing("objects/bedside_table_handle.obj");
    m_bedside_table_doors = Thing("objects/bedside_table_doors.obj");
    m_bedside_lamp = Thing("objects/bedside_lamp.obj");
    m_bedside_lamp_shade = Thing("objects/bedside_lamp_shade.obj");
    m_bedside_table2 = Thing("objects/bedside_table2.obj");
    m_bedside_table_handle2 = Thing("objects/bedside_table_handle2.obj");
    m_bedside_table_doors2 = Thing("objects/bedside_table_doors2.obj");
    m_bedside_lamp2 = Thing("objects/bedside_lamp2.obj");
    m_bedside_lamp_shade2 = Thing("objects/bedside_lamp_shade2.obj");
    m_mattress = Thing("objects/mattress.obj");
    m_sheet_pillow = Thing("objects/sheet_pillow.obj");
    m_bamboo1 = Thing("objects/bamboo1.obj");
    m_bamboo2 = Thing("objects/bamboo2.obj");
    m_bamboo3 = Thing("objects/bamboo3.obj");
    m_bamboo4 = Thing("objects/bamboo4.obj");
    m_bamboo5 = Thing("objects/bamboo5.obj");
    m_bamboo6 = Thing("objects/bamboo6.obj");
    m_bamboo_leaf = Thing("objects/bamboo_leaf.obj");
    m_flower_pot = Thing("objects/flower_pot.obj");
    m_earth = Thing("objects/earth.obj");
    m_books = Thing("objects/books.obj");
    m_bookshelf = Thing("objects/bookshelf.obj");
    m_ceiling_lamp1 = Thing("objects/ceiling_lamp.obj");
    m_ceiling_lamp2 = Thing("objects/ceiling_lamp2.obj");
    m_chair_back = Thing("objects/chair_back.obj");
    m_chair_legs = Thing("objects/chair_legs.obj");
    m_chair_lower_part = Thing("objects/chair_lower_part.obj");
    m_chair_seat = Thing("objects/chair_seat.obj");
    m_chair_wheels = Thing("objects/chair_wheels.obj");
    m_desk = Thing("objects/desk.obj");
    m_desk_cap = Thing("objects/desk_cap.obj");
    m_desk_cap_ring = Thing("objects/desk_cap_ring.obj");
    m_desk_handle = Thing("objects/desk_handle.obj");
    m_desk_lamp = Thing("objects/desk_lamp.obj");
    m_desk_lamp_bulb_holder = Thing("objects/desk_lamp_bulb_holder.obj");
    m_desk_lamp_shade = Thing("objects/desk_lamp_shade.obj");
    m_door = Thing("objects/door.obj");
    m_door_frame = Thing("objects/door_frame.obj");
    m_door_handle = Thing("objects/door_handle.obj");
    m_door_keyhole = Thing("objects/door_keyhole.obj");
    m_floor = Thing("objects/floor.obj");
    m_ceiling = Thing("objects/ceiling.obj");
    m_walls = Thing("objects/walls.obj");
    m_floor2 = Thing("objects/floor2.obj");
    m_ceiling2 = Thing("objects/ceiling2.obj");
    m_walls2 = Thing("objects/walls2.obj");
    m_Hektar_lamp_shade = Thing("objects/Hektar_lamp_shade.obj");
    m_Hektar_lamp_stand = Thing("objects/Hektar_lamp_stand.obj");
    m_Hektar_light_bulb_stand = Thing("objects/Hektar_light_bulb_stand.obj");
    m_TV = Thing("objects/TV.obj");
    m_TV_granite = Thing("objects/TV_granite.obj");
    m_TV_screen = Thing("objects/TV_screen.obj");
    m_TV_table = Thing("objects/TV_table.obj");
    m_window_handle = Thing("objects/window_handle.obj");
    m_windows = Thing("objects/windows.obj");
    m_laptop = Thing("objects/laptop.obj");
    m_laptop_screen = Thing("objects/laptop_screen.obj");;
    // -----------------------


    // Initiell placering och skalning av modeller.
    m_bedside_lamp_lightbulb.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_lamp_lightbulb.MTWmatrix = glm::translate(sceneTrans) * m_bedside_lamp_lightbulb.MTWmatrix;
    m_bedside_lamp_lightbulb2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_lamp_lightbulb2.MTWmatrix = glm::translate(sceneTrans) * m_bedside_lamp_lightbulb2.MTWmatrix;
    m_desk_lamp_lightbulb.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_lamp_lightbulb.MTWmatrix = glm::translate(sceneTrans) * m_desk_lamp_lightbulb.MTWmatrix;
    m_Hektar_light_bulb.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_Hektar_light_bulb.MTWmatrix = glm::translate(sceneTrans) * m_Hektar_light_bulb.MTWmatrix;
    m_ceiling_lamp_lightbulb1.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_ceiling_lamp_lightbulb1.MTWmatrix = glm::translate(sceneTrans) * m_ceiling_lamp_lightbulb1.MTWmatrix;
    m_ceiling_lamp_lightbulb2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_ceiling_lamp_lightbulb2.MTWmatrix = glm::translate(sceneTrans) * m_ceiling_lamp_lightbulb2.MTWmatrix;

    m_bed.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bed.MTWmatrix = glm::translate(sceneTrans) * m_bed.MTWmatrix;
    m_bedside_table.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_table.MTWmatrix = glm::translate(sceneTrans) * m_bedside_table.MTWmatrix;
    m_bedside_table_handle.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_table_handle.MTWmatrix = glm::translate(sceneTrans) * m_bedside_table_handle.MTWmatrix;
    m_bedside_table_doors.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_table_doors.MTWmatrix = glm::translate(sceneTrans) * m_bedside_table_doors.MTWmatrix;
    m_bedside_lamp.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_lamp.MTWmatrix = glm::translate(sceneTrans) * m_bedside_lamp.MTWmatrix;
    m_bedside_lamp_shade.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_lamp_shade.MTWmatrix = glm::translate(sceneTrans) * m_bedside_lamp_shade.MTWmatrix;
    m_bedside_table2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_table2.MTWmatrix = glm::translate(sceneTrans) * m_bedside_table2.MTWmatrix;
    m_bedside_table_handle2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_table_handle2.MTWmatrix = glm::translate(sceneTrans) * m_bedside_table_handle2.MTWmatrix;
    m_bedside_table_doors2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_table_doors2.MTWmatrix = glm::translate(sceneTrans) * m_bedside_table_doors2.MTWmatrix;
    m_bedside_lamp2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_lamp2.MTWmatrix = glm::translate(sceneTrans) * m_bedside_lamp2.MTWmatrix;
    m_bedside_lamp_shade2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bedside_lamp_shade2.MTWmatrix = glm::translate(sceneTrans) * m_bedside_lamp_shade2.MTWmatrix;
    m_bamboo1.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo1.MTWmatrix = glm::translate(sceneTrans) * m_bamboo1.MTWmatrix;
    m_bamboo1.MTWmatrix = glm::translate(bambooTrans) * m_bamboo1.MTWmatrix;
    m_bamboo2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo2.MTWmatrix = glm::translate(sceneTrans) * m_bamboo2.MTWmatrix;
    m_bamboo2.MTWmatrix = glm::translate(bambooTrans) * m_bamboo2.MTWmatrix;
    m_bamboo3.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo3.MTWmatrix = glm::translate(sceneTrans) * m_bamboo3.MTWmatrix;
    m_bamboo3.MTWmatrix = glm::translate(bambooTrans) * m_bamboo3.MTWmatrix;
    m_bamboo4.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo4.MTWmatrix = glm::translate(sceneTrans) * m_bamboo4.MTWmatrix;
    m_bamboo4.MTWmatrix = glm::translate(bambooTrans) * m_bamboo4.MTWmatrix;
    m_bamboo5.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo5.MTWmatrix = glm::translate(sceneTrans) * m_bamboo5.MTWmatrix;
    m_bamboo5.MTWmatrix = glm::translate(bambooTrans) * m_bamboo5.MTWmatrix;
    m_bamboo6.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo6.MTWmatrix = glm::translate(sceneTrans) * m_bamboo6.MTWmatrix;
    m_bamboo6.MTWmatrix = glm::translate(bambooTrans) * m_bamboo6.MTWmatrix;
    m_bamboo_leaf.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bamboo_leaf.MTWmatrix = glm::translate(sceneTrans) * m_bamboo_leaf.MTWmatrix;
    m_flower_pot.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_flower_pot.MTWmatrix = glm::translate(sceneTrans) * m_flower_pot.MTWmatrix;
    m_earth.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_earth.MTWmatrix = glm::translate(sceneTrans) * m_earth.MTWmatrix;
    m_mattress.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_mattress.MTWmatrix = glm::translate(sceneTrans) * m_mattress.MTWmatrix;
    m_sheet_pillow.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_sheet_pillow.MTWmatrix = glm::translate(sceneTrans) * m_sheet_pillow.MTWmatrix;
    m_books.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_books.MTWmatrix = glm::translate(sceneTrans) * m_books.MTWmatrix;
    m_bookshelf.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_bookshelf.MTWmatrix = glm::translate(sceneTrans) * m_bookshelf.MTWmatrix;
    m_ceiling_lamp1.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_ceiling_lamp1.MTWmatrix = glm::translate(sceneTrans) * m_ceiling_lamp1.MTWmatrix;
    m_ceiling_lamp2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_ceiling_lamp2.MTWmatrix = glm::translate(sceneTrans) * m_ceiling_lamp2.MTWmatrix;
    m_chair_back.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_chair_back.MTWmatrix = glm::translate(sceneTrans) * m_chair_back.MTWmatrix;
    m_chair_legs.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_chair_legs.MTWmatrix = glm::translate(sceneTrans) * m_chair_legs.MTWmatrix;
    m_chair_lower_part.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_chair_lower_part.MTWmatrix = glm::translate(sceneTrans) * m_chair_lower_part.MTWmatrix;
    m_chair_seat.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_chair_seat.MTWmatrix = glm::translate(sceneTrans) * m_chair_seat.MTWmatrix;
    m_chair_wheels.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_chair_wheels.MTWmatrix = glm::translate(sceneTrans) * m_chair_wheels.MTWmatrix;
    m_desk.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk.MTWmatrix = glm::translate(sceneTrans) * m_desk.MTWmatrix;
    m_desk_cap.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_cap.MTWmatrix = glm::translate(sceneTrans) * m_desk_cap.MTWmatrix;
    m_desk_cap_ring.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_cap_ring.MTWmatrix = glm::translate(sceneTrans) * m_desk_cap_ring.MTWmatrix;
    m_desk_handle.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_handle.MTWmatrix = glm::translate(sceneTrans) * m_desk_handle.MTWmatrix;
    m_desk_lamp.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_lamp.MTWmatrix = glm::translate(sceneTrans) * m_desk_lamp.MTWmatrix;
    m_desk_lamp_bulb_holder.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_lamp_bulb_holder.MTWmatrix = glm::translate(sceneTrans) * m_desk_lamp_bulb_holder.MTWmatrix;
    m_desk_lamp_shade.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_desk_lamp_shade.MTWmatrix = glm::translate(sceneTrans) * m_desk_lamp_shade.MTWmatrix;
    m_door.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_door.MTWmatrix = glm::translate(sceneTrans) * m_door.MTWmatrix;
    m_door.MTWmatrix = glm::translate(doorTrans) * m_door.MTWmatrix;
    m_door_frame.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_door_frame.MTWmatrix = glm::translate(sceneTrans) * m_door_frame.MTWmatrix;
    m_door_handle.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_door_handle.MTWmatrix = glm::translate(sceneTrans) * m_door_handle.MTWmatrix;
    m_door_handle.MTWmatrix = glm::translate(doorTrans) * m_door_handle.MTWmatrix;
    m_door_keyhole.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_door_keyhole.MTWmatrix = glm::translate(sceneTrans) * m_door_keyhole.MTWmatrix;
    m_door_keyhole.MTWmatrix = glm::translate(doorTrans) * m_door_keyhole.MTWmatrix;
    m_floor.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_floor.MTWmatrix = glm::translate(sceneTrans) * m_floor.MTWmatrix;
    m_ceiling.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_ceiling.MTWmatrix = glm::translate(sceneTrans) * m_ceiling.MTWmatrix;
    m_walls.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_walls.MTWmatrix = glm::translate(sceneTrans) * m_walls.MTWmatrix;
    m_floor2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_floor2.MTWmatrix = glm::translate(sceneTrans) * m_floor2.MTWmatrix;
    m_ceiling2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_ceiling2.MTWmatrix = glm::translate(sceneTrans) * m_ceiling2.MTWmatrix;
    m_walls2.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_walls2.MTWmatrix = glm::translate(sceneTrans) * m_walls2.MTWmatrix;
    m_Hektar_lamp_shade.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_Hektar_lamp_shade.MTWmatrix = glm::translate(sceneTrans) * m_Hektar_lamp_shade.MTWmatrix;
    m_Hektar_light_bulb_stand.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_Hektar_light_bulb_stand.MTWmatrix = glm::translate(sceneTrans) * m_Hektar_light_bulb_stand.MTWmatrix;
    m_Hektar_lamp_stand.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_Hektar_lamp_stand.MTWmatrix = glm::translate(sceneTrans) * m_Hektar_lamp_stand.MTWmatrix;
    m_TV.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_TV.MTWmatrix = glm::translate(sceneTrans) * m_TV.MTWmatrix;
    m_TV_granite.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_TV_granite.MTWmatrix = glm::translate(sceneTrans) * m_TV_granite.MTWmatrix;
    m_TV_screen.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_TV_screen.MTWmatrix = glm::translate(sceneTrans) * m_TV_screen.MTWmatrix;
    m_TV_table.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_TV_table.MTWmatrix = glm::translate(sceneTrans) * m_TV_table.MTWmatrix;
    m_window_handle.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_window_handle.MTWmatrix = glm::translate(sceneTrans) * m_window_handle.MTWmatrix;
    m_windows.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_windows.MTWmatrix = glm::translate(sceneTrans) * m_windows.MTWmatrix;
    m_laptop.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_laptop.MTWmatrix = glm::translate(sceneTrans) * m_laptop.MTWmatrix;
    m_laptop_screen.MTWmatrix = glm::scale(glm::mat4(), sceneSize);
    m_laptop_screen.MTWmatrix = glm::translate(sceneTrans) * m_laptop_screen.MTWmatrix;

    // Texturlista
    texlist.push_back(t_bedside_lamp_lightbulb);
    texlist.push_back(t_bedside_lamp_lightbulb2);
    texlist.push_back(t_desk_lamp_lightbulb);
    texlist.push_back(t_Hektar_light_bulb);
    texlist.push_back(t_ceiling_lamp_lightbulb);
    texlist.push_back(t_ceiling_lamp_lightbulb2);

    texlist.push_back(t_bed);
    texlist.push_back(t_bedside_table);
    texlist.push_back(t_bedside_table_handle);
    texlist.push_back(t_bedside_table_doors);
    texlist.push_back(t_bedside_lamp);
    texlist.push_back(t_bedside_lamp_shade);
    texlist.push_back(t_bedside_table2);
    texlist.push_back(t_bedside_table_handle2);
    texlist.push_back(t_bedside_table_doors2);
    texlist.push_back(t_bedside_lamp2);
    texlist.push_back(t_bedside_lamp_shade2);
    texlist.push_back(t_bamboo1);
    texlist.push_back(t_bamboo2);
    texlist.push_back(t_bamboo3);
    texlist.push_back(t_bamboo4);
    texlist.push_back(t_bamboo5);
    texlist.push_back(t_bamboo6);
    texlist.push_back(t_bamboo_leaf);
    texlist.push_back(t_flower_pot);
    texlist.push_back(t_earth);
    texlist.push_back(t_mattress);
    texlist.push_back(t_sheet_pillow);
    texlist.push_back(t_books);
    texlist.push_back(t_bookshelf);
    texlist.push_back(t_ceiling_lamp1);
    texlist.push_back(t_ceiling_lamp2);
    texlist.push_back(t_chair_back);
    texlist.push_back(t_chair_legs);
    texlist.push_back(t_chair_lower_part);
    texlist.push_back(t_chair_seat);
    texlist.push_back(t_chair_wheels);
    texlist.push_back(t_desk);
    texlist.push_back(t_desk_cap);
    texlist.push_back(t_desk_cap_ring);
    texlist.push_back(t_desk_handle);
    texlist.push_back(t_desk_lamp);
    texlist.push_back(t_desk_lamp_bulb_holder);
    texlist.push_back(t_desk_lamp_shade);
    texlist.push_back(t_door);
    texlist.push_back(t_door_frame);
    texlist.push_back(t_door_handle);
    texlist.push_back(t_door_keyhole);
    texlist.push_back(t_floor);
    texlist.push_back(t_ceiling);
    texlist.push_back(t_walls);
    texlist.push_back(t_floor2);
    texlist.push_back(t_ceiling2);
    texlist.push_back(t_walls2);
    texlist.push_back(t_Hektar_lamp_shade);
    texlist.push_back(t_Hektar_light_bulb_stand);
    texlist.push_back(t_Hektar_lamp_stand);
    texlist.push_back(t_TV);
    texlist.push_back(t_TV_granite);
    texlist.push_back(t_TV_screen);
    texlist.push_back(t_TV_table);
    texlist.push_back(t_window_handle);
    texlist.push_back(t_windows);
    texlist.push_back(t_laptop);
    texlist.push_back(t_laptop_screen);

    // Inladdning av modellerna i objlist. Kommentera ut rader nedan om något objekt ska exkluderas.
    objlist.push_back(m_bedside_lamp_lightbulb);
    objlist.push_back(m_bedside_lamp_lightbulb2);
    objlist.push_back(m_desk_lamp_lightbulb);
    objlist.push_back(m_Hektar_light_bulb);
    objlist.push_back(m_ceiling_lamp_lightbulb1);
    objlist.push_back(m_ceiling_lamp_lightbulb2);

    objlist.push_back(m_bed);
    objlist.push_back(m_bedside_table);
    objlist.push_back(m_bedside_table_handle);
    objlist.push_back(m_bedside_table_doors);
    objlist.push_back(m_bedside_lamp);
    objlist.push_back(m_bedside_lamp_shade);
    objlist.push_back(m_bedside_table2);
    objlist.push_back(m_bedside_table_handle2);
    objlist.push_back(m_bedside_table_doors2);
    objlist.push_back(m_bedside_lamp2);
    objlist.push_back(m_bedside_lamp_shade2);
    objlist.push_back(m_bamboo1);
    objlist.push_back(m_bamboo2);
    objlist.push_back(m_bamboo3);
    objlist.push_back(m_bamboo4);
    objlist.push_back(m_bamboo5);
    objlist.push_back(m_bamboo6);
    objlist.push_back(m_bamboo_leaf);
    leafObjPos = objlist.size() - 1;
    objlist.push_back(m_flower_pot);
    objlist.push_back(m_earth);
    objlist.push_back(m_mattress);
    objlist.push_back(m_sheet_pillow);
    objlist.push_back(m_books);
    objlist.push_back(m_bookshelf);
    objlist.push_back(m_ceiling_lamp1);
    objlist.push_back(m_ceiling_lamp2);
    objlist.push_back(m_chair_back);
    objlist.push_back(m_chair_legs);
    objlist.push_back(m_chair_lower_part);
    objlist.push_back(m_chair_seat);
    objlist.push_back(m_chair_wheels);
    objlist.push_back(m_desk);
    objlist.push_back(m_desk_cap);
    objlist.push_back(m_desk_cap_ring);
    objlist.push_back(m_desk_handle);
    objlist.push_back(m_desk_lamp);
    objlist.push_back(m_desk_lamp_bulb_holder);
    objlist.push_back(m_desk_lamp_shade);
    objlist.push_back(m_door);
    doorObjPos = objlist.size() - 1;
    objlist.push_back(m_door_frame);
    objlist.push_back(m_door_handle);
    objlist.push_back(m_door_keyhole);
    objlist.push_back(m_floor);
    objlist.push_back(m_ceiling);
    objlist.push_back(m_walls);
    objlist.push_back(m_floor2);
    objlist.push_back(m_ceiling2);
    objlist.push_back(m_walls2);
    objlist.push_back(m_Hektar_lamp_shade);
    objlist.push_back(m_Hektar_light_bulb_stand);
    objlist.push_back(m_Hektar_lamp_stand);
    objlist.push_back(m_TV);
    objlist.push_back(m_TV_granite);
    objlist.push_back(m_TV_screen);
    objlist.push_back(m_TV_table);
    objlist.push_back(m_window_handle);
    objlist.push_back(m_windows);
    objlist.push_back(m_laptop);
    objlist.push_back(m_laptop_screen);


    // Scale and bias för shadow map
    scaleBiasMatrix = glm::translate(glm::scale(glm::mat4(), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(1,1,1));

    // Initiell placering och orientering av kamera.
    cam = Camera(zshader, &viewMatrix);

    // SDL för att dölja mus och låsa den till fönstret
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Sätt ljuskällornas projektionsmatrix
    glm::mat4 lightpersp = glm::perspective(float(PI / 2), float(width) / height, 1.0f, 1000.0f);
    sunlight.projectionMatrix = lightpersp;
    bedlight1.projectionMatrix = lightpersp;
    bedlight2.projectionMatrix = lightpersp;
    bedlight21.projectionMatrix = lightpersp;
    bedlight22.projectionMatrix = lightpersp;
    desklamp.projectionMatrix = lightpersp;
    hektarlight.projectionMatrix = lightpersp;
    ceillight11.projectionMatrix = lightpersp;
    ceillight12.projectionMatrix = lightpersp;
    ceillight13.projectionMatrix = lightpersp;
    ceillight21.projectionMatrix = lightpersp;
    ceillight22.projectionMatrix = lightpersp;
    ceillight23.projectionMatrix = lightpersp;

    flashlight.projectionMatrix = lightpersp;
    //spotlight.projectionMatrix = glm::ortho(0.0f, float(width), 0.0f, float(height), 1.0f, 1000.0f);

}

void display(void)
{
    // Rensa framebuffers & z-buffer.
    glClearColor(0.0, 0.0, 0.0, 0);
    useFBO(res_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    useFBO(tmp_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    useFBO(z_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    if (isDoorRotating)
    {
        RotateDoor();
    }

    // Rita ut scenen till z-buffern, sedan med phong till tmp_fbo, och addera till res_fbo.
    if (draw1)
    {
        draw_scene(sunlight);
    }
    if (draw2)
    {
        draw_scene(bedlight1);
        draw_scene(bedlight2);
    }
    if (draw3)
    {
        draw_scene(bedlight21);
        draw_scene(bedlight22);
    }
    if (draw4)
    {
        draw_scene(desklamp);
    }
    if (draw5)
    {
        draw_scene(hektarlight);
    }
    if (draw6)
    {
        draw_scene(ceillight11);
        draw_scene(ceillight12);
        draw_scene(ceillight13);
    }
    if (draw7)
    {
        draw_scene(ceillight21);
    }
    if (draw8)
    {
        draw_scene(ceillight22);
    }
    if (draw9)
    {
        draw_scene(ceillight23);
    }
    if (drawf)
    {
        flashlight.pos = cam.position;
        flashlight.look_at = cam.look_at_pos;
        draw_scene(flashlight);
    }

    // Rita ut ljuskällor till res_fbo.
    if (debugmode)
    {
        if (draw1)
        {
            draw_lights(sunlight);
        }
        if (draw2)
        {
            draw_lights(bedlight1);
            draw_lights(bedlight2);
        }
        if (draw3)
        {
            draw_lights(bedlight1);
            draw_lights(bedlight2);
        }
        if (draw4)
        {
            draw_lights(desklamp);
        }
        if (draw5)
        {
            draw_lights(hektarlight);
        }
        if (draw6)
        {
            draw_lights(ceillight11);
            draw_lights(ceillight12);
            draw_lights(ceillight13);
        }
        if (draw7)
        {
            draw_lights(ceillight21);
        }
        if (draw8)
        {
            draw_lights(ceillight22);
        }
        if (draw9)
        {
            draw_lights(ceillight23);
        }
    }

    // Rita ut res_fbo till skärmen.
    glUseProgram(plainshader);
    useFBO(0L, res_fbo, 0L);
    DrawModel(squareModel, plainshader, "in_Position", NULL, "in_TexCoord");

    swap_buffers();
}

void RotateDoor()
{
    doorAngleMult++;
    Thing door = objlist.at(doorObjPos);
    Thing handle = objlist.at(doorObjPos + 2);
    Thing keyhole = objlist.at(doorObjPos + 3);
    door.MTWmatrix = glm::translate(-totalDoorTrans) * door.MTWmatrix;
    handle.MTWmatrix = glm::translate(-totalDoorTrans) * handle.MTWmatrix;
    keyhole.MTWmatrix = glm::translate(-totalDoorTrans) * keyhole.MTWmatrix;
    if (isDoorClosed)
    {
        door.MTWmatrix = glm::rotate(door.MTWmatrix, doorAngleIncr, glm::vec3(0, 1, 0));
        handle.MTWmatrix = glm::rotate(handle.MTWmatrix, doorAngleIncr, glm::vec3(0, 1, 0));
        keyhole.MTWmatrix = glm::rotate(keyhole.MTWmatrix, doorAngleIncr, glm::vec3(0, 1, 0));
        if (doorAngleMult >= doorAngleMultLim)
        {
            isDoorRotating = false;
            isDoorClosed = false;
            doorAngleMult = 0;
        }
    }
    else
    {
        door.MTWmatrix = glm::rotate(door.MTWmatrix, -doorAngleIncr, glm::vec3(0, 1, 0));
        handle.MTWmatrix = glm::rotate(handle.MTWmatrix, -doorAngleIncr, glm::vec3(0, 1, 0));
        keyhole.MTWmatrix = glm::rotate(keyhole.MTWmatrix, -doorAngleIncr, glm::vec3(0, 1, 0));
        if (doorAngleMult >= doorAngleMultLim)
        {
            isDoorRotating = false;
            isDoorClosed = true;
            doorAngleMult = 0;
        }
    }
    door.MTWmatrix = glm::translate(totalDoorTrans) * door.MTWmatrix;
    handle.MTWmatrix = glm::translate(totalDoorTrans) * handle.MTWmatrix;
    keyhole.MTWmatrix = glm::translate(totalDoorTrans) * keyhole.MTWmatrix;
    objlist.at(doorObjPos) = door;
    objlist.at(doorObjPos + 2) = handle;
    objlist.at(doorObjPos + 3) = keyhole;
}

void fbo_add_tmp_to_res()
{
    // Adds two fbos (tmp_fbo and res_fbo) to res_fbo.
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(addshader);
    useFBO(res_fbo, res_fbo, tmp_fbo);
    glUniform1i(glGetUniformLocation(addshader, "texUnit"), 0);
    glUniform1i(glGetUniformLocation(addshader, "texUnit2"), 1);
    DrawModel(squareModel, addshader, "in_Position", NULL, "in_TexCoord");
}

void draw_scene(lightSource light)
{
    glUseProgram(zshader);
    useFBO(z_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Aktivera z-buffering.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Flytta kamera till ljuskällan.
    glm::vec3 tmp_cam_pos = cam.position;
    glm::vec3 tmp_cam_look_at = cam.look_at_pos;

    cam.position = light.pos;
    cam.look_at_pos = light.look_at;
    cam.update();

    // Uppladdning av matriser och annan data till shadern.
    glUniformMatrix4fv(glGetUniformLocation(zshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(light.projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(zshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // ----------------Scenen renderas till z-buffern ----------------
    draw_order(light);
    // ---------------------------------------------------------------

    glm::mat4 textureMatrix = scaleBiasMatrix * projectionMatrix * viewMatrix;

    // Återställ kameran till ursprungsposition.
    cam.position = tmp_cam_pos;
    cam.look_at_pos = tmp_cam_look_at;
    cam.update();

    glUseProgram(shadowphongshader);
    useFBO(tmp_fbo, z_fbo, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(glGetUniformLocation(shadowphongshader, "VTPMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shadowphongshader, "WTVMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform3fv(glGetUniformLocation(shadowphongshader, "lPos"), 1, glm::value_ptr(light.pos));
    glUniform1f(glGetUniformLocation(shadowphongshader, "bias"), bias);

    // ---------------- Scenen renderas till tmp_fbo ----------------
    glUniform1i(glGetUniformLocation(shadowphongshader, "texUnit"), 0);
    glUniform3fv(glGetUniformLocation(shadowphongshader, "camPos"), 1, glm::value_ptr(cam.position));
    draw_order(light, textureMatrix);
    // --------------------------------------------------------------

    // Scenen adderas till res_fbo.
    fbo_add_tmp_to_res();
}

void draw_order(lightSource light)
{
    for (unsigned int n = 0; n < objlist.size(); n++)
    {
        if (n >= lightbulbs)
        {
            objlist.at(n).draw(zshader);
        }
    }
}

void draw_order(lightSource light, glm::mat4 textureMatrix)
{
    for (unsigned int n = 0; n < objlist.size(); n++)
    {
        if (n == leafObjPos)
        {
            glDisable(GL_CULL_FACE);
            objlist.at(n).draw_with_depthinfo(shadowphongshader, textureMatrix, &texlist.at(n));
            glEnable(GL_CULL_FACE);
        }
        else
        {
            objlist.at(n).draw_with_depthinfo(shadowphongshader, textureMatrix, &texlist.at(n));
        }
    }
}

void draw_lights(lightSource light)
{
    // Rita ut ljuskällor.
    glUseProgram(shadowphongshader);
    useFBO(tmp_fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    lightsrc.MTWmatrix = glm::translate(light.pos);
    lightsrc.draw(shadowphongshader);

    // Addera till res_fbo.
    fbo_add_tmp_to_res();
}

// Timer för uppritande av skärm. Man får inte kalla funktioner här inne.
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

// Hantera event.
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

// Hantera avändardefinierade event.
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

// Hantera knapptryckningar.
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
        case SDLK_3:
            draw3 = !draw3;
            break;
        case SDLK_4:
            draw4 = !draw4;
            break;
        case SDLK_5:
            draw5 = !draw5;
            break;
        case SDLK_6:
            draw6 = !draw6;
            break;
        case SDLK_7:
            draw7 = !draw7;
            break;
        case SDLK_8:
            draw8 = !draw8;
            break;
        case SDLK_9:
            draw9 = !draw9;
            break;
        case SDLK_f:
            drawf = !drawf;
            break;
        case SDLK_z:
            debugmode = !debugmode;
            if (debugmode)
            {
                std::cout << "Debug mode on!\n";
            }
            else
            {
                std::cout << "Debug mode off!\n";
            }
            break;
        case SDLK_x:
            if (!isDoorRotating)
            {
                isDoorRotating = true;
            }
            break;
        case SDLK_UP:
            sunlight.move(glm::vec3(0, 0, -1));
            break;
        case SDLK_DOWN:
            sunlight.move(glm::vec3(0, 0, 1));
            break;
        case SDLK_LEFT:
            sunlight.move(glm::vec3(-1, 0, 0));
            break;
        case SDLK_RIGHT:
            sunlight.move(glm::vec3(1, 0, 0));
            break;
        case SDLK_KP_PLUS:
            sunlight.move(glm::vec3(0, 1, 0));
            break;
        case SDLK_KP_MINUS:
            sunlight.move(glm::vec3(0, -1, 0));
            break;
            // Print camera position for debugging.
        case SDLK_p:
            std::cout << "Camera position: " << cam.position.x / scl << ", " << cam.position.y / scl << ", " << cam.position.z / scl << std::endl;
            break;
            // Print spotlight positions for debugging.
        case SDLK_l:
            std::cout << "sunlight position: " << sunlight.pos.x / scl << ", " << sunlight.pos.y / scl << ", " << sunlight.pos.z / scl << std::endl;
            break;
            // Öka bias.
        case SDLK_b:
            bias += 0.0001;
            std::cout << bias << std::endl;
            break;
            // Minska bias.
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
        cam.forward(0.05*scl*SPEED);
    } else if(keystate[SDL_SCANCODE_S]) {
        cam.forward(-0.05*scl*SPEED);
    }
    if(keystate[SDL_SCANCODE_A]) {
        cam.strafe(0.05*scl*SPEED);
    } else if(keystate[SDL_SCANCODE_D]) {
        cam.strafe(-0.05*scl*SPEED);
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

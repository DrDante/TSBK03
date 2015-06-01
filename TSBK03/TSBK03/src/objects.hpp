#include<glm/glm.hpp>
#include "common/loadobj.h"
#include <string>

#define GLM_FORCE_RADIANS

class Thing
{
    public:
	Thing();
	Thing(std::string object_path);
	~Thing();

	glm::mat4 MTWmatrix{glm::mat4()};

	// Draw to depthbuffer
	void draw(GLuint shader);
	// Draw to depthbuffer, but use external model
	void draw(GLuint shader, Model* ext_model);

	// Draw to screen with depthtexture 
	void draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix);
	void draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix, GLuint *tex);
	void draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix, Model* ext_model);
	Model *model;
};

#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H
#include "common/GL_utilities.h"
#include <iostream>

class ShadowMapFBO
{
    public:
	ShadowMapFBO();
	~ShadowMapFBO();

	bool init(int width, int height);

	void bind_for_writing();
	void bind_for_reading(GLenum TextureUnit);

    private:
	    GLuint m_fbo;
	    GLuint m_shadowMap;
        int width;
        int height;
};

#endif

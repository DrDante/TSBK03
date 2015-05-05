#include "shadow_map.hpp"

FBOstruct *init_z_fbo(int width, int height)
{
    FBOstruct *fbo = (FBOstruct*)malloc(sizeof(FBOstruct));

    fbo->width = width;
    fbo->height = height;

    // create objects
    glGenFramebuffers(1, &fbo->fb); // frame buffer id
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);
    glGenTextures(1, &fbo->texid);
    glBindTexture(GL_TEXTURE_2D, fbo->texid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo->texid, 0);
    glDrawBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
	std::cerr << "ERROR intializing z_fbo: " << width << " x " << height << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}

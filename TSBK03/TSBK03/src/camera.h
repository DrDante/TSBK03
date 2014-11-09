#ifndef CAMERA_H
#define CAMERA_H

#include <GL/gl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    private:
        // x används för att musen inte ska fastna i kanterna på 
        // fönstret
        int x{0};
        int program;
    public:
        Camera(int program, glm::mat4 *matrix);
        Camera();

        glm::vec3 position{0,5,15};
        glm::vec3 look_at_pos{0,0,0};
        glm::vec3 up{0,1,0};

        glm::mat4 *matrix;

        void rotate(char direction, float angle);
        void translate(float dx, float dy, float dz);
        void forward(float d);
        void strafe(float d);
        void update();
        float radius;
        void upload();
        void print_matrix(glm::mat4 m);

        /*************************************************************
         * change_look_at_pos:
         * Tar xrel från MouseMotionEvent och y som absolut koordinat
         * width, height är storlek på nuvarande fönster
         * Sätter look_at_pos därefter
         * **********************************************************/
        void change_look_at_pos(int xrel, int y, int width, int height);
};

#endif

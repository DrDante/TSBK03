#include "camera.h"
#include <cmath>
#include <iostream>
using namespace std;

Camera::Camera(int program, mat4 *matrix)
{
    this->matrix = matrix;
    this->program = program;
    *matrix = lookAtv(position, look_at_pos, up);
    upload();
}

Camera::Camera()
{}


void Camera::upload()
{
    // Upload camera matrix here
    /* glUniformMatrix4fv(glGetUniformLocation(program, "viewMatrix"), 1, GL_TRUE, (*matrix).m); */
    
    cam_position[0] = position.x;
    cam_position[1] = position.y;
    cam_position[2] = position.z;
        
    glUniform3fv(glGetUniformLocation(program, "camPos"), 1, (const GLfloat*) (cam_position));
    
}
void Camera::update()
{
    *matrix = lookAtv(position, look_at_pos, up);
    upload();
}

void Camera::rotate(char direction, float angle)
{
    switch (direction){
        case 'x':
            look_at_pos = Rx(angle) * look_at_pos;
            break;
        case 'y':
            look_at_pos = Ry(angle) * look_at_pos;
            break;
        case 'z':
            look_at_pos = Rz(angle) * look_at_pos;
            break;
    }
    *matrix = lookAtv(position, look_at_pos, up);
    upload();
}

void Camera::translate(float dx, float dy, float dz)
{
    position =  T(dx,dy,dz) * position;
    look_at_pos = T(dx,dy,dz) * look_at_pos;
    *matrix = lookAtv(position, look_at_pos, up);
    upload();
}

void Camera::forward(float d)
{
    vec3 forward_vec = look_at_pos - position;
    forward_vec = d * forward_vec;
    translate(forward_vec.x, forward_vec.y, forward_vec.z);
}

void Camera::strafe(float d)
{
    vec3 strafe_vec = look_at_pos - position;
    strafe_vec = CrossProduct(up, strafe_vec);
    strafe_vec = Normalize(strafe_vec);
    strafe_vec = d * strafe_vec;
    translate(strafe_vec.x, strafe_vec.y, strafe_vec.z);
}

void Camera::change_look_at_pos(int xrel, int y, int width, int height)
{
    // FIXME ful lösning är detta
    if(y==0){
        y = 1;
    }
    x += xrel;
    float fi = ((float)x)/width*2*M_PI;
    float theta = ((float)y)/height*M_PI;

    look_at_pos.x = -sin(theta)*sin(fi) + position.x;
    look_at_pos.y = cos(theta) + position.y;
    look_at_pos.z = sin(theta)*cos(fi) + position.z;
    *matrix = lookAtv(position, look_at_pos, up);
    upload();
}

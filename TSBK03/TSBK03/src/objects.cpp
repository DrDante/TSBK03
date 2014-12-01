#include "objects.hpp"
#include <glm/gtc/type_ptr.hpp>

Thing::Thing()
{
    model = nullptr;
}

Thing::Thing(std::string object_path)
{
    model = LoadModelPlus((char*)object_path.c_str());
}

Thing::~Thing()
{
}

void Thing::draw(GLuint shader)
{
    glUniformMatrix4fv(glGetUniformLocation(shader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(MTWmatrix));
    DrawModel(model, shader, "in_Position", "in_Normal", NULL);
}

void Thing::draw(GLuint shader, Model* ext_model)
{
    glUniformMatrix4fv(glGetUniformLocation(shader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(MTWmatrix));
    DrawModel(ext_model, shader, "in_Position", "in_Normal", NULL);
}

void Thing::draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix)
{
    glm::mat4 textureMatrixTot = textureMatrix * MTWmatrix; 
    glUniformMatrix4fv(glGetUniformLocation(shader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
    glUniformMatrix4fv(glGetUniformLocation(shader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(MTWmatrix));
    DrawModel(model, shader, "in_Position", "in_Normal", NULL);
}

void Thing::draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix, Model* ext_model)
{
    glm::mat4 textureMatrixTot = textureMatrix * MTWmatrix; 
    glUniformMatrix4fv(glGetUniformLocation(shader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
    glUniformMatrix4fv(glGetUniformLocation(shader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(MTWmatrix));
    DrawModel(ext_model, shader, "in_Position", "in_Normal", NULL);
}

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

void Thing::draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix, GLuint *tex)
{
	glm::mat4 textureMatrixTot = textureMatrix * MTWmatrix;
	glUniformMatrix4fv(glGetUniformLocation(shader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
	glUniformMatrix4fv(glGetUniformLocation(shader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(MTWmatrix));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // verkar inte funka
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // verkar inte funka
	glUniform1i(glGetUniformLocation(shader, "texUnit2"), 1);
	DrawModel(model, shader, "in_Position", "in_Normal", "inTexCoord");
	glActiveTexture(GL_TEXTURE0);
}

void Thing::draw_with_depthinfo(GLuint shader, glm::mat4 &textureMatrix, Model* ext_model)
{
    glm::mat4 textureMatrixTot = textureMatrix * MTWmatrix; 
    glUniformMatrix4fv(glGetUniformLocation(shader, "textureMatrix"), 1, GL_FALSE, glm::value_ptr(textureMatrixTot));
    glUniformMatrix4fv(glGetUniformLocation(shader, "MTWMatrix"), 1, GL_FALSE, glm::value_ptr(MTWmatrix));
    DrawModel(ext_model, shader, "in_Position", "in_Normal", NULL);
}

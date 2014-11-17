#ifndef loadobj_h
#define loadobj_h

#ifdef _WIN32
  #include "GL/glew.h"
#else
  #include <GL/gl.h>
#endif

typedef struct
{
  GLfloat* vertexArray;
  GLfloat* normalArray;
  GLfloat* texCoordArray;
  GLfloat* colorArray; // Rarely used
  GLuint* indexArray;
  int numVertices;
  int numIndices;
  
  // Space for saving VBO and VAO IDs
  GLuint vao; // VAO
  GLuint vb, ib, nb, tb; // VBOs
} Model;

Model* LoadModel(char* name);

// NEW:

void DrawModel(Model *m, GLuint program, char const * vertexVariableName, char const * normalVariableName, char const * texCoordVariableName);
void DrawWireframeModel(Model *m, GLuint program, char* vertexVariableName, char* normalVariableName, char* texCoordVariableName);

Model* LoadModelPlus(char* name/*,
			GLuint program,
			char* vertexVariableName,
			char* normalVariableName,
			char* texCoordVariableName*/);
void EnableModelForShader(Model *m, GLuint program, // NOT TESTED
			char* vertexVariableName,
			char* normalVariableName,
			char* texCoordVariableName);
Model* LoadDataToModel(
			GLfloat *vertices,
			GLfloat *normals,
			GLfloat *texCoords,
			GLfloat *colors,
			GLuint *indices,
			int numVert,
			int numInd);

/*
Model* LoadDataToModel(
			GLfloat *vertices,
			GLfloat *normals,
			GLfloat *texCoords,
			GLfloat *colors,
			GLuint *indices,
			int numVert,
			int numInd,
			
			GLuint program,
			char* vertexVariableName,
			char* normalVariableName,
			char* texCoordVariableName);
void BuildModelVAO(Model *m,
			GLuint program,
			char* vertexVariableName,
			char* normalVariableName,
			char* texCoordVariableName);
*/

void CenterModel(Model *m);

#endif

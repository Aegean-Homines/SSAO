#ifndef FSQ_H_
#define FSQ_H_

#include "glsdk\glm\glm\glm.hpp"
#include <glload/gl_3_3.h>
#include <glload/gl_load.hpp>

#include "transform.h"

class FSQ {
public:

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec3 textureCoords;
	};

	glm::vec3 diffuse;
	glm::vec3 specular;

	Vertex* vertices;
	GLuint* indices;
	GLuint numberOfVertices;
	GLuint numberOfIndices;

	GLuint vertexBufferSize;
	GLuint indexBufferSize;
	GLuint indexCount;

	GLuint vaoID;

	MAT4 transform;
	MAT4 scale;

	FSQ(Vertex * verts, GLuint numVert, GLuint* indice, GLuint numIndic);
	FSQ();

	void Init();
	void Draw();
	void DebugDraw();
	void LoadToGFX();
	
};

#endif
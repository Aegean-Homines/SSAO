#include "FSQ.h"


using glm::vec3;

FSQ::FSQ(Vertex * verts, GLuint numVert, GLuint* indice, GLuint numIndic):
	vertices(verts), numberOfVertices(numVert), indices(indice), numberOfIndices(numIndic)
{

}

FSQ::FSQ() {
	transform = Translate(0.5f, 0.5f, 0.5f);
	scale = Scale(0.2f, 0.2f, 0.2f);
}

void FSQ::Init()
{
	Vertex verts[] =
	{
		vec3(+1.0f, +1.0f, +0.0f),	// 0		TR
		vec3(+1.0f, +0.0f, +0.0f),	// color
		vec3(+0.0f, +0.0f, +1.0f),	// normal
		vec3(+1.0f, +1.0f, +0.0f),	// texCoord

		vec3(-1.0f, +1.0f, +0.0f),	// 1		TL
		vec3(+1.0f, +0.0f, +0.0f),	// color
		vec3(+0.0f, +0.0f, +1.0f),	// normal
		vec3(+0.0f, +1.0f, +0.0f),	// texCoord

		vec3(-1.0f, -1.0f, +0.0f),	// 2		BL
		vec3(+1.0f, +0.0f, +0.0f),	// color
		vec3(+0.0f, +0.0f, +1.0f),	// normal
		vec3(+0.0f, +0.0f, +0.0f),	// texCoord

		vec3(+1.0f, -1.0f, +0.0f),	// 3		BR
		vec3(+1.0f, +0.0f, +0.0f),	// color
		vec3(+0.0f, +0.0f, +1.0f),	// normal
		vec3(+1.0f, +0.0f, +0.0f),	// texCoord
	};

	numberOfVertices = sizeof(verts) / sizeof(*verts);
	vertices = new Vertex[numberOfVertices];
	memcpy(vertices, verts, sizeof(verts));

	GLuint tempindices[] = {
		0, 1, 2,
		2, 3, 0
	};
	numberOfIndices = sizeof(tempindices) / sizeof(*tempindices);
	indices = new GLuint[numberOfIndices];
	memcpy(indices, tempindices, sizeof(tempindices));

	vertexBufferSize = numberOfVertices * sizeof(Vertex);
	indexBufferSize = numberOfIndices * sizeof(Vertex);
}

void FSQ::Draw()
{
	glBindVertexArray(vaoID);
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void FSQ::DebugDraw()
{
	glBindVertexArray(vaoID);
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void FSQ::LoadToGFX()
{
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, textureCoords));

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

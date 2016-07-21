///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_
#define _TEXTURE_

#include <glload/gl_3_3.h>
#include "glsdk\glm\glm\glm.hpp"
#include <vector>

class Texture
{
 public:
    GLuint textureId;
    
    Texture() :textureId(0) {};
	void GenerateTexture(int width, int height, unsigned int internalFormat, unsigned int format);
	void GenerateTextureForSSAONoise(glm::vec3* data);
    void Read(const std::string &filename);
    void Bind(const int unit);
    void Unbind();
};

#endif

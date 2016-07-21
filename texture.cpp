///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////

#include <fstream>
#include "texture.h"

#include <glload/gl_3_3.h>
#include <glload/gl_load.hpp>
#include <glimg/glimg.h>

void Texture::GenerateTexture(int width, int height, unsigned int internalFormat, unsigned int format)
{
	glGenTextures(1, &this->textureId);
	glBindTexture(GL_TEXTURE_2D, this->textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, NULL);

}

void Texture::GenerateTextureForSSAONoise(glm::vec3* data)
{
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, data); // Again, this is a 4x4 texture for just noise data
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // This is how we're tiling it over the screen = by using gl_repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::Read(const std::string &filename)
{
    try {
        glimg::ImageSet* img;

        // This pair of lines reads the image into a byte array and
        // loads that array to the graphics card with a call similar to:
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width,height, 0, GL_RGB, GL_UNSIGNED_BYTE, data)
        img = glimg::loaders::stb::LoadFromFile(filename);
        textureId = glimg::CreateTexture(img, 0);

        // Create a MIPMAP and choose the best(linear) close-in and far-out filters;
        glBindTexture(GL_TEXTURE_2D, textureId);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0); }

    catch (glimg::loaders::stb::UnableToLoadException e) {
        // Exit on any kind of read failure
        printf("%s\n", e.what());
        exit(-1); }
}

void Texture::Bind(const int unit)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::Unbind()
{  
    glBindTexture(GL_TEXTURE_2D, 0);
}

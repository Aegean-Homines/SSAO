///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a Frame Buffer Object (i'e' Render
// Target) and its associated texture.  When the FBO is "Bound", the
// output of the graphics pipeline is captured into the texture.  When
// it is "Unbound", the texture is available for use as any normal
// texture.
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include <list>

class FBO {
public:
    unsigned int fbo; // The framebuffer to hold texture and depth -E

    unsigned int texture; // Texture that holds RGB output of the shader -E
    int width, height;  // Size of the texture.

    void CreateFBO(const int w, const int h, unsigned int internalFormat = GL_RGBA32F_ARB, unsigned int format = GL_RGBA);

	void CreateFBOForDeferredShading(const int w, const int h);

	void CreateFBOForSSAO(const int w, const int h);

	void CreateFBOForSSAOColorBuffer(const int w, const int h);
    
	unsigned int gPosition, gNormal, gDifSpec, gSpecular; //used for deferred

	unsigned int gPositionDepth; //used for ssao

	unsigned int depth;

    // Using this will redirect output of shader into texture
    void Bind();
    void Unbind();
	void CreateColorAttachment(GLuint texture, const int width, const int height, GLuint textureImageInternalFormat, GLuint textureImageFormat, GLuint attachmentPlace);
	void PushColorAttachment(GLuint attachmentNo);
private:
	std::list<GLuint> attachments;
};

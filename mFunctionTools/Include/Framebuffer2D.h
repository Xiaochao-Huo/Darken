
#ifndef FRAMEBUFFER2D_H
#define FRAMEBUFFER2D_H
#include <glew.h>
#include <glfw3.h>

class Framebuffer2D
{
public:
	Framebuffer2D(int width, int height);
	~Framebuffer2D();

/**
 * Assigns a buffer specified in $buffer with a texture
 *
 * @buffer unsigned char, The buffer to be assigned, valid targets are specified in Table 1
 *
 *
 *
 * @return bool Returns true if the buffer was assigned successfully.
 */
	bool attachBuffer(	unsigned char buffer,
						GLint internalFormat = 		GL_RGBA,
						GLint format = 				GL_RGBA,
						GLint type = 				GL_UNSIGNED_INT,
						GLint textureMinFilter = 	GL_NEAREST,
						GLint textureMagFilter = 	GL_NEAREST,
						GLint textureWrapS = 		GL_CLAMP_TO_EDGE,
						GLint textureWrapT = GL_CLAMP_TO_EDGE,
						GLboolean mipMap =			GL_FALSE);

	void destroyBuffers(unsigned char bufferBit);

	void resizeBuffers(unsigned char bufferBit, int width, int height);

	inline int getWidth() { return width; }
	inline int getHeight() { return height; }

	void bind();
	void unbind();

	inline unsigned int getBufferHandle(unsigned char buffer) { return *getTextureHandle(buffer); }

private:
	bool bufferIsColor(unsigned char buffer);
	bool bufferIsDepth(unsigned char buffer);
	bool bufferIsDepthStencil(unsigned char buffer);
	bool bufferIsValid(unsigned char buffer);
	unsigned int *getTextureHandle(unsigned char buffer);
	GLenum getGLAttachment(unsigned char buffer);
	void updateColorBuffers();

	int width, height;

	GLenum *ColorBuffers;
	unsigned char numColorBuffers;

	unsigned int bufferHandle[6];

	unsigned int fboHandle;
};

/**
 * Table 1: Buffers that can be specified in assignBuffer
 */
#define FBO_COLOR0	0x00
#define FBO_COLOR1	0x01
#define FBO_COLOR2	0x02
#define FBO_COLOR3	0x03
#define FBO_DEPTH	0x04
#define FBO_DEPTH_STENCIL	0x05
#define FBO_COUNT	0x06

/**
 * Table 2: Buffer bits that can be specified in resizeBuffers
 */
#define FBO_COLOR0_BIT 	0x00
#define FBO_COLOR1_BIT 	0x01
#define FBO_COLOR2_BIT 	0x02
#define FBO_COLOR3_BIT 	0x04
#define FBO_DEPTH_BIT	0x08


#endif
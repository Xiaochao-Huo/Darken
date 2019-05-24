
#include "Framebuffer2D.h"

#define MAX_COLOR_BUFFERS 4

#define clamp(x,min,max) (x < min) ? min : ( (x > max) ? max : x )

Framebuffer2D::Framebuffer2D(int width, int height)
{
    this->width = width;
    this->height = height;

    numColorBuffers = 0;
    ColorBuffers = nullptr;
    
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer2D::~Framebuffer2D()
{
    if(glIsFramebuffer(fboHandle))
        glDeleteFramebuffers(1, &fboHandle);

    destroyBuffers( FBO_COLOR0_BIT | FBO_COLOR1_BIT | FBO_COLOR2_BIT | FBO_COLOR3_BIT |
                    FBO_DEPTH_BIT);
}

bool Framebuffer2D::attachBuffer(   unsigned char buffer,
                                    GLint internalFormat, GLint format, GLint type,
                                    GLint textureMinFilter, GLint textureMagFilter,
                                    GLint textureWrapS, GLint textureWrapT, GLboolean mipMap)
{
    if(bufferIsValid(buffer))
    {
        unsigned int *textureHandle = getTextureHandle(buffer);
        GLenum attachment = getGLAttachment(buffer);
	
		if (!glIsTexture(*textureHandle))
		{
			glDeleteTextures(1, textureHandle);
		}
       
        
        glGenTextures(1, textureHandle);

        glBindTexture(GL_TEXTURE_2D, *textureHandle);

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureMinFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMagFilter );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapS );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapT );
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

        if(mipMap)
            glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        if(bufferIsColor(buffer))
             updateColorBuffers();

        glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, *textureHandle, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }
    

    return false;
}

void Framebuffer2D::destroyBuffers(unsigned char bufferBit)
{
    if( bufferBit & FBO_COLOR0_BIT &&
        glIsTexture(bufferHandle[FBO_COLOR0]))
            glDeleteTextures(1, &bufferHandle[FBO_COLOR0]);

    if( bufferBit & FBO_COLOR1_BIT &&
        glIsTexture(bufferHandle[FBO_COLOR1]))
            glDeleteTextures(1, &bufferHandle[FBO_COLOR1]);

    if( bufferBit & FBO_COLOR2_BIT &&
        glIsTexture(bufferHandle[FBO_COLOR2]))
            glDeleteTextures(1, &bufferHandle[FBO_COLOR2]);

    if( bufferBit & FBO_COLOR3_BIT &&
        glIsTexture(bufferHandle[FBO_COLOR3]))
            glDeleteTextures(1, &bufferHandle[FBO_COLOR3]);

    if(bufferBit & FBO_DEPTH_BIT &&
        glIsTexture(bufferHandle[FBO_DEPTH]))
            glDeleteTextures(1, &bufferHandle[FBO_DEPTH]);
}

void Framebuffer2D::bind()
{
	//GLint PrevViewport[4];
	//glGetIntegerv(GL_VIEWPORT, PrevViewport);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glViewport(0,0,width, height);

    if(numColorBuffers > 0)
        glDrawBuffers(numColorBuffers, ColorBuffers);

	//glViewport(PrevViewport[0], PrevViewport[1], PrevViewport[2], PrevViewport[3]);
}

void Framebuffer2D::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer2D::updateColorBuffers()
{
    numColorBuffers = 0;
    for(unsigned char i=0; i<4; i++)
    {
        if(glIsTexture(bufferHandle[i]))
            numColorBuffers++;
    }

    if(ColorBuffers)
        delete[] ColorBuffers;

	ColorBuffers = new GLenum[numColorBuffers];

    for(unsigned char i=0; i<numColorBuffers; i++)
    {
		ColorBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
}

bool Framebuffer2D::bufferIsColor(unsigned char buffer)
{
    return  buffer == FBO_COLOR0 ||
            buffer == FBO_COLOR1 ||
            buffer == FBO_COLOR2 ||
            buffer == FBO_COLOR3;
}

bool Framebuffer2D::bufferIsDepth(unsigned char buffer)
{
    return buffer == FBO_DEPTH;
}

bool Framebuffer2D::bufferIsDepthStencil(unsigned char buffer)
{
	return buffer == FBO_DEPTH_STENCIL;
}

bool Framebuffer2D::bufferIsValid(unsigned char buffer)
{
    return  buffer == FBO_COLOR0      ||
            buffer == FBO_COLOR1      ||
            buffer == FBO_COLOR2      ||
            buffer == FBO_COLOR3      ||
            buffer == FBO_DEPTH       ||
		    buffer == FBO_DEPTH_STENCIL;
}

unsigned int *Framebuffer2D::getTextureHandle(unsigned char buffer)
{
    buffer = clamp(buffer,0,FBO_COUNT-1);
    return &bufferHandle[buffer];
}

GLenum Framebuffer2D::getGLAttachment(unsigned char buffer)
{
    static unsigned short attachmentMap[FBO_COUNT] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_DEPTH_ATTACHMENT,
		GL_DEPTH_STENCIL_ATTACHMENT
    };

    // ASSERT THIS!
    buffer = clamp(buffer,0,FBO_COUNT-1);

    return attachmentMap[buffer];
}

#undef clamp
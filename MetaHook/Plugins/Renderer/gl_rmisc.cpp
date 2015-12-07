#include "gl_local.h"

void R_PushFrameBuffer(void)
{
	if(gl_framebuffer_object)
	{
		qglGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint *)&readframebuffer);
		qglGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint *)&drawframebuffer);
	}
}

void R_PopFrameBuffer(void)
{
	if(gl_framebuffer_object)
	{
		qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, readframebuffer);
		qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, drawframebuffer);
	}
}

void R_GLBindFrameBuffer(GLenum target, GLuint framebuffer)
{
	if(gl_framebuffer_object)
	{
		qglBindFramebufferEXT(target, framebuffer);
	}
}

GLuint R_GLGenTexture(int w, int h)
{
	GLuint texid = GL_GenTexture();
	GL_Bind(texid);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	return texid;
}
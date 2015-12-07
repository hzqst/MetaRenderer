#include "gl_local.h"

GLenum TEXTURE0_SGIS;
GLenum TEXTURE1_SGIS;
GLenum TEXTURE2_SGIS;
GLenum TEXTURE3_SGIS;

int *currenttexid;//for 3xxx~4xxx

GLuint GL_GenTexture(void)
{
	GLuint tex;
	if(g_dwEngineBuildnum < 5953)
	{
		tex = (*currenttexid);
		(*currenttexid) ++;
	}
	else
	{
		qglGenTextures(1, &tex);
	}
	return tex;
}

void GL_DeleteTexture(GLuint tex)
{
	qglDeleteTextures(1, &tex);
}

void GL_Bind(int texnum)
{
	gRefFuncs.GL_Bind(texnum);
}

void GL_SelectTexture(GLenum target)
{
	gRefFuncs.GL_SelectTexture(target);
}

void GL_DisableMultitexture(void)
{
	gRefFuncs.GL_DisableMultitexture();
}

void GL_EnableMultitexture(void)
{
	gRefFuncs.GL_EnableMultitexture();
}
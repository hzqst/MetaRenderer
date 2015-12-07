#pragma once

#include <gl/gl.h>

typedef struct
{
	void (*R_RenderView)(void);
	void (*GL_Bind)(int texnum);
	void (*GL_SelectTexture)(GLenum target);
	void (*GL_DisableMultitexture)(void);
	void (*GL_EnableMultitexture)(void);
	void (*GL_BeginRendering)(int *x, int *y, int *width, int *height);
	void (*GL_EndRendering)(void);
	int (*GL_LoadTexture2)(char *identifier, int textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal, int filter);
	void (*FreeFBObjects)(void);//only above 5953
	void (*VID_UpdateWindowVars)(RECT *prc, int x, int y);
}ref_funcs_t;

extern ref_funcs_t gRefFuncs;

#define META_RENDERER_VERSION "Meta Renderer 1.5"
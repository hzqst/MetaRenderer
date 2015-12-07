#pragma once

#include <metahook.h>
#include <math.h>
#include <assert.h>
#include <mathlib.h>
#include <archtypes.h>
#include <const.h>
#include <custom.h>
#include <ref_params.h>
#include <cvardef.h>
#include <studio.h>
#include <r_studioint.h>
#include <pm_movevars.h>

#include "plugins.h"

#include "exportfuncs.h"

#include "qgl.h"

#include "gl_shader.h"
#include "enginedef.h"
#include "gl_hud.h"
#include "gl_draw.h"
#include "ref_int_internal.h"

extern RECT *window_rect;
extern float *videowindowaspect;
extern float *windowvideoaspect;
extern float videowindowaspect_old;
extern float windowvideoaspect_old;

extern GLuint screenframebuffer;
extern GLuint drawframebuffer;
extern GLuint readframebuffer;

//gl extension
extern qboolean gl_framebuffer_object;
extern qboolean gl_shader_support;
extern qboolean gl_program_support;
extern qboolean gl_msaa_support;
extern qboolean gl_msaa_blit_support;
extern qboolean gl_csaa_support;
extern qboolean gl_float_buffer_support;

extern int gl_mtexable;
extern int gl_max_texture_size;

extern qboolean bDoMSAAFBO;
extern qboolean bDoScaledFBO;
extern qboolean bDoDirectBlit;
extern qboolean bDoHDR;
extern qboolean bNoStretchAspect;

extern FBO_Container_t s_MSAAFBO;
extern FBO_Container_t s_BackBufferFBO;
extern FBO_Container_t s_DownSampleFBO[DOWNSAMPLE_BUFFERS];
extern FBO_Container_t s_LuminFBO[LUMIN_BUFFERS];
extern FBO_Container_t s_Lumin1x1FBO[LUMIN1x1_BUFFERS];
extern FBO_Container_t s_BrightPassFBO;
extern FBO_Container_t s_BlurPassFBO[BLUR_BUFFERS][2];
extern FBO_Container_t s_BrightAccumFBO;
extern FBO_Container_t s_ToneMapFBO;

extern cvar_t *v_gamma;

void R_FillAddress(void);
void R_InstallHook(void);
void R_RenderView(void);
void R_Init(void);
void R_Shutdown(void);

void GL_Init(void);
void GL_BeginRendering(int *x, int *y, int *width, int *height);
void GL_EndRendering(void);
void GL_InitExtensions(void);
GLuint GL_GenTexture(void);
void GL_DeleteTexture(GLuint tex);
void GL_Bind(int texnum);
void GL_SelectTexture(GLenum target);
void GL_DisableMultitexture(void);
void GL_EnableMultitexture(void);

//framebuffer
void R_PushFrameBuffer(void);
void R_PopFrameBuffer(void);
void R_GLBindFrameBuffer(GLenum target, GLuint framebuffer);

void R_InitRefHUD(void);

extern double g_flFrameTime;
extern int last_luminance;

#define glwidth g_iVideoWidth
#define glheight g_iVideoHeight
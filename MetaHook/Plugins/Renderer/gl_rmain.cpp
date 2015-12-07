#include "gl_local.h"

ref_funcs_t gRefFuncs;

RECT *window_rect;

float *videowindowaspect;
float *windowvideoaspect;
float videowindowaspect_old;
float windowvideoaspect_old;

GLuint screenframebuffer;
GLuint drawframebuffer;
GLuint readframebuffer;

qboolean gl_framebuffer_object = false;
qboolean gl_shader_support = false;
qboolean gl_program_support = false;
qboolean gl_msaa_support = false;
qboolean gl_msaa_blit_support = false;
qboolean gl_csaa_support = false;
qboolean gl_float_buffer_support = false;

int gl_msaa_samples = 0;
int gl_csaa_samples = 0;

FBO_Container_t s_MSAAFBO;
FBO_Container_t s_BackBufferFBO;
FBO_Container_t s_DownSampleFBO[DOWNSAMPLE_BUFFERS];
FBO_Container_t s_LuminFBO[LUMIN_BUFFERS];
FBO_Container_t s_Lumin1x1FBO[LUMIN1x1_BUFFERS];
FBO_Container_t s_BrightPassFBO;
FBO_Container_t s_BlurPassFBO[BLUR_BUFFERS][2];
FBO_Container_t s_BrightAccumFBO;
FBO_Container_t s_ToneMapFBO;

qboolean bDoMSAAFBO = true;
qboolean bDoScaledFBO = true;
qboolean bDoDirectBlit = true;
qboolean bDoHDR = true;
qboolean bNoStretchAspect = false;

cvar_t *v_gamma = NULL;

int gl_mtexable;

void CheckMultiTextureExtensions(void)
{
	if (gl_mtexable)
	{
		TEXTURE0_SGIS = GL_TEXTURE0;
		TEXTURE1_SGIS = GL_TEXTURE1;
		TEXTURE2_SGIS = GL_TEXTURE2;
		TEXTURE3_SGIS = GL_TEXTURE3;
	}
	else
	{
		Sys_ErrorEx("don't support multitexture extension!");
	}
}

void GL_ClearFBO(FBO_Container_t *s)
{
	s->s_hBackBufferFBO = 0;
	s->s_hBackBufferCB = 0;
	s->s_hBackBufferDB = 0;
	s->s_hBackBufferTex = 0;
	s->iWidth = s->iHeight = 0;
}

void GL_FreeFBO(FBO_Container_t *s)
{
	if (s->s_hBackBufferFBO)
		qglDeleteFramebuffersEXT(1, &s->s_hBackBufferFBO);

	if (s->s_hBackBufferCB)
		qglDeleteRenderbuffersEXT(1, &s->s_hBackBufferCB);

	if (s->s_hBackBufferDB)
		qglDeleteRenderbuffersEXT(1, &s->s_hBackBufferDB);

	if (s->s_hBackBufferTex)
		qglDeleteTextures(1, &s->s_hBackBufferTex);

	GL_ClearFBO(s);
}

void R_GLGenFrameBuffer(FBO_Container_t *s)
{
	qglGenFramebuffersEXT(1, &s->s_hBackBufferFBO);
	qglBindFramebufferEXT(GL_FRAMEBUFFER, s->s_hBackBufferFBO);
}

void R_GLGenRenderBuffer(FBO_Container_t *s, qboolean depth)
{
	if(!depth)
	{
		qglGenRenderbuffersEXT(1, &s->s_hBackBufferCB);
		qglBindRenderbufferEXT(GL_RENDERBUFFER, s->s_hBackBufferCB);
	}
	else
	{
		qglGenRenderbuffersEXT(1, &s->s_hBackBufferDB);
		qglBindRenderbufferEXT(GL_RENDERBUFFER, s->s_hBackBufferDB);
	}
}

void R_GLRenderBufferStorage(FBO_Container_t *s, qboolean depth, GLuint iInternalFormat, qboolean multisample)
{
	if(multisample)
	{
		if(gl_csaa_support)
			qglRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, gl_csaa_samples, gl_msaa_samples, iInternalFormat, s->iWidth, s->iHeight);
		else
			qglRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, gl_msaa_samples, iInternalFormat, s->iWidth, s->iHeight);
	}
	else
	{
		qglRenderbufferStorageEXT(GL_RENDERBUFFER, iInternalFormat, s->iWidth, s->iHeight);
	}
	qglFramebufferRenderbufferEXT(GL_FRAMEBUFFER, (depth) ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (depth) ? s->s_hBackBufferDB : s->s_hBackBufferCB);
}

void R_GLFrameBufferColorTexture(FBO_Container_t *s, GLuint iInternalFormat)
{
	s->s_hBackBufferTex = GL_GenTexture();
	GL_Bind(s->s_hBackBufferTex);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexImage2D(GL_TEXTURE_2D, 0, iInternalFormat, s->iWidth, s->iHeight, 0, GL_RGBA, (iInternalFormat != GL_RGBA && iInternalFormat != GL_RGBA8) ? GL_FLOAT : GL_UNSIGNED_BYTE, 0);

	qglFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s->s_hBackBufferTex, 0);
}

void GL_GenerateFBO(void)
{
	if (!gl_framebuffer_object)
		return;

	bNoStretchAspect = (gEngfuncs.CheckParm("-stretchaspect", NULL) == 0);

	if (gEngfuncs.CheckParm("-nomsaa", NULL))
		bDoMSAAFBO = false;

	//if (!gl_msaa_blit_support)
	//	bDoMSAAFBO = false;

	if (gEngfuncs.CheckParm("-nofbo", NULL))
		bDoScaledFBO = false;

	if (gEngfuncs.CheckParm("-directblit", NULL))
		bDoDirectBlit = true;

	if (gEngfuncs.CheckParm("-nodirectblit", NULL))
		bDoDirectBlit = false;

	if (gEngfuncs.CheckParm("-nohdr", NULL))
		bDoHDR = false;

	if(!gl_float_buffer_support)
		bDoHDR = false;

	if (!qglGenFramebuffersEXT || !qglBindFramebufferEXT || !qglBlitFramebufferEXT)
		bDoScaledFBO = false;

	GL_ClearFBO(&s_MSAAFBO);
	GL_ClearFBO(&s_BackBufferFBO);

	for(int i = 0; i < DOWNSAMPLE_BUFFERS; ++i)
		GL_ClearFBO(&s_DownSampleFBO[i]);
	for(int i = 0; i < LUMIN_BUFFERS; ++i)
		GL_ClearFBO(&s_LuminFBO[i]);
	for(int i = 0; i < LUMIN1x1_BUFFERS; ++i)
		GL_ClearFBO(&s_Lumin1x1FBO[i]);
	GL_ClearFBO(&s_BrightPassFBO);
	for(int i = 0; i < BLUR_BUFFERS; ++i)
	{
		GL_ClearFBO(&s_BlurPassFBO[i][0]);
		GL_ClearFBO(&s_BlurPassFBO[i][1]);
	}
	GL_ClearFBO(&s_BrightAccumFBO);
	GL_ClearFBO(&s_ToneMapFBO);

	if(!bDoScaledFBO)
	{
		gEngfuncs.Con_Printf("FBO backbuffer rendering disabled.\n");
		bDoHDR = false;
	}

	qglEnable(GL_TEXTURE_2D);

	GLuint iColorInternalFormat = GL_RGBA8;

	if(bDoHDR)
	{
		iColorInternalFormat = GL_RGBA16F;

		const char *s_HDRColor;
		if(g_pInterface->CommandLine->CheckParm("-hdrcolor", &s_HDRColor))
		{
			if(s_HDRColor && s_HDRColor[0] >= '0' && s_HDRColor[0] <= '9')
			{
				int i_HDRColor = atoi(s_HDRColor);
				if(i_HDRColor == 8)
					iColorInternalFormat = GL_RGBA8;
				else if(i_HDRColor == 16)
					iColorInternalFormat = GL_RGBA16F;
				else if(i_HDRColor == 32)
					iColorInternalFormat = GL_RGBA32F;
			}
		}
	}

	if (bDoScaledFBO && gl_msaa_support && bDoMSAAFBO)
	{
		const char *s_Samples;
		gl_msaa_samples = 4;
		if(gl_csaa_support)
			gl_csaa_samples = 8;
		if(g_pInterface->CommandLine->CheckParm("-msaa", &s_Samples))
		{
			if(s_Samples && s_Samples[0] >= '0' && s_Samples[0] <= '9')
			{
				int i_Samples = atoi(s_Samples);
				if(i_Samples == 4)
					gl_msaa_samples = 4;
				else if(i_Samples == 8)
					gl_msaa_samples = 8;
				else if(i_Samples == 16)
					gl_msaa_samples = 16;
			}
		}

		if(g_pInterface->CommandLine->CheckParm("-csaa", &s_Samples))
		{
			if(s_Samples && s_Samples[0] >= '0' && s_Samples[0] <= '9')
			{
				int i_Samples = atoi(s_Samples);
				if(i_Samples == 4)
					gl_csaa_samples = 4;
				else if(i_Samples == 8)
					gl_csaa_samples = 8;
				else if(i_Samples == 16)
					gl_csaa_samples = 16;
			}
		}

		s_MSAAFBO.iWidth = g_iVideoWidth;
		s_MSAAFBO.iHeight = g_iVideoHeight;

		//fbo
		R_GLGenFrameBuffer(&s_MSAAFBO);
		//color
		R_GLGenRenderBuffer(&s_MSAAFBO, false);
		R_GLRenderBufferStorage(&s_MSAAFBO, false, iColorInternalFormat, true);
		//depth
		R_GLGenRenderBuffer(&s_MSAAFBO, true);
		R_GLRenderBufferStorage(&s_MSAAFBO, true, GL_DEPTH_COMPONENT24, true);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_MSAAFBO);
			gl_msaa_support = false;
			gEngfuncs.Con_Printf("Error initializing MSAA frame buffer\n");
		}
	}
	else
	{
		gEngfuncs.Con_Printf("MSAA backbuffer rendering disabled.\n");
	}

	if (bDoScaledFBO)
	{
		if (s_MSAAFBO.s_hBackBufferFBO)
			qglEnable(GL_MULTISAMPLE);

		s_BackBufferFBO.iWidth = g_iVideoWidth;
		s_BackBufferFBO.iHeight = g_iVideoHeight;

		//fbo
		R_GLGenFrameBuffer(&s_BackBufferFBO);
		//color
		R_GLFrameBufferColorTexture(&s_BackBufferFBO, iColorInternalFormat);
		//depth
		R_GLGenRenderBuffer(&s_BackBufferFBO, true);
		R_GLRenderBufferStorage(&s_BackBufferFBO, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, false);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_MSAAFBO);
			GL_FreeFBO(&s_BackBufferFBO);		
			gEngfuncs.Con_Printf("FBO backbuffer rendering disabled due to create error.\n");
		}
	}

	int downW, downH;

	//DownSample FBO 1->1/4->1/16
	if(bDoHDR)
	{
		downW = g_iVideoWidth;
		downH = g_iVideoHeight;
		for(int i = 0; i < DOWNSAMPLE_BUFFERS && bDoHDR; ++i)
		{
			downW >>= 1;
			downH >>= 1;
			s_DownSampleFBO[i].iWidth = downW;
			s_DownSampleFBO[i].iHeight = downH;
			//fbo
			R_GLGenFrameBuffer(&s_DownSampleFBO[i]);
			//color
			R_GLFrameBufferColorTexture(&s_DownSampleFBO[i], iColorInternalFormat);

			if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				GL_FreeFBO(&s_DownSampleFBO[i]);
				bDoHDR = false;
				gEngfuncs.Con_Printf("DownSample FBO #%d rendering disabled due to create error.\n", i);
			}
		}
	}

	//Luminance FBO
	if(bDoHDR)
	{
		downW = 64;
		downH = 64;
		//64x64 16x16 4x4
		for(int i = 0; i < LUMIN_BUFFERS && bDoHDR; ++i)
		{
			s_LuminFBO[i].iWidth = downW;
			s_LuminFBO[i].iHeight = downH;
			//fbo
			R_GLGenFrameBuffer(&s_LuminFBO[i]);
			//color
			R_GLFrameBufferColorTexture(&s_LuminFBO[i], GL_R32F);

			if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				GL_FreeFBO(&s_LuminFBO[i]);
				bDoHDR = false;
				gEngfuncs.Con_Printf("Luminance FBO #%d rendering disabled due to create error.\n", i);
			}
			downW >>= 2;
			downH >>= 2;
		}
	}

	//Luminance 1x1 FBO
	if(bDoHDR)
	{
		for(int i = 0; i < LUMIN1x1_BUFFERS; ++i)
		{
			s_Lumin1x1FBO[i].iWidth = 1;
			s_Lumin1x1FBO[i].iHeight = 1;
			//fbo
			R_GLGenFrameBuffer(&s_Lumin1x1FBO[i]);
			//color
			R_GLFrameBufferColorTexture(&s_Lumin1x1FBO[i], GL_R32F);

			if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				GL_FreeFBO(&s_Lumin1x1FBO[i]);
				bDoHDR = false;
				gEngfuncs.Con_Printf("Luminance FBO #%d rendering disabled due to create error.\n", i);
			}
		}
	}

	//Bright Pass FBO
	if(bDoHDR)
	{
		s_BrightPassFBO.iWidth = (g_iVideoWidth >> DOWNSAMPLE_BUFFERS);
		s_BrightPassFBO.iHeight = (g_iVideoHeight >> DOWNSAMPLE_BUFFERS);

		//fbo
		R_GLGenFrameBuffer(&s_BrightPassFBO);
		//color
		R_GLFrameBufferColorTexture(&s_BrightPassFBO, iColorInternalFormat);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_BrightPassFBO);
			bDoHDR = false;
			gEngfuncs.Con_Printf("BrightPass FBO rendering disabled due to create error.\n");
		}
	}

	//Blur FBO
	if(bDoHDR)
	{
		downW = g_iVideoWidth >> DOWNSAMPLE_BUFFERS;
		downH = g_iVideoHeight >> DOWNSAMPLE_BUFFERS;
		for(int i = 0; i < BLUR_BUFFERS; ++i)
		{
			for(int j = 0; j < 2; ++j)
			{
				s_BlurPassFBO[i][j].iWidth = downW;
				s_BlurPassFBO[i][j].iHeight = downH;

				//fbo
				R_GLGenFrameBuffer(&s_BlurPassFBO[i][j]);
				//color
				R_GLFrameBufferColorTexture(&s_BlurPassFBO[i][j], iColorInternalFormat);

				if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				{
					GL_FreeFBO(&s_BlurPassFBO[i][j]);
					bDoHDR = false;
					gEngfuncs.Con_Printf("Blur FBO #%d rendering disabled due to create error.\n", i);
				}
			}
			downW >>= 1;
			downH >>= 1;
		}
	}

	if(bDoHDR)
	{
		s_BrightAccumFBO.iWidth = g_iVideoWidth >> DOWNSAMPLE_BUFFERS;
		s_BrightAccumFBO.iHeight = g_iVideoHeight >> DOWNSAMPLE_BUFFERS;

		//fbo
		R_GLGenFrameBuffer(&s_BrightAccumFBO);
		//color
		R_GLFrameBufferColorTexture(&s_BrightAccumFBO, iColorInternalFormat);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_BrightAccumFBO);
			bDoHDR = false;
			gEngfuncs.Con_Printf("Bright accumulate FBO #%d rendering disabled due to create error.\n");
		}
	}

	if(bDoHDR)
	{
		s_ToneMapFBO.iWidth = g_iVideoWidth;
		s_ToneMapFBO.iHeight = g_iVideoHeight;

		//fbo
		R_GLGenFrameBuffer(&s_ToneMapFBO);
		//color
		R_GLFrameBufferColorTexture(&s_ToneMapFBO, GL_RGBA8);

		if (qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			GL_FreeFBO(&s_ToneMapFBO);
			bDoHDR = false;
			gEngfuncs.Con_Printf("Tone mapping FBO #%d rendering disabled due to create error.\n");
		}
	}
	qglBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	readframebuffer = drawframebuffer = 0;
}

void GL_Init(void)
{
	QGL_Init();

	CheckMultiTextureExtensions();

	GL_GenerateFBO();
}

void GL_Shutdown(void)
{
	GL_FreeFBO(&s_MSAAFBO);
	GL_FreeFBO(&s_BackBufferFBO);
	for(int i = 0; i < DOWNSAMPLE_BUFFERS; ++i)
		GL_FreeFBO(&s_DownSampleFBO[i]);
	for(int i = 0; i < LUMIN_BUFFERS; ++i)
		GL_FreeFBO(&s_LuminFBO[i]);
	for(int i = 0; i < LUMIN1x1_BUFFERS; ++i)
		GL_FreeFBO(&s_Lumin1x1FBO[i]);
	for(int i = 0; i < BLUR_BUFFERS; ++i)
	{
		GL_FreeFBO(&s_BlurPassFBO[i][0]);
		GL_FreeFBO(&s_BlurPassFBO[i][1]);
	}
	GL_FreeFBO(&s_ToneMapFBO);
}

void GL_BeginRendering(int *x, int *y, int *width, int *height)
{
	gRefFuncs.GL_BeginRendering(x, y, width, height);

	screenframebuffer = 0;

	//no MSAA here
	if (s_BackBufferFBO.s_hBackBufferFBO)
	{
		screenframebuffer = s_BackBufferFBO.s_hBackBufferFBO;
		qglBindFramebufferEXT(GL_FRAMEBUFFER, screenframebuffer);
	}
	
	qglClearColor(0.0, 0.0, 0.0, 1.0);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void R_RenderView(void)
{
	if (s_BackBufferFBO.s_hBackBufferFBO)
	{
		if (s_MSAAFBO.s_hBackBufferFBO)
			screenframebuffer = s_MSAAFBO.s_hBackBufferFBO;
		else
			screenframebuffer = s_BackBufferFBO.s_hBackBufferFBO;

		qglBindFramebufferEXT(GL_FRAMEBUFFER, screenframebuffer);
	}

	gRefFuncs.R_RenderView();

	if (s_BackBufferFBO.s_hBackBufferFBO)
	{
		//Do MSAA here so HUD won't be AA
		if (s_MSAAFBO.s_hBackBufferFBO)
		{
			qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);
			qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, s_MSAAFBO.s_hBackBufferFBO);
			qglBlitFramebufferEXT(0, 0, s_MSAAFBO.iWidth, s_MSAAFBO.iHeight, 0, 0, s_BackBufferFBO.iWidth, s_BackBufferFBO.iHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

		R_BeginHUDQuad();
		if(bDoHDR && r_hdr->value > 0)
		{
			static float cl_time = 0;
			static float cl_oldtime = 0;
			cl_oldtime = cl_time;
			cl_time = gEngfuncs.GetClientTime();
			//normal downsample
			R_DownSample(&s_BackBufferFBO, &s_DownSampleFBO[0], false);//(1->1/4)
			R_DownSample(&s_DownSampleFBO[0], &s_DownSampleFBO[1], false);//(1/4)->(1/16)
			
			//Log Luminance DownSample from .. (HDRColor to 32RF)
			R_LuminPass(&s_DownSampleFBO[1], &s_LuminFBO[0], 1);//(1/16)->64x64
			//Luminance DownSample from..
			R_LuminPass(&s_LuminFBO[0], &s_LuminFBO[1], 0);//64x64->16x16
			R_LuminPass(&s_LuminFBO[1], &s_LuminFBO[2], 0);//16x16->4x4
			//exp Luminance DownSample from..
			R_LuminPass(&s_LuminFBO[2], &s_Lumin1x1FBO[2], 2);//4x4->1x1
			//Luminance Adaptation
			R_LuminAdaptation(&s_Lumin1x1FBO[2], &s_Lumin1x1FBO[!last_luminance], &s_Lumin1x1FBO[last_luminance], cl_time - cl_oldtime);
			last_luminance = !last_luminance;
			//Bright Pass (with 1/16)
			R_BrightPass(&s_DownSampleFBO[1], &s_BrightPassFBO, &s_Lumin1x1FBO[last_luminance]);
			//Gaussian Blur Pass (with bright pass)
			R_BlurPass(&s_BrightPassFBO, &s_BlurPassFBO[0][0], false);
			R_BlurPass(&s_BlurPassFBO[0][0], &s_BlurPassFBO[0][1], true);
			//Blur again and downsample from 1/16 to 1/32
			R_BlurPass(&s_BlurPassFBO[0][1], &s_BlurPassFBO[1][0], false);
			R_BlurPass(&s_BlurPassFBO[1][0], &s_BlurPassFBO[1][1], true);
			//Blur again and downsample from 1/32 to 1/64
			R_BlurPass(&s_BlurPassFBO[1][1], &s_BlurPassFBO[2][0], false);
			R_BlurPass(&s_BlurPassFBO[2][0], &s_BlurPassFBO[2][1], true);
			//Accumulate all blurred textures
			R_BrightAccum(&s_BlurPassFBO[0][1], &s_BlurPassFBO[1][1], &s_BlurPassFBO[2][1], &s_BrightAccumFBO);
			//Tone mapping
			R_ToneMapping(&s_BackBufferFBO, &s_ToneMapFBO, &s_BrightAccumFBO, &s_Lumin1x1FBO[last_luminance]);

			R_BlitToFBO(&s_ToneMapFBO, &s_BackBufferFBO);
		}

		screenframebuffer = s_BackBufferFBO.s_hBackBufferFBO;
	}
	
	qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, screenframebuffer);
	qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, screenframebuffer);
}

void GL_EndRendering(void)
{
	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);
		qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);
		qglClearColor(0, 0, 0, 0);
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int windowW = g_iVideoWidth;
		int windowH = g_iVideoHeight;

		windowW = window_rect->right - window_rect->left;
		windowH = window_rect->bottom - window_rect->top;

		int dstX = 0;
		int dstY = 0;
		int dstX2 = windowW;
		int dstY2 = windowH;

		*videowindowaspect = *windowvideoaspect = 1;

		float videoAspect = (float)g_iVideoWidth / g_iVideoHeight;
		float windowAspect = (float)windowW / windowH;
		if ( bNoStretchAspect )
		{
			if ( windowAspect < videoAspect )
			{
				dstY = (windowH - 1.0 / videoAspect * windowW) / 2.0;
				dstY2 = windowH - dstY;
				*videowindowaspect = videoAspect / windowAspect;
			}
			else
			{
				dstX = (windowW - windowH * videoAspect) / 2.0;
				dstX2 = windowW - dstX;
				*windowvideoaspect = windowAspect / videoAspect;
			}
		}

		if ( bDoDirectBlit )
		{
			qglBlitFramebufferEXT(0, 0, g_iVideoWidth, g_iVideoHeight, dstX, dstY, dstX2, dstY2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
		else
		{
			qglDisable(GL_BLEND);
			qglDisable(GL_LIGHTING);
			qglDisable(GL_DEPTH_TEST);
			qglDisable(GL_ALPHA_TEST);
			qglDisable(GL_CULL_FACE);
			qglMatrixMode(GL_PROJECTION);
			qglPushMatrix();
			qglLoadIdentity();
			qglOrtho(0.0, windowW, windowH, 0.0, -1.0, 1.0);
			qglMatrixMode(GL_MODELVIEW);
			qglPushMatrix();
			qglLoadIdentity();
			qglViewport(0, 0, windowW, windowH);
			qglEnable(GL_TEXTURE_2D);

			qglColor4f(1, 1, 1, 1);			
			qglBindTexture(GL_TEXTURE_2D, s_BackBufferFBO.s_hBackBufferTex);
			qglBegin(GL_QUADS);
			qglTexCoord2f(0, 1);
			qglVertex3f(dstX, dstY, 0);
			qglTexCoord2f(1, 1);
			qglVertex3f(dstX2, dstY, 0);
			qglTexCoord2f(1, 0);
			qglVertex3f(dstX2, dstY2, 0);
			qglTexCoord2f(0, 0);
			qglVertex3f(dstX, dstY2, 0);
			qglEnd();

			qglBindTexture(GL_TEXTURE_2D, 0);
			qglMatrixMode(GL_PROJECTION);
			qglPopMatrix();
			qglMatrixMode(GL_MODELVIEW);
			qglPopMatrix();
			qglDisable(GL_TEXTURE_2D);
		}
		qglBindFramebufferEXT(GL_READ_FRAMEBUFFER, 0);
	}

	//this will call GL_SwapBuffer for us.
	gRefFuncs.GL_EndRendering();
}

void R_InitCvars(void)
{
	v_gamma = gEngfuncs.pfnGetCvarPointer("gamma");
}

void R_Init(void)
{
	if(g_dwEngineBuildnum >= 5953)
		gRefFuncs.FreeFBObjects();

	R_InitCvars();
	R_InitShaders();
	R_InitRefHUD();
}
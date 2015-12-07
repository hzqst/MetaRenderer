#include <metahook.h>
#include "exportfuncs.h"
#include "gl_local.h"
#include "qgl.h"

//Error when can't find sig
void Sys_ErrorEx(const char *fmt, ...)
{
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if(gEngfuncs.pfnClientCmd)
		gEngfuncs.pfnClientCmd("escape\n");

	MessageBox(NULL, msg, "Error", MB_ICONERROR);
	exit(0);
}

char *UTIL_VarArgs(char *format, ...)
{
	va_list argptr;
	static int index = 0;
	static char string[16][1024];

	va_start(argptr, format);
	vsprintf(string[index], format, argptr);
	va_end(argptr);

	char *result = string[index];
	index = (index + 1) % 16;
	return result;
}

cl_enginefunc_t gEngfuncs;

void HUD_Init(void)
{
	gExportfuncs.HUD_Init();

	R_Init();
}

int HUD_Redraw(float time, int intermission)
{
	if(r_hdr_debug->value)
	{
		qglDisable(GL_BLEND);
		qglDisable(GL_ALPHA_TEST);
		qglColor4f(1,1,1,1);

		qglEnable(GL_TEXTURE_2D);
		FBO_Container_t *pFBO = NULL;
		switch((int)r_hdr_debug->value)
		{
		case 1:
			pFBO = &s_DownSampleFBO[0];break;
		case 2:
			pFBO = &s_DownSampleFBO[1];break;
		case 3:
			pFBO = &s_BrightPassFBO;break;
		case 4:
			pFBO = &s_BlurPassFBO[0][0];break;
		case 5:
			pFBO = &s_BlurPassFBO[0][1];break;
		case 6:
			pFBO = &s_BlurPassFBO[1][0];break;
		case 7:
			pFBO = &s_BlurPassFBO[1][1];break;
		case 8:
			pFBO = &s_BlurPassFBO[2][0];break;
		case 9:
			pFBO = &s_BlurPassFBO[2][1];break;
		case 10:
			pFBO = &s_BrightAccumFBO;break;
		case 11:
			pFBO = &s_ToneMapFBO;break;
		default:
			break;
		}

		if(pFBO)
		{
			qglBindTexture(GL_TEXTURE_2D, pFBO->s_hBackBufferTex);
			qglBegin(GL_QUADS);
			qglTexCoord2f(0,1);
			qglVertex3f(0,0,0);
			qglTexCoord2f(1,1);
			qglVertex3f(glwidth/2, 0,0);
			qglTexCoord2f(1,0);
			qglVertex3f(glwidth/2, glheight/2,0);
			qglTexCoord2f(0,0);
			qglVertex3f(0, glheight/2,0);
			qglEnd();
		}
	}
	return gExportfuncs.HUD_Redraw(time, intermission);
}

void hudGetMousePos(struct tagPOINT *ppt)
{
	gEngfuncs.pfnGetMousePos(ppt);

	/*if ( !g_bWindowed && g_pBTEClient )
	{
		RECT rectWin;
		GetWindowRect(g_pBTEClient->GetMainHWND(), &rectWin);
		int videoW = g_iVideoWidth;
		int videoH = g_iVideoHeight;
		int winW = rectWin.right - rectWin.left;
		int winH = rectWin.bottom - rectWin.top;
		ppt->x *= (float)videoW / winW;
		ppt->y *= (float)videoH / winH;
		ppt->x *= (*windowvideoaspect - 1) * (ppt->x - videoW / 2);
		ppt->y *= (*videowindowaspect - 1) * (ppt->y - videoH / 2);
	}*/
}

void hudGetMousePosition(int *x, int *y)
{
	gEngfuncs.GetMousePosition(x, y);

	/*if ( !g_bWindowed && g_pBTEClient )
	{
		RECT rectWin;
		GetWindowRect(g_pBTEClient->GetMainHWND(), &rectWin);
		int videoW = g_iVideoWidth;
		int videoH = g_iVideoHeight;
		int winW = rectWin.right - rectWin.left;
		int winH = rectWin.bottom - rectWin.top;
		*x *= (float)videoW / winW;
		*y *= (float)videoH / winH;
		*x *= (*windowvideoaspect - 1) * (*x - videoW / 2);
		*y *= (*videowindowaspect - 1) * (*y - videoH / 2);
	}*/
}
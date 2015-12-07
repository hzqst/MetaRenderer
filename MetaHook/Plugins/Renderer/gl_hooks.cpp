#include "gl_local.h"

#define R_CLEAR_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xDC\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0"
#define R_CLEAR_SIG2 "\x8B\x15\x2A\x2A\x2A\x2A\x33\xC0\x83\xFA\x01\x0F\x9F\xC0\x50\xE8\x2A\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xDC\x1D\x2A\x2A\x2A\x2A\x83\xC4\x04\xDF\xE0"
#define R_CLEAR_SIG_NEW "\x8B\x15\x2A\x2A\x2A\x2A\x33\xC0\x83\xFA\x01\x0F\x9F\xC0\x50\xE8\x2A\x2A\x2A\x2A\xD9\x05\x2A\x2A\x2A\x2A\xDC\x1D\x2A\x2A\x2A\x2A\x83\xC4\x04"

#define GL_BIND_SIG "\x8B\x44\x24\x04\x8B\x0D\x2A\x2A\x2A\x2A\x56\x8B\xF0\xC1\xFE\x10\x25\xFF\xFF\x00\x00\x4E\x3B\xC8"
#define GL_BIND_SIG_NEW "\x55\x8B\xEC\x8B\x45\x08\x8B\x0D\x2A\x2A\x2A\x2A\x56\x8B\xF0\xC1\xFE\x10\x25\xFF\xFF\x00\x00\x4E\x3B\xC8"

#define GL_SELECTTEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x56\x85\xC0\x2A\x2A\x8B\x74\x24\x08\x56\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x3B\xF0"
#define GL_SELECTTEXTURE_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x56\x85\xC0\x2A\x2A\x8B\x75\x08\x56\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x3B\xF0"

#define GL_BEGINRENDERING_SIG "\x8B\x44\x24\x08\x8B\x4C\x24\x04\x8B\x54\x24\x0C\xC7\x00\x00\x00\x00\x00\xA1\x2A\x2A\x2A\x2A\xC7\x01\x00\x00\x00\x00\x8B\x0D\x2A\x2A\x2A\x2A\x2B\xC1"
#define GL_BEGINRENDERING_SIG_NEW "\x55\x8B\xEC\x8B\x45\x0C\x8B\x4D\x08\x56\x57\xC7\x00\x00\x00\x00\x00\xC7\x01\x00\x00\x00\x00\xE8"

#define GL_ENDRENDERING_SIG "\xFF\x25\x2A\x2A\x2A\x2A\x90"
#define GL_ENDRENDERING_SIG_NEW "\x55\x8B\xEC\x83\xEC\x34\xA1\x2A\x2A\x2A\x2A\x53\x33\xDB"

#define R_RENDERVIEW_SIG "\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x83\xEC\x14\xDF\xE0\xF6\xC4"
#define R_RENDERVIEW_SIG_NEW "\x55\x8B\xEC\x83\xEC\x14\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4\x44"

#define FREEFBOBJECTS_SIG_NEW "\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x3B\xC6\x74\x0D\x68\x2A\x2A\x2A\x2A\x6A\x01\xFF\x15"

#define GL_DISABLEMULTITEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\x68\xE1\x0D\x00\x00\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8"
#define GL_DISABLEMULTITEXTURE_SIG_NEW "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\x68\xE1\x0D\x00\x00\xFF\x15\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8"

#define GL_ENABLEMULTITEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x68\xE1\x0D\x00\x00\xFF\x15"
#define GL_ENABLEMULTITEXTURE_SIG_NEW "\xA1\x2A\x2A\x2A\x2A\x85\xC0\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x68\xE1\x0D\x00\x00\xFF\x15"

#define GL_LOADTEXTURE2_SIG "\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x53\x8B\x9C\x24\x14\x40\x00\x00\x55\x56\x8A\x03\x33\xF6"
#define GL_LOADTEXTURE2_SIG2 "\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x84\x24\x10\x40\x00\x00\x53\x55\x33\xDB\x8A\x08\x56\x84\xC9\x57\x89\x5C\x24\x14"
#define GL_LOADTEXTURE2_SIG_NEW "\x55\x8B\xEC\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x45\x08\x53\x33\xDB\x56\x8A\x08\x57\x84\xC9\x89\x5D\xF4\x74\x2A\x33\xF6"

#define VID_UPDATEWINDOWVARS_SIG "\x56\x8B\x74\x24\x08\x8B\xC6\x8B\x08\x89\x0D\x2A\x2A\x2A\x2A\x8B\x50\x04\x89\x15"
#define VID_UPDATEWINDOWVARS_SIG_NEW "\x55\x8B\xEC\x51\x56\x8B\x75\x08\x8B\xC6\x8B\x08\x89\x0D\x2A\x2A\x2A\x2A\x8B\x50\x04\x89\x15"

void Sys_ErrorEx(const char *fmt, ...);

void R_FillAddress(void)
{
	DWORD addr;
	if(g_iVideoMode == 2)
	{
		Sys_ErrorEx("D3D mode is not supported.");
	}
	if(g_iVideoMode == 0)
	{
		Sys_ErrorEx("Software mode is not supported.");
	}
	if (g_dwEngineBuildnum >= 5953)
	{
		gRefFuncs.GL_Bind = (void (*)(int))
			Search_Pattern(GL_BIND_SIG_NEW);
		Sig_FuncNotFound(GL_Bind);

		gRefFuncs.GL_SelectTexture = (void (*)(GLenum))
			Search_Pattern_From(GL_Bind, GL_SELECTTEXTURE_SIG_NEW);
		Sig_FuncNotFound(GL_SelectTexture);

		gRefFuncs.GL_LoadTexture2 = (int (*)(char *, int, int, int, byte *, qboolean, int, byte *, int))
			Search_Pattern_From(GL_SelectTexture, GL_LOADTEXTURE2_SIG_NEW);
		Sig_FuncNotFound(GL_LoadTexture2);

		gRefFuncs.R_RenderView = (void (*)(void))
			Search_Pattern(R_RENDERVIEW_SIG_NEW);
		Sig_FuncNotFound(R_RenderView);

		gRefFuncs.GL_DisableMultitexture = (void (*)(void))
			Search_Pattern(GL_DISABLEMULTITEXTURE_SIG_NEW);
		Sig_FuncNotFound(GL_DisableMultitexture);

		gRefFuncs.GL_EnableMultitexture = (void (*)(void))
			Search_Pattern_From(GL_DisableMultitexture, GL_ENABLEMULTITEXTURE_SIG_NEW);
		Sig_FuncNotFound(GL_EnableMultitexture);

		gRefFuncs.GL_BeginRendering = (void (*)(int *, int *, int *, int *))
			Search_Pattern(GL_BEGINRENDERING_SIG_NEW);
		Sig_FuncNotFound(GL_BeginRendering);

		gRefFuncs.GL_EndRendering = (void (*)(void))
			Search_Pattern_From(GL_BeginRendering, GL_ENDRENDERING_SIG_NEW);
		Sig_FuncNotFound(GL_EndRendering);

		gRefFuncs.VID_UpdateWindowVars = (void (*)(RECT *prc, int x, int y))
			Search_Pattern(VID_UPDATEWINDOWVARS_SIG_NEW);
		Sig_FuncNotFound(VID_UpdateWindowVars);

		//5953 above only
		gRefFuncs.FreeFBObjects = (void (*)(void))
			Search_Pattern_From(GL_EndRendering, FREEFBOBJECTS_SIG_NEW);
		Sig_FuncNotFound(FreeFBObjects);
	}
	else
	{
		gRefFuncs.GL_Bind = (void (*)(int))
			Search_Pattern(GL_BIND_SIG);
		Sig_FuncNotFound(GL_Bind);

		gRefFuncs.GL_SelectTexture = (void (*)(GLenum))
			Search_Pattern_From(GL_Bind, GL_SELECTTEXTURE_SIG);
		Sig_FuncNotFound(GL_SelectTexture);

		gRefFuncs.GL_LoadTexture2 = (int (*)(char *, int, int, int, byte *, qboolean, int, byte *, int))
			Search_Pattern_From(GL_SelectTexture, GL_LOADTEXTURE2_SIG);
		if (!gRefFuncs.GL_LoadTexture2)
			gRefFuncs.GL_LoadTexture2 = (int(*)(char *, int, int, int, byte *, qboolean, int, byte *, int))
			Search_Pattern_From(GL_SelectTexture, GL_LOADTEXTURE2_SIG2);
		Sig_FuncNotFound(GL_LoadTexture2);

		gRefFuncs.R_RenderView = (void (*)(void))
			Search_Pattern(R_RENDERVIEW_SIG);
		Sig_FuncNotFound(R_RenderView);

		gRefFuncs.GL_DisableMultitexture = (void (*)(void))
			Search_Pattern(GL_DISABLEMULTITEXTURE_SIG);
		if(!gRefFuncs.GL_DisableMultitexture)
			Sig_FuncNotFound(GL_DisableMultitexture);

		gRefFuncs.GL_EnableMultitexture = (void (*)(void))
			Search_Pattern_From(GL_DisableMultitexture, GL_ENABLEMULTITEXTURE_SIG);
		if(!gRefFuncs.GL_EnableMultitexture)
			Sig_FuncNotFound(GL_EnableMultitexture);

		gRefFuncs.GL_BeginRendering = (void (*)(int *, int *, int *, int *))
			Search_Pattern(GL_BEGINRENDERING_SIG);
		Sig_FuncNotFound(GL_BeginRendering);

		gRefFuncs.GL_EndRendering = (void (*)(void))
			Search_Pattern_From(GL_BeginRendering, GL_ENDRENDERING_SIG);
		Sig_FuncNotFound(GL_EndRendering);

		gRefFuncs.VID_UpdateWindowVars = (void (*)(RECT *prc, int x, int y))
			Search_Pattern(VID_UPDATEWINDOWVARS_SIG);
		Sig_FuncNotFound(VID_UpdateWindowVars);
	}

#define WINDOW_RECT_SIG "\x89\x0D"
		//89 0D 20 42 7B 02		mov     window_rect.left, ecx
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.VID_UpdateWindowVars, 0x50, WINDOW_RECT_SIG, sizeof(WINDOW_RECT_SIG)-1);
		Sig_AddrNotFound(window_rect);
		window_rect = *(RECT **)(addr+2);

	//Engine special

	if(g_dwEngineBuildnum >= 5953)
	{
		//5953 use qglGenTexture instead of currenttexid

		//6153 have to use windowvideoaspect
#define WINDOW_VIDEO_ASPECT_SIG_NEW "\xA3\x2A\x2A\x2A\x2A\xA3\x2A\x2A\x2A\x2A\xA1"
		//A3 F8 4D E4 01		mov     videowindowaspect, eax
		//A3 F4 4D E4 01		mov     windowvideoaspect, eax
		//A1 F0 4D E4 01		mov     eax, bNoStretchAspect
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_EndRendering, 0x200, WINDOW_VIDEO_ASPECT_SIG_NEW, sizeof(WINDOW_VIDEO_ASPECT_SIG_NEW)-1);
		Sig_AddrNotFound(windowvideoaspect);
		videowindowaspect = *(float **)(addr+1);
		windowvideoaspect = *(float **)(addr+6);
	}
	else
	{
		//Below 5953 only
#define CURRENTTEXID_SIG "\x83\xC4\x04\xA1\x2A\x2A\x2A\x2A\x89\x06\x40\xA3"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.GL_LoadTexture2, 0x300, CURRENTTEXID_SIG, sizeof(CURRENTTEXID_SIG)-1);
		//add esp, 4
		//mov eax, currenttexid
		Sig_AddrNotFound(currenttexid);
		currenttexid = *(int **)(addr+4);

		//< 5953 don't have videowindowaspect & windowvideoaspect so we create one;
		videowindowaspect = &videowindowaspect_old;
		windowvideoaspect = &windowvideoaspect_old;
	}
}

void R_InstallHook(void)
{
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_BeginRendering, GL_BeginRendering, (void *&)gRefFuncs.GL_BeginRendering);
	g_pMetaHookAPI->InlineHook(gRefFuncs.GL_EndRendering, GL_EndRendering, (void *&)gRefFuncs.GL_EndRendering);
	g_pMetaHookAPI->InlineHook(gRefFuncs.R_RenderView, R_RenderView, (void *&)gRefFuncs.R_RenderView);
}
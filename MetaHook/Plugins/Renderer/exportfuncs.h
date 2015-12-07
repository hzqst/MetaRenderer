#pragma once

void HUD_Init(void);
int HUD_Redraw(float time, int intermission);

void Sys_ErrorEx(const char *fmt, ...);
char *UTIL_VarArgs(char *format, ...);
void hudGetMousePos(struct tagPOINT *ppt);
void hudGetMousePosition(int *x, int *y);

#define GetCallAddress(addr) (addr + (*(DWORD *)((addr)+1)) + 5)
#define Sig_NotFound(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum£º%d", #name, g_dwEngineBuildnum);
#define Sig_FuncNotFound(name) if(!gRefFuncs.name) Sig_NotFound(name)
#define Sig_AddrNotFound(name) if(!addr) Sig_NotFound(name)
#define SIG_NOT_FOUND(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum£º%d", name, g_dwEngineBuildnum);

#define Sig_Length(a) (sizeof(a)-1)
#define Search_Pattern(sig) g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, sig, Sig_Length(sig));
#define Search_Pattern_From(fn, sig) g_pMetaHookAPI->SearchPattern((void *)gRefFuncs.fn, g_dwEngineSize - (DWORD)gRefFuncs.fn + g_dwEngineBase, sig, Sig_Length(sig));
#define InstallHook(fn) g_pMetaHookAPI->InlineHook((void *)gRefFuncs.fn, fn, (void *&)gRefFuncs.fn);
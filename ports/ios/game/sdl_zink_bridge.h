/* First-playable bridge: desktop GL via OSMesa, CPU readback to SDL Metal.
 * Initial correctness path, not a zero-copy/performance solution. */
#pragma once
#include "SDL.h"
/* SDL suppresses desktop GL declarations on iOS. Mesa provides that API here;
 * restore the platform macro immediately after importing declarations only. */
#ifdef QVR_IOS
#pragma push_macro("__IPHONEOS__")
#undef __IPHONEOS__
#include "SDL_opengl.h"
#pragma pop_macro("__IPHONEOS__")
#endif
#ifdef __cplusplus
extern "C" {
#endif
int QVR_IOSCanRender(void);
void QVR_IOSSuspend(void);
void QVR_IOSResume(void);
SDL_Window *QVR_CreateWindow(const char*,int,int,int,int,Uint32);
SDL_GLContext QVR_GL_CreateContext(SDL_Window*);
void QVR_GL_DeleteContext(SDL_GLContext);
void *QVR_GL_GetProcAddress(const char*);
void QVR_GL_SwapWindow(SDL_Window*);
int QVR_GL_SetAttribute(SDL_GLattr,int);
int QVR_GL_GetAttribute(SDL_GLattr,int*);
int QVR_GL_SetSwapInterval(int);
int QVR_GL_GetSwapInterval(void);
SDL_GLContext QVR_GL_GetCurrentContext(void);
void QVR_GL_GetDrawableSize(SDL_Window*,int*,int*);
#ifdef __cplusplus
}
#endif
#ifndef QVR_BRIDGE_IMPL
#define SDL_CreateWindow QVR_CreateWindow
#define SDL_GL_CreateContext QVR_GL_CreateContext
#define SDL_GL_DeleteContext QVR_GL_DeleteContext
#define SDL_GL_GetProcAddress QVR_GL_GetProcAddress
#define SDL_GL_SwapWindow QVR_GL_SwapWindow
#define SDL_GL_SetAttribute QVR_GL_SetAttribute
#define SDL_GL_GetAttribute QVR_GL_GetAttribute
#define SDL_GL_SetSwapInterval QVR_GL_SetSwapInterval
#define SDL_GL_GetSwapInterval QVR_GL_GetSwapInterval
#define SDL_GL_GetCurrentContext QVR_GL_GetCurrentContext
#define SDL_GL_GetDrawableSize QVR_GL_GetDrawableSize
#endif

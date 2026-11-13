#ifndef QVR_BRIDGE_IMPL
#define QVR_BRIDGE_IMPL
#endif
#include "sdl_zink_bridge.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static void *mesa,*ctx;
static void*(*create_context)(const int*,void*);
static unsigned char(*make_current)(void*,void*,unsigned,int,int);
static void(*destroy_context)(void*);
static void*(*get_proc)(const char*);
static SDL_Renderer *renderer;
static SDL_Window *game_window;
static SDL_Texture *texture;
static unsigned char *pixels;
static int width=640,height=360,attrs[64],interval=1;
SDL_Window *QVR_CreateWindow(const char*t,int x,int y,int w,int h,Uint32 flags){
 (void)w;(void)h;
 return game_window=SDL_CreateWindow(t,x,y,width,height,(flags&~SDL_WINDOW_OPENGL)|SDL_WINDOW_ALLOW_HIGHDPI);
}
int QVR_GL_SetAttribute(SDL_GLattr a,int value){if((unsigned)a<64)attrs[a]=value;return 0;}
int QVR_GL_GetAttribute(SDL_GLattr a,int*v){if(!v)return -1;*v=(unsigned)a<64?attrs[a]:0;return 0;}
int QVR_GL_SetSwapInterval(int v){interval=v;return 0;}
int QVR_GL_GetSwapInterval(void){return interval;}
SDL_GLContext QVR_GL_GetCurrentContext(void){return ctx;}
void QVR_GL_GetDrawableSize(SDL_Window*w,int*x,int*y){(void)w;if(x)*x=width;if(y)*y=height;}
void *QVR_GL_GetProcAddress(const char*n){return get_proc?get_proc(n):NULL;}
void QVR_GL_DeleteContext(SDL_GLContext c){
 if(c&&destroy_context)destroy_context(c);
 ctx=NULL;
 if(texture)SDL_DestroyTexture(texture);
 texture=NULL;
 if(renderer)SDL_DestroyRenderer(renderer);
 renderer=NULL;
 free(pixels);pixels=NULL;
 /* Mesa module retained for process lifetime: driver threads can outlive context. */
}
SDL_GLContext QVR_GL_CreateContext(SDL_Window*w){
 if(ctx)return ctx;
 const char*path=getenv("QVR_OSMESA_LIBRARY");const char*vk=getenv("QVR_MOLTENVK_LIBRARY");
 if(!path||!vk){SDL_SetError("Missing bundled Mesa/MoltenVK paths");return NULL;}
 setenv("GALLIUM_DRIVER","zink",1);setenv("ZINK_VULKAN_LIBRARY",vk,1);
 setenv("MESA_GL_VERSION_OVERRIDE","4.3",1);setenv("MESA_GLSL_VERSION_OVERRIDE","430",1);
 setenv("MESA_SHADER_CACHE_DISABLE","true",1);
 mesa=dlopen(path,RTLD_NOW|RTLD_LOCAL);if(!mesa){SDL_SetError("OSMesa load: %s",dlerror());return NULL;}
 create_context=dlsym(mesa,"OSMesaCreateContextAttribs");make_current=dlsym(mesa,"OSMesaMakeCurrent");destroy_context=dlsym(mesa,"OSMesaDestroyContext");get_proc=dlsym(mesa,"OSMesaGetProcAddress");
 if(!create_context||!make_current||!destroy_context||!get_proc){SDL_SetError("Incomplete OSMesa API");return NULL;}
 const int a[]={0x22,0x1908,0x30,24,0x31,8,0x33,0x34,0x36,4,0x37,3,0};
 ctx=create_context(a,NULL);pixels=calloc((size_t)width*height,4);
 if(!ctx||!pixels||!make_current(ctx,pixels,0x1401,width,height))goto fail;
 const unsigned char*(*str)(unsigned)=get_proc("glGetString");const char*r=str?(const char*)str(0x1F01):NULL;
 if(!r||(!strstr(r,"zink")&&!strstr(r,"Zink"))){SDL_SetError("Expected Zink renderer, got %s",r?r:"null");goto fail;}
 SDL_SetHint(SDL_HINT_RENDER_DRIVER,"metal");renderer=SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
 if(!renderer)goto fail;
 texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STREAMING,width,height);if(!texture)goto fail;
 attrs[SDL_GL_DEPTH_SIZE]=24;attrs[SDL_GL_STENCIL_SIZE]=8;
 fprintf(stderr,"UNLEASHED initial iOS bridge: %s, %dx%d CPU readback to Metal\n",r,width,height);return ctx;
 fail:QVR_GL_DeleteContext(ctx);return NULL;
}
void QVR_GL_SwapWindow(SDL_Window*w){
 static unsigned presents;
 (void)w;if(!ctx||!texture)return;
 if(presents<3)fprintf(stderr,"UNLEASHED present %u: readback begin\n",presents);
 void(*read)(int,int,int,int,unsigned,unsigned,void*)=get_proc("glReadPixels");
 if(!read)return;
 read(0,0,width,height,0x1908,0x1401,pixels);
 if(SDL_UpdateTexture(texture,NULL,pixels,width*4)!=0 || SDL_RenderClear(renderer)!=0 ||
    SDL_RenderCopyEx(renderer,texture,NULL,NULL,0,NULL,SDL_FLIP_VERTICAL)!=0){
   if(presents<3)fprintf(stderr,"UNLEASHED present failed: %s\n",SDL_GetError());
 }else{SDL_RenderPresent(renderer);}
 if(presents<3)fprintf(stderr,"UNLEASHED present %u: returned\n",presents);
 ++presents;
}

#ifdef QVR_IOS
int QVR_IOSStartFrames(void (*callback)(void *)) {
 return SDL_iPhoneSetAnimationCallback(game_window,1,callback,NULL);
}
#endif

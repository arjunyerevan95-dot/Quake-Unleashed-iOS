/* Host harness only. Its PASS is not evidence for iOS/Zink/Metal. */
#include "probe.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
static void *lib;
static void *(*getproc)(const char*);
static void *resolve(const char*n){void*p=getproc(n);return p?p:dlsym(lib,n);}
int main(void){
 lib=dlopen("libEGL.so.1",RTLD_NOW|RTLD_LOCAL);if(!lib){puts(dlerror());return 2;}
 getproc=dlsym(lib,"eglGetProcAddress");
 void*(*platform)(unsigned,void*,const int*)=getproc("eglGetPlatformDisplayEXT");
 unsigned(*init)(void*,int*,int*)=dlsym(lib,"eglInitialize");
 unsigned(*bind)(unsigned)=dlsym(lib,"eglBindAPI");
 unsigned(*choose)(void*,const int*,void**,int,int*)=dlsym(lib,"eglChooseConfig");
 void*(*context)(void*,void*,void*,const int*)=dlsym(lib,"eglCreateContext");
 void*(*surface)(void*,void*,const int*)=dlsym(lib,"eglCreatePbufferSurface");
 unsigned(*current)(void*,void*,void*,void*)=dlsym(lib,"eglMakeCurrent");
 unsigned(*destroyC)(void*,void*)=dlsym(lib,"eglDestroyContext");
 unsigned(*destroyS)(void*,void*)=dlsym(lib,"eglDestroySurface");
 unsigned(*terminate)(void*)=dlsym(lib,"eglTerminate");
 if(!platform||!init||!bind||!choose||!context||!surface||!current){puts("EGL API incomplete");return 2;}
 void*d=platform(0x31DD,0,0);int major,minor;if(!init(d,&major,&minor)||!bind(0x30A2)){puts("EGL init/bind failed");return 2;}
 const int ca[]={0x3033,1,0x3040,8,0x3024,8,0x3023,8,0x3022,8,0x3038};void*cfg;int n;
 if(!choose(d,ca,&cfg,1,&n)||n<1){puts("EGL config missing");return 2;}
 const int ctx[]={0x3098,4,0x30FB,3,0x30FD,1,0x3038};void*c=context(d,cfg,0,ctx);
 const int pb[]={0x3057,16,0x3056,16,0x3038};void*s=surface(d,cfg,pb);
 if(!c||!s||!current(d,s,s,c)){puts("EGL GL4.3 context failed");return 2;}
 qvru_probe_result r=qvru_probe(resolve);puts(r.detail);
 current(d,0,0,0);destroyC(d,c);destroyS(d,s);terminate(d);dlclose(lib);
 return r.passed?0:1;
}

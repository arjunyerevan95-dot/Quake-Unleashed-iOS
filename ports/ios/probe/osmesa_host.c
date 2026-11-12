/* Independent native OSMesa adapter. No Wine or BCD source is included. */
#include "probe.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
qvru_probe_result qvru_osmesa_probe(const char *mesa, const char *vulkan, int experimental_override) {
 qvru_probe_result r={0};
 /* Version override is diagnostic only: never let inherited env silently claim support. */
 unsetenv("MESA_GL_VERSION_OVERRIDE");unsetenv("MESA_GLSL_VERSION_OVERRIDE");
 if(experimental_override){setenv("MESA_GL_VERSION_OVERRIDE","4.3",1);setenv("MESA_GLSL_VERSION_OVERRIDE","430",1);}
 setenv("GALLIUM_DRIVER","zink",1);setenv("ZINK_VULKAN_LIBRARY",vulkan,1);
 setenv("MESA_SHADER_CACHE_DISABLE","true",1);
 void*lib=dlopen(mesa,RTLD_NOW|RTLD_LOCAL);
 if(!lib){snprintf(r.detail,sizeof(r.detail),"OSMesa load failed: %s",dlerror());return r;}
 void*(*create)(const int*,void*)=dlsym(lib,"OSMesaCreateContextAttribs");
 unsigned char(*current)(void*,void*,unsigned,int,int)=dlsym(lib,"OSMesaMakeCurrent");
 void(*destroy)(void*)=dlsym(lib,"OSMesaDestroyContext");
 void*(*get)(const char*)=dlsym(lib,"OSMesaGetProcAddress");
 if(!create||!current||!destroy||!get){snprintf(r.detail,sizeof(r.detail),"OSMesa API incomplete");dlclose(lib);return r;}
 const int attrs[]={0x22,0x1908,0x30,24,0x31,8,0x33,0x34,0x36,4,0x37,3,0};
 void*ctx=create(attrs,0);unsigned char pixels[16*16*4]={0};
 if(!ctx){snprintf(r.detail,sizeof(r.detail),"GL4.3 core context rejected (%s). No renderer compatibility proven.",experimental_override?"explicit experimental override":"honest capabilities");dlclose(lib);return r;}
 if(!current(ctx,pixels,0x1401,16,16)){snprintf(r.detail,sizeof(r.detail),"OSMesaMakeCurrent failed");destroy(ctx);dlclose(lib);return r;}
 r=qvru_probe(get);
 /* Require actual Zink, not an unnoticed CPU softpipe fallback. */
 const unsigned char*(*str)(unsigned)=get("glGetString");
 const char*renderer=str?(const char*)str(0x1F01):0;
 if(!renderer || (!strstr(renderer,"zink")&&!strstr(renderer,"Zink"))){r.passed=0;snprintf(r.detail,sizeof(r.detail),"Unexpected renderer: %s; requires Zink for this experiment",renderer?renderer:"null");}
 if(experimental_override){char detail[2048];snprintf(detail,sizeof(detail),"EXPERIMENTAL GL VERSION OVERRIDE. Feature results only, not GL4.3 conformance: %.1900s",r.detail);snprintf(r.detail,sizeof(r.detail),"%s",detail);}
 current(0,0,0,0,0);destroy(ctx);dlclose(lib);return r;
}
#ifdef QVRU_OSMESA_CLI
int main(int argc,char**argv){
 if(argc<3){fprintf(stderr,"usage: probe libOSMesa libMoltenVK [--experimental-override]\n");return 2;}
 int exp=argc==4&&!strcmp(argv[3],"--experimental-override");
 qvru_probe_result r=qvru_osmesa_probe(argv[1],argv[2],exp);puts(r.detail);return r.passed?0:1;
}
#endif

/* Asset-free engine shader syntax gate. Not rendering or full permutation coverage. */
#include "../probe/probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
qvru_probe_result qvru_probe(qvru_get_proc get){
 qvru_probe_result r={0}; const char*dir=getenv("QVR_SHADER_DIR"); if(!dir)dir=".";
 unsigned(*create)(unsigned)=get("glCreateShader");
 void(*source)(unsigned,int,const char*const*,const int*)=get("glShaderSource");
 void(*compile)(unsigned)=get("glCompileShader");
 void(*status)(unsigned,unsigned,int*)=get("glGetShaderiv");
 void(*log)(unsigned,int,int*,char*)=get("glGetShaderInfoLog");
 void(*destroy)(unsigned)=get("glDeleteShader");
 if(!create||!source||!compile||!status||!log||!destroy){snprintf(r.detail,sizeof(r.detail),"Missing shader API");return r;}
 DIR*d=opendir(dir);if(!d){snprintf(r.detail,sizeof(r.detail),"Missing corpus");return r;} struct dirent*e;int total=0,failed=0;char first_error[1200]={0};
 while((e=readdir(d))){if(!strstr(e->d_name,".glsl"))continue;unsigned type=strstr(e->d_name,"vertex")?0x8B31:strstr(e->d_name,"fragment")?0x8B30:0x91B9;
  char path[2048];snprintf(path,sizeof(path),"%s/%s",dir,e->d_name);FILE*f=fopen(path,"rb");if(!f){failed++;continue;}fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<0||size>2000000){fclose(f);failed++;continue;}char*body=calloc((size_t)size+1,1);if(!body){fclose(f);failed++;continue;}if(fread(body,1,(size_t)size,f)!=(size_t)size){free(body);fclose(f);failed++;continue;}fclose(f);
  int variants=strstr(e->d_name,"alias_")?3:1;
  for(int pose=0;pose<variants;pose++){char header[512];snprintf(header,sizeof(header),"#version 430\n#define BINDLESS 0\n#define REVERSED_Z 0\n#define QVR_SHADER_AB 0\n#define MODE 0\n#define OIT 0\n#define DITHER 0\n#define ALPHATEST 0\n#define POSEVERTTYPE %d\n#define PALETTIZE 0\n#define WARP 0\n#define ANIM 0\n#define MSAA 0\n",pose);
   const char*parts[]={header,body};unsigned shader=create(type);source(shader,2,parts,0);compile(shader);int ok=0;status(shader,0x8B81,&ok);total++;if(!ok){char error[4096]={0};log(shader,sizeof(error),0,error);printf("FAIL %s pose=%d: %s\n",e->d_name,pose,error);if(!first_error[0])snprintf(first_error,sizeof(first_error),"%.120s pose=%d: %.950s",e->d_name,pose,error);failed++;}else printf("PASS %s pose=%d\n",e->d_name,pose);destroy(shader);
  }free(body);
 }closedir(d);r.passed=total>0&&!failed;snprintf(r.detail,sizeof(r.detail),"Engine stage syntax gate: %d tests, %d failures. No program linking, draw validation or full variants. First error: %s",total,failed,first_error);return r;
}

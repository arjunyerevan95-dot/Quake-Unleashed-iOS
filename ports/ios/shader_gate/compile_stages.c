/* Asset-free engine shader syntax gate. Not rendering or full permutation coverage. */
#include "../probe/probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
int qvru_render_gui(qvru_get_proc,unsigned,unsigned char*);
qvru_probe_result qvru_probe(qvru_get_proc get){
 qvru_probe_result r={0}; const char*dir=getenv("QVR_SHADER_DIR"); if(!dir)dir=".";
 unsigned(*create)(unsigned)=get("glCreateShader");
 void(*source)(unsigned,int,const char*const*,const int*)=get("glShaderSource");
 void(*compile)(unsigned)=get("glCompileShader");
 void(*status)(unsigned,unsigned,int*)=get("glGetShaderiv");
 void(*log)(unsigned,int,int*,char*)=get("glGetShaderInfoLog");
 void(*destroy)(unsigned)=get("glDeleteShader");
 if(!create||!source||!compile||!status||!log||!destroy){snprintf(r.detail,sizeof(r.detail),"Missing shader API");return r;}
 unsigned(*createP)(void)=get("glCreateProgram");
 void(*attach)(unsigned,unsigned)=get("glAttachShader");
 void(*link)(unsigned)=get("glLinkProgram");
 void(*pstatus)(unsigned,unsigned,int*)=get("glGetProgramiv");
 void(*plog)(unsigned,int,int*,char*)=get("glGetProgramInfoLog");
 void(*deleteP)(unsigned)=get("glDeleteProgram");
 if(!createP||!attach||!link||!pstatus||!plog||!deleteP){snprintf(r.detail,sizeof(r.detail),"Missing link API");return r;}
 struct {char name[256];int pose;unsigned shader,type;} cache[64];int count=0,links=0,linkfails=0,gui_draw=0;
 DIR*d=opendir(dir);if(!d){snprintf(r.detail,sizeof(r.detail),"Missing corpus");return r;} struct dirent*e;int total=0,failed=0;char first_error[1200]={0};
 while((e=readdir(d))){if(!strstr(e->d_name,".glsl"))continue;unsigned type=strstr(e->d_name,"vertex")?0x8B31:strstr(e->d_name,"fragment")?0x8B30:0x91B9;
  char path[2048];snprintf(path,sizeof(path),"%s/%s",dir,e->d_name);FILE*f=fopen(path,"rb");if(!f){failed++;continue;}fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);if(size<0||size>2000000){fclose(f);failed++;continue;}char*body=calloc((size_t)size+1,1);if(!body){fclose(f);failed++;continue;}if(fread(body,1,(size_t)size,f)!=(size_t)size){free(body);fclose(f);failed++;continue;}fclose(f);
  int variants=strstr(e->d_name,"alias_")?3:1;
  for(int pose=0;pose<variants;pose++){char header[512];snprintf(header,sizeof(header),"#version 430\n#define BINDLESS 0\n#define REVERSED_Z 0\n#define QVR_SHADER_AB 0\n#define MODE 0\n#define OIT 0\n#define DITHER 0\n#define ALPHATEST 0\n#define POSEVERTTYPE %d\n#define PALETTIZE 0\n#define WARP 0\n#define ANIM 0\n#define MSAA 0\n",pose);
   const char*parts[]={header,body};unsigned shader=create(type);source(shader,2,parts,0);compile(shader);int ok=0;status(shader,0x8B81,&ok);total++;if(!ok){char error[4096]={0};log(shader,sizeof(error),0,error);printf("FAIL %s pose=%d: %s\n",e->d_name,pose,error);if(!first_error[0])snprintf(first_error,sizeof(first_error),"%.120s pose=%d: %.950s",e->d_name,pose,error);failed++;}else printf("PASS %s pose=%d\n",e->d_name,pose);if(ok && count<64){snprintf(cache[count].name,256,"%s",e->d_name);cache[count].pose=pose;cache[count].shader=shader;cache[count].type=type;count++;}else destroy(shader);
  }free(body);
 }closedir(d);
 for(int i=0;i<count;i++){
  if(cache[i].type==0x8B30)continue;
  unsigned program=createP();attach(program,cache[i].shader);
  if(cache[i].type==0x8B31){
   char partner[256];snprintf(partner,sizeof(partner),"%s",cache[i].name);
   char*part=strstr(partner,"vertex");if(part)snprintf(part,(size_t)(partner+sizeof(partner)-part),"fragment_shader.glsl");
   if(!strcmp(cache[i].name,"wound_paint_box_vertex_shader.glsl"))snprintf(partner,sizeof(partner),"wound_paint_fragment_shader.glsl");
   for(int j=0;j<count;j++)if(!strcmp(cache[j].name,partner)&&cache[j].pose==cache[i].pose){attach(program,cache[j].shader);break;}
  }
  link(program);int ok=0;pstatus(program,0x8B82,&ok);links++;
  if(!ok){char error[4096]={0};plog(program,sizeof(error),0,error);printf("LINK FAIL %s pose=%d %s\n",cache[i].name,cache[i].pose,error);linkfails++;if(!first_error[0])snprintf(first_error,sizeof(first_error),"LINK %.120s: %.950s",cache[i].name,error);}
  else {printf("LINK PASS %s pose=%d\n",cache[i].name,cache[i].pose);if(!strcmp(cache[i].name,"gui_vertex_shader.glsl"))gui_draw=qvru_render_gui(get,program,r.rgba);}
  deleteP(program);
 }
 for(int i=0;i<count;i++)destroy(cache[i].shader);
 r.passed=total>0&&!failed&&!linkfails&&gui_draw;snprintf(r.detail,sizeof(r.detail),"Engine gate: %d stage tests / %d failures; %d program links / %d failures; real GUI textured draw/readback: %s. No skinned-scene draw, full variants, live Metal presentation or performance proof. First error: %s",total,failed,links,linkfails,gui_draw?"PASS":"FAIL",first_error);return r;
}

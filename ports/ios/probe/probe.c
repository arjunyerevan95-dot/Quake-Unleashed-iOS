#include "probe.h"
#include <stdio.h>
#include <string.h>
typedef struct {
 const unsigned char * (*GetString)(unsigned);
 void (*GetIntegerv)(unsigned,int*);
 unsigned (*GetError)(void);
 unsigned (*CreateShader)(unsigned);
 void (*ShaderSource)(unsigned,int,const char*const*,const int*);
 void (*CompileShader)(unsigned);
 void (*GetShaderiv)(unsigned,unsigned,int*);
 void (*GetShaderInfoLog)(unsigned,int,int*,char*);
 void (*DeleteShader)(unsigned);
 unsigned (*CreateProgram)(void);
 void (*AttachShader)(unsigned,unsigned);
 void (*LinkProgram)(unsigned);
 void (*GetProgramiv)(unsigned,unsigned,int*);
 void (*GetProgramInfoLog)(unsigned,int,int*,char*);
 void (*UseProgram)(unsigned);
 void (*DeleteProgram)(unsigned);
 void (*GenBuffers)(int,unsigned*);
 void (*BindBuffer)(unsigned,unsigned);
 void (*BufferData)(unsigned,ptrdiff_t,const void*,unsigned);
 void (*BindBufferBase)(unsigned,unsigned,unsigned);
 void (*DispatchCompute)(unsigned,unsigned,unsigned);
 void (*MemoryBarrier)(unsigned);
 void* (*MapBufferRange)(unsigned,ptrdiff_t,ptrdiff_t,unsigned);
 unsigned char (*UnmapBuffer)(unsigned);
 void (*DeleteBuffers)(int,const unsigned*);
 void (*GenTextures)(int,unsigned*);
 void (*BindTexture)(unsigned,unsigned);
 void (*TexStorage2D)(unsigned,int,unsigned,int,int);
 void (*BindImageTexture)(unsigned,unsigned,int,unsigned char,int,unsigned,unsigned);
 void (*DeleteTextures)(int,const unsigned*);
 void (*GenFramebuffers)(int,unsigned*);
 void (*BindFramebuffer)(unsigned,unsigned);
 void (*FramebufferTexture2D)(unsigned,unsigned,unsigned,unsigned,int);
 unsigned (*CheckFramebufferStatus)(unsigned);
 void (*ReadPixels)(int,int,int,int,unsigned,unsigned,void*);
 void (*DeleteFramebuffers)(int,const unsigned*);
 void (*GenVertexArrays)(int,unsigned*);
 void (*BindVertexArray)(unsigned);
 void (*DeleteVertexArrays)(int,const unsigned*);
 void (*Viewport)(int,int,int,int);
 void (*DrawArrays)(unsigned,int,int);
 void (*Finish)(void);
} GL;
static unsigned shader(GL*g,unsigned type,const char*src,char*error){
 unsigned s=g->CreateShader(type); int ok=0;
 g->ShaderSource(s,1,&src,0);g->CompileShader(s);g->GetShaderiv(s,0x8B81,&ok);
 if(!ok){g->GetShaderInfoLog(s,1024,0,error);g->DeleteShader(s);return 0;}return s;
}
static unsigned program(GL*g,unsigned a,unsigned b,char*error){
 unsigned p=g->CreateProgram();int ok=0;g->AttachShader(p,a);
 if(b)g->AttachShader(p,b);
 g->LinkProgram(p);g->GetProgramiv(p,0x8B82,&ok);
 if(!ok){g->GetProgramInfoLog(p,1024,0,error);g->DeleteProgram(p);return 0;}return p;
}
qvru_probe_result qvru_probe(qvru_get_proc get){
 qvru_probe_result r={0};GL g={0};int major=0,minor=0;
 if(!get){snprintf(r.detail,sizeof(r.detail),"Missing GL resolver");return r;}
 unsigned cs=0,vs=0,fs=0,cp=0,dp=0,buf=0,tex=0,fbo=0,vao=0;
 char error[1024]={0};const char*stage="load entry points";
 g.GetString=(const unsigned char *(*)(unsigned))get("glGetString"); if(!g.GetString){snprintf(r.detail,sizeof(r.detail),"Missing glGetString");
 return r;}
 g.GetIntegerv=(void(*)(unsigned,int*))get("glGetIntegerv"); if(!g.GetIntegerv){snprintf(r.detail,sizeof(r.detail),"Missing glGetIntegerv");
 return r;}
 g.GetError=(unsigned(*)(void))get("glGetError"); if(!g.GetError){snprintf(r.detail,sizeof(r.detail),"Missing glGetError");
 return r;}
 g.CreateShader=(unsigned(*)(unsigned))get("glCreateShader"); if(!g.CreateShader){snprintf(r.detail,sizeof(r.detail),"Missing glCreateShader");
 return r;}
 g.ShaderSource=(void(*)(unsigned,int,const char*const*,const int*))get("glShaderSource"); if(!g.ShaderSource){snprintf(r.detail,sizeof(r.detail),"Missing glShaderSource");
 return r;}
 g.CompileShader=(void(*)(unsigned))get("glCompileShader"); if(!g.CompileShader){snprintf(r.detail,sizeof(r.detail),"Missing glCompileShader");
 return r;}
 g.GetShaderiv=(void(*)(unsigned,unsigned,int*))get("glGetShaderiv"); if(!g.GetShaderiv){snprintf(r.detail,sizeof(r.detail),"Missing glGetShaderiv");
 return r;}
 g.GetShaderInfoLog=(void(*)(unsigned,int,int*,char*))get("glGetShaderInfoLog"); if(!g.GetShaderInfoLog){snprintf(r.detail,sizeof(r.detail),"Missing glGetShaderInfoLog");
 return r;}
 g.DeleteShader=(void(*)(unsigned))get("glDeleteShader"); if(!g.DeleteShader){snprintf(r.detail,sizeof(r.detail),"Missing glDeleteShader");
 return r;}
 g.CreateProgram=(unsigned(*)(void))get("glCreateProgram"); if(!g.CreateProgram){snprintf(r.detail,sizeof(r.detail),"Missing glCreateProgram");
 return r;}
 g.AttachShader=(void(*)(unsigned,unsigned))get("glAttachShader"); if(!g.AttachShader){snprintf(r.detail,sizeof(r.detail),"Missing glAttachShader");
 return r;}
 g.LinkProgram=(void(*)(unsigned))get("glLinkProgram"); if(!g.LinkProgram){snprintf(r.detail,sizeof(r.detail),"Missing glLinkProgram");
 return r;}
 g.GetProgramiv=(void(*)(unsigned,unsigned,int*))get("glGetProgramiv"); if(!g.GetProgramiv){snprintf(r.detail,sizeof(r.detail),"Missing glGetProgramiv");
 return r;}
 g.GetProgramInfoLog=(void(*)(unsigned,int,int*,char*))get("glGetProgramInfoLog"); if(!g.GetProgramInfoLog){snprintf(r.detail,sizeof(r.detail),"Missing glGetProgramInfoLog");
 return r;}
 g.UseProgram=(void(*)(unsigned))get("glUseProgram"); if(!g.UseProgram){snprintf(r.detail,sizeof(r.detail),"Missing glUseProgram");
 return r;}
 g.DeleteProgram=(void(*)(unsigned))get("glDeleteProgram"); if(!g.DeleteProgram){snprintf(r.detail,sizeof(r.detail),"Missing glDeleteProgram");
 return r;}
 g.GenBuffers=(void(*)(int,unsigned*))get("glGenBuffers"); if(!g.GenBuffers){snprintf(r.detail,sizeof(r.detail),"Missing glGenBuffers");
 return r;}
 g.BindBuffer=(void(*)(unsigned,unsigned))get("glBindBuffer"); if(!g.BindBuffer){snprintf(r.detail,sizeof(r.detail),"Missing glBindBuffer");
 return r;}
 g.BufferData=(void(*)(unsigned,ptrdiff_t,const void*,unsigned))get("glBufferData"); if(!g.BufferData){snprintf(r.detail,sizeof(r.detail),"Missing glBufferData");
 return r;}
 g.BindBufferBase=(void(*)(unsigned,unsigned,unsigned))get("glBindBufferBase"); if(!g.BindBufferBase){snprintf(r.detail,sizeof(r.detail),"Missing glBindBufferBase");
 return r;}
 g.DispatchCompute=(void(*)(unsigned,unsigned,unsigned))get("glDispatchCompute"); if(!g.DispatchCompute){snprintf(r.detail,sizeof(r.detail),"Missing glDispatchCompute");
 return r;}
 g.MemoryBarrier=(void(*)(unsigned))get("glMemoryBarrier"); if(!g.MemoryBarrier){snprintf(r.detail,sizeof(r.detail),"Missing glMemoryBarrier");
 return r;}
 g.MapBufferRange=(void*(*)(unsigned,ptrdiff_t,ptrdiff_t,unsigned))get("glMapBufferRange"); if(!g.MapBufferRange){snprintf(r.detail,sizeof(r.detail),"Missing glMapBufferRange");
 return r;}
 g.UnmapBuffer=(unsigned char(*)(unsigned))get("glUnmapBuffer"); if(!g.UnmapBuffer){snprintf(r.detail,sizeof(r.detail),"Missing glUnmapBuffer");
 return r;}
 g.DeleteBuffers=(void(*)(int,const unsigned*))get("glDeleteBuffers"); if(!g.DeleteBuffers){snprintf(r.detail,sizeof(r.detail),"Missing glDeleteBuffers");
 return r;}
 g.GenTextures=(void(*)(int,unsigned*))get("glGenTextures"); if(!g.GenTextures){snprintf(r.detail,sizeof(r.detail),"Missing glGenTextures");
 return r;}
 g.BindTexture=(void(*)(unsigned,unsigned))get("glBindTexture"); if(!g.BindTexture){snprintf(r.detail,sizeof(r.detail),"Missing glBindTexture");
 return r;}
 g.TexStorage2D=(void(*)(unsigned,int,unsigned,int,int))get("glTexStorage2D"); if(!g.TexStorage2D){snprintf(r.detail,sizeof(r.detail),"Missing glTexStorage2D");
 return r;}
 g.BindImageTexture=(void(*)(unsigned,unsigned,int,unsigned char,int,unsigned,unsigned))get("glBindImageTexture"); if(!g.BindImageTexture){snprintf(r.detail,sizeof(r.detail),"Missing glBindImageTexture");
 return r;}
 g.DeleteTextures=(void(*)(int,const unsigned*))get("glDeleteTextures"); if(!g.DeleteTextures){snprintf(r.detail,sizeof(r.detail),"Missing glDeleteTextures");
 return r;}
 g.GenFramebuffers=(void(*)(int,unsigned*))get("glGenFramebuffers"); if(!g.GenFramebuffers){snprintf(r.detail,sizeof(r.detail),"Missing glGenFramebuffers");
 return r;}
 g.BindFramebuffer=(void(*)(unsigned,unsigned))get("glBindFramebuffer"); if(!g.BindFramebuffer){snprintf(r.detail,sizeof(r.detail),"Missing glBindFramebuffer");
 return r;}
 g.FramebufferTexture2D=(void(*)(unsigned,unsigned,unsigned,unsigned,int))get("glFramebufferTexture2D"); if(!g.FramebufferTexture2D){snprintf(r.detail,sizeof(r.detail),"Missing glFramebufferTexture2D");
 return r;}
 g.CheckFramebufferStatus=(unsigned(*)(unsigned))get("glCheckFramebufferStatus"); if(!g.CheckFramebufferStatus){snprintf(r.detail,sizeof(r.detail),"Missing glCheckFramebufferStatus");
 return r;}
 g.ReadPixels=(void(*)(int,int,int,int,unsigned,unsigned,void*))get("glReadPixels"); if(!g.ReadPixels){snprintf(r.detail,sizeof(r.detail),"Missing glReadPixels");
 return r;}
 g.DeleteFramebuffers=(void(*)(int,const unsigned*))get("glDeleteFramebuffers"); if(!g.DeleteFramebuffers){snprintf(r.detail,sizeof(r.detail),"Missing glDeleteFramebuffers");
 return r;}
 g.GenVertexArrays=(void(*)(int,unsigned*))get("glGenVertexArrays"); if(!g.GenVertexArrays){snprintf(r.detail,sizeof(r.detail),"Missing glGenVertexArrays");
 return r;}
 g.BindVertexArray=(void(*)(unsigned))get("glBindVertexArray"); if(!g.BindVertexArray){snprintf(r.detail,sizeof(r.detail),"Missing glBindVertexArray");
 return r;}
 g.DeleteVertexArrays=(void(*)(int,const unsigned*))get("glDeleteVertexArrays"); if(!g.DeleteVertexArrays){snprintf(r.detail,sizeof(r.detail),"Missing glDeleteVertexArrays");
 return r;}
 g.Viewport=(void(*)(int,int,int,int))get("glViewport"); if(!g.Viewport){snprintf(r.detail,sizeof(r.detail),"Missing glViewport");
 return r;}
 g.DrawArrays=(void(*)(unsigned,int,int))get("glDrawArrays"); if(!g.DrawArrays){snprintf(r.detail,sizeof(r.detail),"Missing glDrawArrays");
 return r;}
 g.Finish=(void(*)(void))get("glFinish"); if(!g.Finish){snprintf(r.detail,sizeof(r.detail),"Missing glFinish");
 return r;}
 g.GetIntegerv(0x821B,&major);g.GetIntegerv(0x821C,&minor);
 if(major<4||(major==4&&minor<3)){snprintf(r.detail,sizeof(r.detail),"Requires real GL4.3; got %d.%d",major,minor);
 return r;}
 /* Reject version spoofing: entry-point presence isn't proof. Execute actual workloads. */
 stage="compute GLSL430 compile";
 const char*compute="#version 430\nlayout(local_size_x=4)in; layout(std430,binding=0)buffer B{uint v[];};layout(rgba8,binding=1)uniform writeonly image2D target;void main(){uint i=gl_GlobalInvocationID.x;v[i]=i*7u+3u;if(i==0u)imageStore(target,ivec2(0),vec4(1,0,1,1));}";
 cs=shader(&g,0x91B9,compute,error);
 if(!cs)goto cleanup;
 stage="compute link";cp=program(&g,cs,0,error);
 if(!cp)goto cleanup;
 g.GenBuffers(1,&buf);g.BindBuffer(0x90D2,buf);unsigned initial[4]={0};g.BufferData(0x90D2,sizeof(initial),initial,0x88E8);g.BindBufferBase(0x90D2,0,buf);
 g.GenTextures(1,&tex);g.BindTexture(0x0DE1,tex);g.TexStorage2D(0x0DE1,1,0x8058,1,1);g.BindImageTexture(1,tex,0,0,0,0x88B9,0x8058);
 g.UseProgram(cp);g.DispatchCompute(1,1,1);g.MemoryBarrier(0xFFFFFFFFu);g.Finish();
 stage="SSBO roundtrip";
 unsigned*out=g.MapBufferRange(0x90D2,0,sizeof(initial),1);
 if(!out){snprintf(error,sizeof(error),"map returned null");goto cleanup;}
 int values=1;for(unsigned i=0;i<4;i++)if(out[i]!=i*7+3)values=0;g.UnmapBuffer(0x90D2);
 if(!values){snprintf(error,sizeof(error),"wrong compute result");goto cleanup;}
 stage="image store/readback";g.GenFramebuffers(1,&fbo);g.BindFramebuffer(0x8D40,fbo);g.FramebufferTexture2D(0x8D40,0x8CE0,0x0DE1,tex,0);
 if(g.CheckFramebufferStatus(0x8D40)!=0x8CD5){snprintf(error,sizeof(error),"incomplete image framebuffer");goto cleanup;}
 g.ReadPixels(0,0,1,1,0x1908,0x1401,r.rgba);
 if(r.rgba[0]!=255||r.rgba[1]!=0||r.rgba[2]!=255||r.rgba[3]!=255){snprintf(error,sizeof(error),"wrong image-store pixel");goto cleanup;}
 stage="draw GLSL430 compile";
 vs=shader(&g,0x8B31,"#version 430\nvoid main(){vec2 p[3]=vec2[3](vec2(-1,-1),vec2(3,-1),vec2(-1,3));gl_Position=vec4(p[gl_VertexID],0,1);}",error);
 if(!vs)goto cleanup;
 fs=shader(&g,0x8B30,"#version 430\nlayout(location=0)out vec4 color;void main(){color=vec4(0,1,0,1);}",error);
 if(!fs)goto cleanup;
 stage="draw link";dp=program(&g,vs,fs,error);
 if(!dp)goto cleanup;
 stage="draw/readback";g.UseProgram(dp);g.GenVertexArrays(1,&vao);g.BindVertexArray(vao);g.Viewport(0,0,1,1);g.DrawArrays(4,0,3);g.Finish();g.ReadPixels(0,0,1,1,0x1908,0x1401,r.rgba);
 if(r.rgba[0]!=0||r.rgba[1]!=255||r.rgba[2]!=0||r.rgba[3]!=255){snprintf(error,sizeof(error),"wrong rendered pixel");goto cleanup;}
 unsigned e=g.GetError();
 if(e){snprintf(error,sizeof(error),"GL error 0x%x",e);goto cleanup;}
 r.passed=1;snprintf(r.detail,sizeof(r.detail),"PASS GL%d.%d compute + SSBO + image-store + triangle readback. Renderer: %s. This is not game or presentation validation.",major,minor,g.GetString(0x1F01));
cleanup:
 if(!r.passed)snprintf(r.detail,sizeof(r.detail),"FAIL %s: %s",stage,error);
 g.UseProgram(0);g.BindFramebuffer(0x8D40,0);g.BindVertexArray(0);
 if(dp)g.DeleteProgram(dp);
 if(cp)g.DeleteProgram(cp);
 if(cs)g.DeleteShader(cs);
 if(vs)g.DeleteShader(vs);
 if(fs)g.DeleteShader(fs);
 if(vao)g.DeleteVertexArrays(1,&vao);
 if(fbo)g.DeleteFramebuffers(1,&fbo);
 if(tex)g.DeleteTextures(1,&tex);
 if(buf)g.DeleteBuffers(1,&buf);
 return r;
}

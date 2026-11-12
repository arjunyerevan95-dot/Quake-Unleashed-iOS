/* Draw using the actual engine GUI program, generated texture and geometry only. */
#include "../probe/probe.h"
#define LOAD(ret,n,args) ret(*n)args=get("gl" #n);if(!n)return 0
int qvru_render_gui(qvru_get_proc get,unsigned program,unsigned char*out){
 LOAD(void,UseProgram,(unsigned));LOAD(void,GenVertexArrays,(int,unsigned*));LOAD(void,BindVertexArray,(unsigned));LOAD(void,DeleteVertexArrays,(int,const unsigned*));
 LOAD(void,GenBuffers,(int,unsigned*));LOAD(void,BindBuffer,(unsigned,unsigned));LOAD(void,BufferData,(unsigned,ptrdiff_t,const void*,unsigned));LOAD(void,DeleteBuffers,(int,const unsigned*));
 LOAD(void,EnableVertexAttribArray,(unsigned));LOAD(void,VertexAttribPointer,(unsigned,int,unsigned,unsigned char,int,const void*));
 LOAD(void,GenTextures,(int,unsigned*));LOAD(void,BindTexture,(unsigned,unsigned));LOAD(void,TexImage2D,(unsigned,int,int,int,int,int,unsigned,unsigned,const void*));LOAD(void,TexParameteri,(unsigned,unsigned,int));LOAD(void,DeleteTextures,(int,const unsigned*));LOAD(void,ActiveTexture,(unsigned));
 LOAD(void,Viewport,(int,int,int,int));LOAD(void,ClearColor,(float,float,float,float));LOAD(void,Clear,(unsigned));LOAD(void,DrawArrays,(unsigned,int,int));LOAD(void,ReadPixels,(int,int,int,int,unsigned,unsigned,void*));LOAD(unsigned,GetError,(void));LOAD(void,Uniform4f,(int,float,float,float,float));
 unsigned vao=0,vbo=0,texture=0;GenVertexArrays(1,&vao);BindVertexArray(vao);GenBuffers(1,&vbo);BindBuffer(0x8892,vbo);
 const float vertices[]={-1,-1, 0,0, 1,1,1,1, 3,-1, 1,0, 1,1,1,1, -1,3, 0,1, 1,1,1,1};
 BufferData(0x8892,sizeof(vertices),vertices,0x88E4);
 for(unsigned i=0;i<3;i++){EnableVertexAttribArray(i);VertexAttribPointer(i,i==2?4:2,0x1406,0,8*sizeof(float),(const void*)(size_t)(i*2*sizeof(float)));}
 const unsigned char texel[]={32,192,64,255};GenTextures(1,&texture);ActiveTexture(0x84C0);BindTexture(0x0DE1,texture);TexImage2D(0x0DE1,0,0x8058,1,1,0,0x1908,0x1401,texel);TexParameteri(0x0DE1,0x2801,0x2600);TexParameteri(0x0DE1,0x2800,0x2600);
 UseProgram(program);Uniform4f(0,0,0,0,0);Viewport(0,0,16,16);ClearColor(0,0,0,1);Clear(0x4000);DrawArrays(4,0,3);ReadPixels(8,8,1,1,0x1908,0x1401,out);unsigned error=GetError();
 UseProgram(0);BindVertexArray(0);DeleteVertexArrays(1,&vao);DeleteBuffers(1,&vbo);DeleteTextures(1,&texture);
 return !error&&out[0]>=30&&out[0]<=34&&out[1]>=190&&out[1]<=194&&out[2]>=62&&out[2]<=66;
}

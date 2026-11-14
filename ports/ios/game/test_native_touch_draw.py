"""Compile production native-overlay function with recording SDL test doubles."""
import pathlib, subprocess, tempfile
root=pathlib.Path(__file__).parent
s=(root/'sdl_zink_bridge.c').read_text()
block=s[s.index('static void QVR_DrawNativeTouch'):s.index('void QVR_GL_SwapWindow')]
source='#include <cassert>\nstruct SDL_Rect{int x,y,w,h;};\nint renderer=1,width=1042,height=480,ow=2868,oh=1320,lw=1042,lh=480,dw,dh; SDL_Rect v;\nint SDL_GetRendererOutputSize(int,int*w,int*h){*w=ow;*h=oh;return 0;}\nint SDL_RenderSetLogicalSize(int,int w,int h){lw=w;lh=h;return 0;}\nvoid SDL_RenderSetViewport(int,SDL_Rect*p){v=*p;}\nvoid SDL_RenderSetScale(int,int,int){}\nvoid QVR_TouchDraw(int,int w,int h){assert(lw==0&&lh==0);dw=w;dh=h;}\n'+block+'int main(){QVR_DrawNativeTouch();assert(dw>width&&dh>height);assert(v.x>=0&&v.y>=0&&v.x+v.w<=ow&&v.y+v.h<=oh);assert(lw==width&&lh==height);ow=1320;oh=2868;QVR_DrawNativeTouch();assert(v.y>0&&v.x>=0);assert(lw==width&&lh==height);return 0;}'
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d);(p/'test.cpp').write_text(source)
 subprocess.run(['c++','-std=c++17',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS native drawable overlay dimensions and restored logical touch coordinates')

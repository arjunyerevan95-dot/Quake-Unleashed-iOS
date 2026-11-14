"""Compile independently authored solid glyphs and check geometry at edit scale limits."""
from pathlib import Path
import subprocess,sys,tempfile
s=(Path(__file__).parent/'touch_overlay.c').read_text()
a=s.index('static void circle(');b=s.index('void QVR_TouchDraw(',a)
body=s[a:b]
source='''#include <math.h>
#include <assert.h>
#include <stddef.h>
typedef unsigned char Uint8;
typedef struct{float x,y;} SDL_FPoint;
typedef struct{Uint8 r,g,b,a;} SDL_Color;
typedef struct{SDL_FPoint position;SDL_Color color;SDL_FPoint tex_coord;} SDL_Vertex;
typedef struct{int unused;} SDL_Renderer;
enum { LOOK,MOVE,FIRE,JUMP,NEXT,PREV,MENU,BACK,CROUCH,GYRO,ROLE_COUNT };
static int triangles,lines;
static int SDL_GetRenderDrawColor(SDL_Renderer*r,Uint8*a,Uint8*b,Uint8*c,Uint8*d){*a=*b=*c=*d=255;return 0;}
static int SDL_RenderDrawLinesF(SDL_Renderer*r,const SDL_FPoint*p,int n){for(int i=0;i<n;i++)assert(isfinite(p[i].x)&&isfinite(p[i].y));lines++;return 0;}
static int SDL_RenderGeometry(SDL_Renderer*r,void*t,const SDL_Vertex*v,int n,const int*ix,int m){assert(n>=3&&n<=24&&m%3==0);for(int i=0;i<n;i++)assert(isfinite(v[i].position.x)&&isfinite(v[i].position.y));for(int i=0;i<m;i++)assert(ix[i]>=0&&ix[i]<n);triangles+=m/3;return 0;}
'''+body+'''
int main(void){SDL_Renderer r;float sizes[]={8,24,60};for(int j=0;j<3;j++){for(int role=FIRE;role<ROLE_COUNT;role++){triangles=lines=0;glyph(&r,role,120,120,sizes[j]);assert(triangles+lines>0);}for(int action=0;action<4;action++){triangles=lines=0;editorIcon(&r,action,120,120,sizes[j]);assert(triangles+lines>0);}}return 0;}
'''
if len(sys.argv)>1:Path(sys.argv[1]).write_text(source)
else:
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'test.c').write_text(source);subprocess.run(['cc','-std=c99',str(p/'test.c'),'-lm','-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('PASS production vector glyph geometry: every action/editor icon at 3 sizes')

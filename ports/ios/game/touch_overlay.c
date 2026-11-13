/* Touch controls feed an enumerated SDL Xbox controller, never keyboard bindings.
 * Relative mouse motion is used ONLY for swipe look. No retail resources. */
#include "touch_overlay.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define NF 12
static SDL_Joystick *pad;
static SDL_Window *window;
static int device = -1, menuMode;
static float aspect = 16.f/9.f;
static struct Finger { SDL_FingerID id; int used, role; float x,y,ox,oy,rx,ry; } fingers[NF];
enum { LOOK, MOVE, FIRE, JUMP, NEXT, PREV, MENU, BACK };
static const struct {float x,y; int role;} buttons[] = {
 { .64f,.78f,FIRE },{ .82f,.78f,JUMP },{ .92f,.59f,NEXT },
 { .81f,.59f,PREV },{ .93f,.12f,MENU },{ .75f,.12f,BACK }
};
static void axis(int a,float v) { if(pad) SDL_JoystickSetVirtualAxis(pad,a,(Sint16)(fmaxf(-1,fminf(1,v))*32767)); }
static void button(int b,int v) { if(pad) SDL_JoystickSetVirtualButton(pad,b,(Uint8)(v!=0)); }
void QVR_TouchReset(void) {
 memset(fingers,0,sizeof fingers);
 if(!pad)return;
 for(int i=0;i<SDL_CONTROLLER_AXIS_MAX;i++) axis(i,i>=SDL_CONTROLLER_AXIS_TRIGGERLEFT?-1.f:0.f);
 for(int i=0;i<SDL_CONTROLLER_BUTTON_MAX;i++)button(i,0);
 SDL_JoystickUpdate();
}
int QVR_TouchInit(SDL_Window *w) {
 if(pad)return 0;
 window=w;
 SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
 SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,"0");
 if(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER)!=0)return -1;
 SDL_VirtualJoystickDesc d; SDL_zero(d);
 d.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION; d.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;
 d.naxes=SDL_CONTROLLER_AXIS_MAX;d.nbuttons=SDL_CONTROLLER_BUTTON_MAX;
 d.vendor_id=0x045e;d.product_id=0x028e;d.name="UNLEASHED Touch Xbox 360 Controller";
 d.axis_mask=(1u<<SDL_CONTROLLER_AXIS_MAX)-1;d.button_mask=(1u<<SDL_CONTROLLER_BUTTON_MAX)-1;
 device=SDL_JoystickAttachVirtualEx(&d);
 if(device<0)return -1;
 char guid[64],mapping[1024]; SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(device),guid,sizeof guid);
 SDL_snprintf(mapping,sizeof mapping,"%s,UNLEASHED Touch Xbox 360 Controller,a:b0,b:b1,x:b2,y:b3,back:b4,guide:b5,start:b6,leftstick:b7,rightstick:b8,leftshoulder:b9,rightshoulder:b10,dpup:b11,dpdown:b12,dpleft:b13,dpright:b14,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5,type:Xbox360,",guid);
 if(SDL_GameControllerAddMapping(mapping)<0){SDL_JoystickDetachVirtual(device);device=-1;return -1;}
 pad=SDL_JoystickOpen(device); if(!pad){SDL_JoystickDetachVirtual(device);device=-1;return -1;}
 QVR_TouchReset();
 fprintf(stderr,"UNLEASHED touch: enumerated SDL Xbox controller index=%d id=%d recognized=%d\n",device,(int)SDL_JoystickInstanceID(pad),SDL_IsGameController(device));
 return 0;
}
static void state(void) {
 int held[8]={0};float mx=0,my=0;
 for(int i=0;i<NF;i++)if(fingers[i].used){
  struct Finger*f=&fingers[i];held[f->role]=1;
  if(f->role==MOVE){mx=(f->x-f->ox)*aspect/.15f;my=(f->y-f->oy)/.15f;float n=sqrtf(mx*mx+my*my);if(n>1){mx/=n;my/=n;}}
 }
 axis(SDL_CONTROLLER_AXIS_LEFTX,menuMode?0:mx);axis(SDL_CONTROLLER_AXIS_LEFTY,menuMode?0:my);
 axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,held[FIRE]&&!menuMode?1:-1);
 axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,held[JUMP]&&!menuMode?1:-1);
 button(SDL_CONTROLLER_BUTTON_A,menuMode&&(held[JUMP]||held[FIRE]));
 button(SDL_CONTROLLER_BUTTON_B,menuMode&&(held[BACK]||held[NEXT]));
 button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,!menuMode&&held[NEXT]);
 button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,!menuMode&&held[PREV]);
 button(SDL_CONTROLLER_BUTTON_START,held[MENU]);
 button(SDL_CONTROLLER_BUTTON_DPAD_LEFT,menuMode&&mx<-.5f);button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT,menuMode&&mx>.5f);
 button(SDL_CONTROLLER_BUTTON_DPAD_UP,menuMode&&my<-.5f);button(SDL_CONTROLLER_BUTTON_DPAD_DOWN,menuMode&&my>.5f);
 SDL_JoystickUpdate();
}
static void look(struct Finger*f,float dx,float dy) {
 if(menuMode)return;
 /* Width-normalized horizontal units keep sensitivity independent of resolution. */
 f->rx+=dx*aspect*480;f->ry+=dy*480;
 int x=(int)f->rx,y=(int)f->ry;f->rx-=x;f->ry-=y;
 if(x||y){SDL_Event e;SDL_zero(e);e.type=SDL_MOUSEMOTION;e.motion.windowID=SDL_GetWindowID(window);e.motion.which=SDL_TOUCH_MOUSEID;e.motion.xrel=x;e.motion.yrel=y;SDL_PushEvent(&e);}
}
int QVR_TouchEvent(const SDL_Event *e,int menu) {
 if(menuMode!=menu){QVR_TouchReset();menuMode=menu;}
 if(e->type==SDL_APP_WILLENTERBACKGROUND || (e->type==SDL_WINDOWEVENT&&e->window.event==SDL_WINDOWEVENT_FOCUS_LOST)){QVR_TouchReset();return 0;}
 if(e->type!=SDL_FINGERDOWN&&e->type!=SDL_FINGERMOTION&&e->type!=SDL_FINGERUP)return 0;
 int i;for(i=0;i<NF;i++)if(fingers[i].used&&fingers[i].id==e->tfinger.fingerId)break;
 if(e->type==SDL_FINGERDOWN){
  if(i==NF)for(i=0;i<NF;i++)if(!fingers[i].used)break;
  if(i==NF)return 1;
  struct Finger*f=&fingers[i];memset(f,0,sizeof *f);f->used=1;f->id=e->tfinger.fingerId;
  f->x=f->ox=e->tfinger.x;f->y=f->oy=e->tfinger.y;f->role=LOOK;
  for(unsigned b=0;b<sizeof buttons/sizeof buttons[0];b++){
   float dx=(f->x-buttons[b].x)*aspect,dy=f->y-buttons[b].y;
   if(dx*dx+dy*dy<.075f*.075f){f->role=buttons[b].role;break;}
  }
  if(f->role==LOOK&&f->x<.45f){int moving=0;for(int j=0;j<NF;j++)if(j!=i&&fingers[j].used&&fingers[j].role==MOVE)moving=1;if(!moving)f->role=MOVE;}
 }else if(i<NF){
  struct Finger*f=&fingers[i];
  if(e->type==SDL_FINGERUP)f->used=0;
  else {if(f->role==LOOK||f->role==FIRE)look(f,e->tfinger.x-f->x,e->tfinger.y-f->y);f->x=e->tfinger.x;f->y=e->tfinger.y;}
 }
 state();return 1;
}
static void circle(SDL_Renderer*r,float x,float y,float radius) {
 SDL_FPoint p[49];for(int i=0;i<=48;i++){float a=(float)i*6.2831853f/48;p[i].x=x+cosf(a)*radius;p[i].y=y+sinf(a)*radius;}SDL_RenderDrawLinesF(r,p,49);
}
static void line(SDL_Renderer*r,float x,float y,float a,float b,float c,float d,float s){SDL_RenderDrawLineF(r,x+a*s,y+b*s,x+c*s,y+d*s);}
void QVR_TouchDraw(SDL_Renderer*r,int w,int h) {
 aspect=(float)w/h;
 SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(r,240,245,250,190);
 for(unsigned i=0;i<sizeof buttons/sizeof buttons[0];i++){
  float x=buttons[i].x*w,y=buttons[i].y*h,s=h*.030f;int role=buttons[i].role;circle(r,x,y,h*.067f);
  if(role==FIRE){circle(r,x,y,s*.65f);line(r,x,y,-1.25,0,1.25,0,s);line(r,x,y,0,-1.25,0,1.25,s);}
  else if(role==JUMP){line(r,x,y,-1,0,0,-1,s);line(r,x,y,0,-1,1,0,s);line(r,x,y,0,-1,0,1,s);}
  else if(role==NEXT||role==PREV||role==BACK){float v=role==NEXT?1:-1;line(r,x,y,-v,0,v,0,s);line(r,x,y,v,0,0,-1,s);line(r,x,y,v,0,0,1,s);}
  else if(role==MENU)for(int k=-1;k<=1;k++)line(r,x,y,-1,k*.7f,1,k*.7f,s);
 }
 for(int i=0;i<NF;i++)if(fingers[i].used&&fingers[i].role==MOVE){struct Finger*f=&fingers[i];circle(r,f->ox*w,f->oy*h,h*.15f);circle(r,f->x*w,f->y*h,h*.05f);}
}

/* Touch controls feed an enumerated SDL Xbox controller, never keyboard bindings.
 * Relative mouse motion is used ONLY for swipe look. No retail resources. */
#include "touch_overlay.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "touch_layout.h"
#define NF 12
static SDL_Joystick *pad;
static SDL_Window *window;
static int device = -1, menuMode;
static float aspect = 16.f/9.f;
static struct Finger { SDL_FingerID id; int used, role; float x,y,ox,oy,rx,ry; } fingers[NF];
enum { LOOK, MOVE, FIRE, JUMP, NEXT, PREV, MENU, BACK, CROUCH, GYRO, ROLE_COUNT };
#define DEFAULT_LAYOUT { {.78f,.78f,1.0f,FIRE},{.89f,.67f,1.0f,JUMP}, \
 {.95f,.49f,1.0f,NEXT},{.84f,.49f,1.0f,PREV},{.95f,.12f,1.0f,MENU}, \
 {.72f,.12f,1.0f,BACK},{.94f,.83f,1.0f,CROUCH},{.87f,.12f,1.0f,GYRO}, \
 {.15f,.76f,1.0f,MOVE} }
static QVR_TouchItem buttons[]=DEFAULT_LAYOUT;
static const QVR_TouchItem defaults[]=DEFAULT_LAYOUT;
#define NB ((int)(sizeof buttons/sizeof buttons[0]))
static QVR_TouchItem editSaved[NB];
static int editing,selected=-1,editDrag=-1,editResize,saveFailed;
static SDL_FingerID editFinger;
static SDL_FingerID editSecond;
static int editPinch;
static float editX,editY,editX2,editY2,editSpan,editScale;
static char layoutPath[1024]="touch-layout-v1.cfg";
static SDL_Sensor *gyroSensor;
static int gyroEnabled,gyroBackground,gyroIndex=-1;
static Uint64 gyroTimestamp;
static Uint32 gyroFreshTick;
static void gyroBaseline(void){float v[3];gyroFreshTick=SDL_GetTicks()-100;if(gyroSensor){SDL_SensorUpdate();SDL_SensorGetDataWithTimestamp(gyroSensor,&gyroTimestamp,v,3);}}
int QVR_TouchGyro(float out[3]){
 if(menuMode||editing||gyroBackground||!gyroEnabled||!gyroSensor)return 0;
 SDL_SensorUpdate();Uint64 stamp=0;
 if(SDL_SensorGetDataWithTimestamp(gyroSensor,&stamp,out,3)!=0||!stamp)return 0;
 Uint32 now=SDL_GetTicks();
 if(stamp!=gyroTimestamp){gyroTimestamp=stamp;gyroFreshTick=now;}
 return (Uint32)(now-gyroFreshTick)<100&&isfinite(out[0])&&isfinite(out[1])&&isfinite(out[2]);
}
int QVR_TouchGyroAvailable(void){return gyroIndex>=0;}
static void gyroStreaming(void){
 if(gyroEnabled&&!gyroBackground&&gyroIndex>=0){if(!gyroSensor)gyroSensor=SDL_SensorOpen(gyroIndex);if(!gyroSensor)gyroEnabled=0;else gyroBaseline();}
 else if(gyroSensor){SDL_SensorClose(gyroSensor);gyroSensor=NULL;}
}
static float itemRadius(int i){return (buttons[i].role==MOVE?.15f:buttons[i].role==FIRE?.09f:.065f)*buttons[i].scale;}
int QVR_TouchEditing(void){return editing;}
static void beginEdit(void){QVR_TouchReset();memcpy(editSaved,buttons,sizeof buttons);editing=1;selected=editDrag=-1;saveFailed=0;}
static void cancelEdit(void){memcpy(buttons,editSaved,sizeof buttons);editing=0;editDrag=-1;QVR_TouchReset();}
static void clampItem(int i){float r=itemRadius(i);buttons[i].x=fmaxf(r/aspect,fminf(1-r/aspect,buttons[i].x));buttons[i].y=fmaxf(r,fminf(1-r,buttons[i].y));}
static int hit(float x,float y,float cx,float cy,float r){float dx=(x-cx)*aspect,dy=y-cy;return dx*dx+dy*dy<r*r;}
static void editEvent(const SDL_Event*e){
 float x=e->tfinger.x,y=e->tfinger.y;
 if(!isfinite(x)||!isfinite(y))return;
 if(e->type==SDL_FINGERDOWN){
  if(editDrag>=0){
   if(!editPinch&&e->tfinger.fingerId!=editFinger){
    editSecond=e->tfinger.fingerId;editX2=x;editY2=y;
    editSpan=hypotf((editX2-editX)*aspect,editY2-editY);
    editScale=buttons[editDrag].scale;editPinch=1;
   }
   return;
  }
  editX=x;editY=y;
  /* Persistent toolbar: cancel, reset, save. */
  if(hit(x,y,.42f,.1f,.065f)){cancelEdit();return;}
  if(hit(x,y,.50f,.1f,.065f)){memcpy(buttons,defaults,sizeof buttons);selected=-1;return;}
  if(hit(x,y,.58f,.1f,.065f)){
   if(QVR_LayoutSave(layoutPath,buttons,NB)==0){editing=0;QVR_TouchReset();}else saveFailed=1;return;
  }
  if(selected>=0){float r=itemRadius(selected);if(hit(x,y,buttons[selected].x+r/aspect,buttons[selected].y+r,.045f)){editDrag=selected;editResize=1;editFinger=e->tfinger.fingerId;return;}}
  selected=-1;
  for(int i=0;i<NB;i++)if(hit(x,y,buttons[i].x,buttons[i].y,itemRadius(i))){selected=editDrag=i;editResize=0;editFinger=e->tfinger.fingerId;break;}
 }else if(editDrag>=0&&editPinch){
  if(e->tfinger.fingerId!=editFinger&&e->tfinger.fingerId!=editSecond)return;
  if(e->type==SDL_FINGERUP){
   /* Drain the remaining owned contact; never jump back into drag or gameplay. */
   if(editPinch==2){editPinch=0;editDrag=-1;return;}
   if(e->tfinger.fingerId==editFinger)editFinger=editSecond;
   editSecond=editFinger;editPinch=2;return;
  }
  if(editPinch==2)return;
  if(e->tfinger.fingerId==editFinger){editX=x;editY=y;}else{editX2=x;editY2=y;}
  float span=hypotf((editX2-editX)*aspect,editY2-editY);
  if(editSpan<.01f){if(span>=.01f){editSpan=span;editScale=buttons[editDrag].scale;}return;}
  buttons[editDrag].scale=fmaxf(.6f,fminf(1.8f,editScale*span/editSpan));clampItem(editDrag);
 }else if(editDrag>=0&&e->tfinger.fingerId==editFinger){
  if(e->type==SDL_FINGERUP){editDrag=-1;return;}
  editX=x;editY=y;
  QVR_TouchItem*b=&buttons[editDrag];
  if(editResize){float dx=(x-b->x)*aspect,dy=y-b->y;float base=b->role==MOVE?.15f:b->role==FIRE?.09f:.065f;b->scale=fmaxf(.6f,fminf(1.8f,sqrtf(dx*dx+dy*dy)/(base*1.41421356f)));}
  else{b->x=x;b->y=y;}clampItem(editDrag);
 }
}
static void axis(int a,float v) { if(pad) SDL_JoystickSetVirtualAxis(pad,a,(Sint16)(fmaxf(-1,fminf(1,v))*32767)); }
static void button(int b,int v) { if(pad) SDL_JoystickSetVirtualButton(pad,b,(Uint8)(v!=0)); }
void QVR_TouchReset(void) {
 editDrag=-1;editPinch=0;
 memset(fingers,0,sizeof fingers);
 if(!pad)return;
 for(int i=0;i<SDL_CONTROLLER_AXIS_MAX;i++) axis(i,i>=SDL_CONTROLLER_AXIS_TRIGGERLEFT?-1.f:0.f);
 for(int i=0;i<SDL_CONTROLLER_BUTTON_MAX;i++)button(i,0);
 SDL_JoystickUpdate();
}
int QVR_TouchInit(SDL_Window *w) {
 if(pad)return 0;
 window=w;
 QVR_LayoutLoad(layoutPath,buttons,NB);
 if(SDL_InitSubSystem(SDL_INIT_SENSOR)==0)for(int i=0;i<SDL_NumSensors();i++)if(SDL_SensorGetDeviceType(i)==SDL_SENSOR_GYRO){gyroIndex=i;break;}
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
/* Device indices can move when another joystick is removed. Resolve our owned
 * instance each time rather than assuming index zero or retaining a stale index. */
int QVR_TouchDeviceIndex(void) {
 if(!pad || !SDL_JoystickGetAttached(pad))return -1;
 SDL_JoystickID id=SDL_JoystickInstanceID(pad);
 for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_JoystickGetDeviceInstanceID(i)==id)return i;
 return -1;
}
static void state(void) {
 int held[ROLE_COUNT]={0};float mx=0,my=0;
 for(int i=0;i<NF;i++)if(fingers[i].used){
  struct Finger*f=&fingers[i];held[f->role]=1;
  if(f->role==MOVE){float radius=.15f;for(int b=0;b<NB;b++)if(buttons[b].role==MOVE)radius=itemRadius(b);mx=(f->x-f->ox)*aspect/radius;my=(f->y-f->oy)/radius;float n=sqrtf(mx*mx+my*my);if(n>1){mx/=n;my/=n;}}
 }
 axis(SDL_CONTROLLER_AXIS_LEFTX,menuMode?0:mx);axis(SDL_CONTROLLER_AXIS_LEFTY,menuMode?0:my);
 axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,held[FIRE]&&!menuMode?1:-1);
 axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,-1);
 button(SDL_CONTROLLER_BUTTON_A,held[JUMP]||(menuMode&&held[FIRE]));
 button(SDL_CONTROLLER_BUTTON_B,held[NEXT]||(menuMode&&held[BACK]));
 button(SDL_CONTROLLER_BUTTON_Y,!menuMode&&held[PREV]);
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,!menuMode&&held[CROUCH]);
 button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,0);
 button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,0);
 button(SDL_CONTROLLER_BUTTON_START,held[MENU]);
 button(SDL_CONTROLLER_BUTTON_DPAD_LEFT,menuMode&&mx<-.5f);button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT,menuMode&&mx>.5f);
 button(SDL_CONTROLLER_BUTTON_DPAD_UP,menuMode&&my<-.5f);button(SDL_CONTROLLER_BUTTON_DPAD_DOWN,menuMode&&my>.5f);
 SDL_JoystickUpdate();
}
static void look(struct Finger*f,float dx,float dy) {
 if(menuMode)return;
 /* Width-normalized horizontal units keep sensitivity independent of resolution. */
 f->rx+=dx*aspect*1200;f->ry+=dy*1200;
 int x=(int)f->rx,y=(int)f->ry;f->rx-=x;f->ry-=y;
 if(x||y){SDL_Event e;SDL_zero(e);e.type=SDL_MOUSEMOTION;e.motion.windowID=SDL_GetWindowID(window);e.motion.which=SDL_TOUCH_MOUSEID;e.motion.xrel=x;e.motion.yrel=y;SDL_PushEvent(&e);}
}
int QVR_TouchEvent(const SDL_Event *e,int menu) {
 if(menuMode!=menu){QVR_TouchReset();menuMode=menu;}
 if(e->type==SDL_APP_DIDENTERFOREGROUND||(e->type==SDL_WINDOWEVENT&&e->window.event==SDL_WINDOWEVENT_FOCUS_GAINED)){gyroBackground=0;gyroStreaming();}
 if(e->type==SDL_APP_WILLENTERBACKGROUND || (e->type==SDL_WINDOWEVENT&&e->window.event==SDL_WINDOWEVENT_FOCUS_LOST)){gyroBackground=1;gyroStreaming();if(editing)cancelEdit();QVR_TouchReset();return 0;}
 if(e->type!=SDL_FINGERDOWN&&e->type!=SDL_FINGERMOTION&&e->type!=SDL_FINGERUP)return 0;
 if(editing){editEvent(e);return 1;}
 if(menuMode&&e->type==SDL_FINGERDOWN&&hit(e->tfinger.x,e->tfinger.y,.50f,.1f,.065f)){beginEdit();return 1;}
 int i;for(i=0;i<NF;i++)if(fingers[i].used&&fingers[i].id==e->tfinger.fingerId)break;
 if(e->type==SDL_FINGERDOWN){
  if(i==NF)for(i=0;i<NF;i++)if(!fingers[i].used)break;
  if(i==NF)return 1;
  struct Finger*f=&fingers[i];memset(f,0,sizeof *f);f->used=1;f->id=e->tfinger.fingerId;
  f->x=f->ox=e->tfinger.x;f->y=f->oy=e->tfinger.y;f->role=LOOK;
  for(unsigned b=0;b<sizeof buttons/sizeof buttons[0];b++){
   if(buttons[b].role==BACK&&!menuMode)continue;
   if(hit(f->x,f->y,buttons[b].x,buttons[b].y,itemRadius(b))){f->role=buttons[b].role;break;}
  }
  if(f->role==MOVE||(f->role==LOOK&&f->x<.45f)){int moving=0;for(int j=0;j<NF;j++)if(j!=i&&fingers[j].used&&fingers[j].role==MOVE)moving=1;f->role=moving?LOOK:MOVE;}
  if(f->role==GYRO&&gyroIndex>=0){gyroEnabled=!gyroEnabled;gyroStreaming();}
 }else if(i<NF){
  struct Finger*f=&fingers[i];
  if(e->type==SDL_FINGERUP)f->used=0;
  else {if(f->role==LOOK||f->role==FIRE)look(f,e->tfinger.x-f->x,e->tfinger.y-f->y);f->x=e->tfinger.x;f->y=e->tfinger.y;}
 }
 state();
 /* Bounded device evidence: distinguish UIKit delivery from controller sampling. */
 static unsigned traced;
 if(traced<12 && e->type!=SDL_FINGERMOTION){
  ++traced;
  fprintf(stderr,"UNLEASHED touch event=%u role=%d menu=%d pad=%d focus=%d rawRT=%d rawA=%d\n",
   (unsigned)e->type,i<NF?fingers[i].role:-1,menuMode,pad!=NULL,
   SDL_GetKeyboardFocus()==window,pad?SDL_JoystickGetAxis(pad,SDL_CONTROLLER_AXIS_TRIGGERRIGHT):0,
   pad?SDL_JoystickGetButton(pad,SDL_CONTROLLER_BUTTON_A):0);
 }
 return 1;
}
static void circle(SDL_Renderer*r,float x,float y,float radius) {
 SDL_FPoint p[49];for(int i=0;i<=48;i++){float a=(float)i*6.2831853f/48;p[i].x=x+cosf(a)*radius;p[i].y=y+sinf(a)*radius;}SDL_RenderDrawLinesF(r,p,49);
}
static void line(SDL_Renderer*r,float x,float y,float a,float b,float c,float d,float s){SDL_RenderDrawLineF(r,x+a*s,y+b*s,x+c*s,y+d*s);}
/* Solid geometric glyphs, not a ring of indistinguishable hairline buttons. */
static void bar(SDL_Renderer*r,float x,float y,float a,float b,float w,float h,float s){SDL_FRect q={x+a*s,y+b*s,w*s,h*s};SDL_RenderFillRectF(r,&q);}
static void stroke(SDL_Renderer*r,float x,float y,float a,float b,float c,float d,float s){
 float dx=c-a,dy=d-b,n=sqrtf(dx*dx+dy*dy);if(n==0)return;
 for(float k=-.09f;k<=.09f;k+=.03f)line(r,x,y,a-dy/n*k,b+dx/n*k,c-dy/n*k,d+dx/n*k,s);
}
static void glyph(SDL_Renderer*r,int role,float x,float y,float s){
 if(role==FIRE){ /* slide, barrel, grip and trigger guard of a sidearm */
  bar(r,x,y,-1.05f,-.65f,1.9f,.42f,s);bar(r,x,y,.7f,-.53f,.4f,.2f,s);
  bar(r,x,y,-.87f,-.25f,.48f,.95f,s);bar(r,x,y,-.39f,-.23f,.8f,.18f,s);
  stroke(r,x,y,.36f,-.1f,.23f,.38f,s);stroke(r,x,y,.23f,.38f,-.42f,.38f,s);
  bar(r,x,y,-.8f,-.83f,.16f,.2f,s);
 }else if(role==JUMP){
  stroke(r,x,y,-.7f,-.05f,0,-.8f,s);stroke(r,x,y,0,-.8f,.7f,-.05f,s);
  bar(r,x,y,-.1f,-.7f,.2f,1.25f,s);bar(r,x,y,-.7f,.75f,1.4f,.18f,s);
 }else if(role==NEXT||role==PREV){
  /* Three cartridges plus a clear cycling chevron. */
  for(int i=0;i<3;i++){float a=-.8f+i*.4f;bar(r,x,y,a,-.55f,.23f,.7f,s);stroke(r,x,y,a,-.55f,a+.115f,-.75f,s);stroke(r,x,y,a+.115f,-.75f,a+.23f,-.55f,s);}
  float v=role==NEXT?1:-1;stroke(r,x,y,-.7f*v,.55f,.7f*v,.55f,s);stroke(r,x,y,.7f*v,.55f,.35f*v,.25f,s);stroke(r,x,y,.7f*v,.55f,.35f*v,.85f,s);
 }else if(role==CROUCH){circle(r,x+s*.22f,y-s*.65f,s*.18f);stroke(r,x,y,.1f,-.4f,-.35f,.1f,s);stroke(r,x,y,-.35f,.1f,.35f,.2f,s);stroke(r,x,y,.35f,.2f,-.15f,.8f,s);stroke(r,x,y,-.15f,.8f,.7f,.8f,s);stroke(r,x,y,-.08f,-.13f,.7f,-.03f,s);
 }else if(role==GYRO){circle(r,x,y,s*.7f);stroke(r,x,y,-1,0,1,0,s);stroke(r,x,y,0,-1,0,1,s);circle(r,x,y,s*.22f);
 }else if(role==BACK){stroke(r,x,y,.8f,0,-.8f,0,s);stroke(r,x,y,-.8f,0,-.15f,-.65f,s);stroke(r,x,y,-.8f,0,-.15f,.65f,s);
 }else if(role==MENU){for(int k=-1;k<=1;k++)bar(r,x,y,-.8f,k*.55f-.09f,1.6f,.18f,s);}
}
static void editorIcon(SDL_Renderer*r,int action,float x,float y,float s){
 if(action==0){stroke(r,x,y,-.6f,-.6f,.6f,.6f,s);stroke(r,x,y,-.6f,.6f,.6f,-.6f,s);}
 if(action==1){circle(r,x,y,s*.65f);stroke(r,x,y,-.65f,-.05f,-.9f,-.55f,s);stroke(r,x,y,-.9f,-.55f,-.25f,-.4f,s);}
 if(action==2){stroke(r,x,y,-.75f,0,-.2f,.6f,s);stroke(r,x,y,-.2f,.6f,.8f,-.6f,s);}
 if(action==3){circle(r,x,y,s*.55f);for(int k=0;k<8;k++){float a=k*.785398f;stroke(r,x,y,cosf(a)*.55f,sinf(a)*.55f,cosf(a)*.9f,sinf(a)*.9f,s);}}
}
void QVR_TouchDraw(SDL_Renderer*r,int w,int h) {
 aspect=(float)w/h;SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
 for(int i=0;i<NB;i++){
  int role=buttons[i].role;if(role==MOVE)continue;if(role==BACK&&!menuMode&&!editing)continue;
  float x=buttons[i].x*w,y=buttons[i].y*h,s=h*(role==FIRE?.049f:.033f)*buttons[i].scale;int held=0;
  for(int j=0;j<NF;j++)if(fingers[j].used&&fingers[j].role==role)held=1;
  if(role==GYRO)held=gyroEnabled;
  SDL_SetRenderDrawColor(r,8,12,16,180);glyph(r,role,x+h*.003f,y+h*.004f,s*1.06f);
  SDL_SetRenderDrawColor(r,held?255:238,held?198:243,held?100:248,role==GYRO&&gyroIndex<0?65:held?255:215);glyph(r,role,x,y,s);
 }
 /* Inventory silhouette is decorative; previous/next remain distinct buttons. */
 float px=.84f,py=.49f,nx=.95f,ny=.49f;for(int i=0;i<NB;i++){if(buttons[i].role==PREV){px=buttons[i].x;py=buttons[i].y;}if(buttons[i].role==NEXT){nx=buttons[i].x;ny=buttons[i].y;}}
 SDL_SetRenderDrawColor(r,225,230,235,135);glyph(r,FIRE,(px+nx)*.5f*w,(py+ny)*.5f*h,h*.028f);
 float ox=.15f*w,oy=.76f*h,radius=.15f;
 for(int b=0;b<NB;b++)if(buttons[b].role==MOVE){ox=buttons[b].x*w;oy=buttons[b].y*h;radius=itemRadius(b);}
 float tx=ox,ty=oy;int moving=0;
 for(int i=0;i<NF;i++)if(fingers[i].used&&fingers[i].role==MOVE){struct Finger*f=&fingers[i];ox=f->ox*w;oy=f->oy*h;tx=f->x*w;ty=f->y*h;moving=1;break;}
 SDL_SetRenderDrawColor(r,12,18,25,moving?115:55);circle(r,ox,oy,h*radius*.72f);
 SDL_SetRenderDrawColor(r,235,242,249,moving?150:70);circle(r,ox,oy,h*radius*.70f);circle(r,tx,ty,h*radius*.27f);
 if(editing){
  for(int i=0;i<NB;i++){SDL_SetRenderDrawColor(r,selected==i?255:120,selected==i?196:170,selected==i?80:210,160);circle(r,buttons[i].x*w,buttons[i].y*h,itemRadius(i)*h);}
  if(selected>=0){float rad=itemRadius(selected);SDL_FRect handle={(buttons[selected].x+rad/aspect)*w-h*.018f,(buttons[selected].y+rad)*h-h*.018f,h*.036f,h*.036f};SDL_RenderFillRectF(r,&handle);}
  for(int k=0;k<3;k++){SDL_SetRenderDrawColor(r,saveFailed&&k==2?255:245,saveFailed&&k==2?80:245,saveFailed&&k==2?80:245,255);editorIcon(r,k,(.42f+.08f*k)*w,.1f*h,h*.033f);}
 }else if(menuMode){SDL_SetRenderDrawColor(r,240,245,250,230);editorIcon(r,3,.5f*w,.1f*h,h*.033f);}
}

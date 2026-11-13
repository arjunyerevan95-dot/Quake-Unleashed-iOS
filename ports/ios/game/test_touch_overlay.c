/* Focused real SDL controller, simultaneous input and layout-editor fixture. */
#define SDL_MAIN_HANDLED
#include "touch_overlay.c"
#include <assert.h>
static void finger(Uint32 type,Sint64 id,float x,float y,int menu){SDL_Event e;SDL_zero(e);e.type=type;e.tfinger.fingerId=id;e.tfinger.x=x;e.tfinger.y=y;assert(QVR_TouchEvent(&e,menu));}
static int slot(int role){for(int i=0;i<NB;i++)if(buttons[i].role==role)return i;assert(0);return -1;}
static void press(Uint32 type,Sint64 id,int role,int menu){int i=slot(role);finger(type,id,buttons[i].x,buttons[i].y,menu);}
static int near(float a,float b){return fabsf(a-b)<.0001f;}
int main(void){
 SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");SDL_SetMainReady();assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER)==0);
 const char*tmp=SDL_getenv("TMPDIR");if(!tmp)tmp=SDL_getenv("TEMP");if(!tmp)tmp=".";
 SDL_snprintf(layoutPath,sizeof layoutPath,"%s/qvr-touch-fixture-%llu.cfg",tmp,(unsigned long long)SDL_GetPerformanceCounter());
 SDL_Window*w=SDL_CreateWindow("test",0,0,960,480,SDL_WINDOW_HIDDEN);assert(w);
 int base=SDL_NumJoysticks(),other=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_UNKNOWN,2,2,0);assert(other==base);assert(!SDL_IsGameController(other));
 assert(QVR_TouchInit(w)==0);assert(QVR_TouchDeviceIndex()==base+1);assert(SDL_JoystickDetachVirtual(other)==0);assert(QVR_TouchDeviceIndex()==base);device=QVR_TouchDeviceIndex();
 SDL_GameController*c=SDL_GameControllerOpen(device);assert(c);assert(SDL_GameControllerGetType(c)==SDL_CONTROLLER_TYPE_XBOX360);
 press(SDL_FINGERDOWN,1,FIRE,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);
 finger(SDL_FINGERDOWN,2,.15f,.60f,0);finger(SDL_FINGERMOTION,2,.22f,.55f,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_LEFTX)>10000);
 finger(SDL_FINGERDOWN,3,.65f,.40f,0);finger(SDL_FINGERMOTION,3,.70f,.42f,0);
 int saw=0;SDL_Event e;while(SDL_PollEvent(&e))if(e.type==SDL_MOUSEMOTION&&e.motion.xrel>0)saw=1;assert(saw);
 assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);
 finger(SDL_FINGERUP,3,.70f,.42f,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);
 press(SDL_FINGERUP,1,FIRE,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)==0);
 SDL_zero(e);e.type=SDL_APP_WILLENTERBACKGROUND;QVR_TouchEvent(&e,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_LEFTX)==0);
 press(SDL_FINGERDOWN,4,JUMP,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));press(SDL_FINGERUP,4,JUMP,0);assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));
 press(SDL_FINGERDOWN,5,NEXT,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_B));
 press(SDL_FINGERDOWN,6,PREV,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_Y));
 press(SDL_FINGERDOWN,7,CROUCH,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_LEFTSTICK));
 QVR_TouchReset();assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_B));assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_Y));assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_LEFTSTICK));
 finger(SDL_FINGERDOWN,8,.15f,.6f,1);finger(SDL_FINGERMOTION,8,.15f,.4f,1);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_DPAD_UP));
 press(SDL_FINGERDOWN,9,JUMP,1);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));QVR_TouchReset();
 /* Gear enters editor; drag and resize the fire control, never firing. */
 finger(SDL_FINGERDOWN,10,.5f,.1f,1);assert(QVR_TouchEditing());
 int fire=slot(FIRE);press(SDL_FINGERDOWN,11,FIRE,1);finger(SDL_FINGERMOTION,11,.70f,.72f,1);finger(SDL_FINGERUP,11,.70f,.72f,1);
 assert(near(buttons[fire].x,.70f)&&near(buttons[fire].y,.72f));assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)==0);assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));
 float rad=itemRadius(fire);finger(SDL_FINGERDOWN,12,buttons[fire].x+rad/aspect,buttons[fire].y+rad,1);assert(editResize);
 finger(SDL_FINGERMOTION,12,buttons[fire].x+rad*1.3f/aspect,buttons[fire].y+rad*1.3f,1);finger(SDL_FINGERUP,12,.8f,.8f,1);assert(near(buttons[fire].scale,1.3f));
 finger(SDL_FINGERDOWN,13,.58f,.1f,1);assert(!editing&&!saveFailed);
 memcpy(buttons,defaults,sizeof buttons);assert(QVR_LayoutLoad(layoutPath,buttons,NB)==1);assert(near(buttons[fire].x,.70f)&&near(buttons[fire].scale,1.3f));
 /* Explicit subsequent save preserves the existing user's layout as a backup. */
 assert(QVR_LayoutSave(layoutPath,buttons,NB)==0);char backup[1100];snprintf(backup,sizeof backup,"%s.bak.0",layoutPath);
 QVR_TouchItem previous[NB];memcpy(previous,defaults,sizeof previous);assert(QVR_LayoutLoad(backup,previous,NB)==1);assert(near(previous[fire].x,.70f));remove(backup);
 /* Cancel restores a user's saved layout. */
 beginEdit();press(SDL_FINGERDOWN,14,FIRE,1);finger(SDL_FINGERMOTION,14,.62f,.6f,1);finger(SDL_FINGERUP,14,.62f,.6f,1);finger(SDL_FINGERDOWN,15,.42f,.1f,1);assert(!editing&&near(buttons[fire].x,.70f));
 /* Invalid/newer files do not replace an in-memory customized layout. */
 char invalid[1100];snprintf(invalid,sizeof invalid,"%s.invalid",layoutPath);FILE*f=fopen(invalid,"w");assert(f);fputs("QVR_TOUCH_LAYOUT 99 1\n2 0.1 0.1 1\n",f);fclose(f);assert(QVR_LayoutLoad(invalid,buttons,NB)==-1);assert(near(buttons[fire].x,.70f));remove(invalid);
 /* Edited position still reaches native fire while swiping. */
 press(SDL_FINGERDOWN,16,FIRE,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);press(SDL_FINGERUP,16,FIRE,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)==0);
 SDL_Surface*s=SDL_CreateRGBSurfaceWithFormat(0,1536,707,32,SDL_PIXELFORMAT_RGBA32);assert(s);SDL_Renderer*r=SDL_CreateSoftwareRenderer(s);assert(r);
 memcpy(buttons,defaults,sizeof buttons);QVR_TouchDraw(r,1536,707);const char*preview=SDL_getenv("QVR_TOUCH_PREVIEW");if(preview){SDL_RenderPresent(r);assert(SDL_SaveBMP(s,preview)==0);}
 remove(layoutPath);SDL_DestroyRenderer(r);SDL_FreeSurface(s);SDL_GameControllerClose(c);SDL_Quit();puts("PASS real SDL routes, multitouch, crouch, menu, edit drag/resize/save/reload/cancel and glyph draw");return 0;
}

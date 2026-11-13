/* Run with SDL_VIDEODRIVER=dummy; validates the real SDL virtual-controller route. */
#define SDL_MAIN_HANDLED
#include "touch_overlay.c"
#include <assert.h>
static void finger(Uint32 type, Sint64 id,float x,float y,int menu){SDL_Event e;SDL_zero(e);e.type=type;e.tfinger.fingerId=id;e.tfinger.x=x;e.tfinger.y=y;assert(QVR_TouchEvent(&e,menu));}
int main(void){
 /* Dummy/hidden window has no OS focus. Permit synthetic controller input in this headless harness. */
 SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
 SDL_SetMainReady();assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER)==0);
 SDL_Window*w=SDL_CreateWindow("test",0,0,960,480,SDL_WINDOW_HIDDEN);assert(w);
 /* Reproduce the actual iPhone topology: non-controller device zero precedes
  * the touch Xbox device. Engine selection must resolve the owned instance. */
 int base=SDL_NumJoysticks();
 int other=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_UNKNOWN,2,2,0);assert(other==base);
 assert(!SDL_IsGameController(other));
 assert(QVR_TouchInit(w)==0);assert(SDL_IsGameController(device));
 assert(QVR_TouchDeviceIndex()==base+1);
 assert(SDL_JoystickDetachVirtual(other)==0);
 assert(QVR_TouchDeviceIndex()==base); /* hotplug reindex, same owned instance */
 device=QVR_TouchDeviceIndex();
 SDL_GameController*c=SDL_GameControllerOpen(device);assert(c);
 assert(SDL_GameControllerGetType(c)==SDL_CONTROLLER_TYPE_XBOX360);
 finger(SDL_FINGERDOWN,1,.64f,.78f,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);
 finger(SDL_FINGERDOWN,2,.15f,.60f,0);finger(SDL_FINGERMOTION,2,.22f,.55f,0);
 assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_LEFTX)>10000);
 finger(SDL_FINGERDOWN,3,.75f,.40f,0);finger(SDL_FINGERMOTION,3,.80f,.42f,0);
 int saw=0;SDL_Event e;while(SDL_PollEvent(&e))if(e.type==SDL_MOUSEMOTION&&e.motion.xrel>0)saw=1;assert(saw);
 assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);
 finger(SDL_FINGERUP,3,.80f,.42f,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)>30000);
 finger(SDL_FINGERUP,1,.64f,.78f,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)==0);
 SDL_zero(e);e.type=SDL_APP_WILLENTERBACKGROUND;QVR_TouchEvent(&e,0);assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_LEFTX)==0);
 finger(SDL_FINGERDOWN,6,.82f,.78f,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));assert(SDL_GameControllerGetAxis(c,SDL_CONTROLLER_AXIS_TRIGGERLEFT)==0);
 finger(SDL_FINGERUP,6,.82f,.78f,0);assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));
 finger(SDL_FINGERDOWN,7,.92f,.59f,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_B));
 finger(SDL_FINGERDOWN,8,.81f,.59f,0);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_Y));
 QVR_TouchReset();assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_B));assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_Y));
 finger(SDL_FINGERDOWN,4,.15f,.6f,1);finger(SDL_FINGERMOTION,4,.15f,.4f,1);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_DPAD_UP));
 finger(SDL_FINGERDOWN,5,.82f,.78f,1);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));
 finger(SDL_FINGERDOWN,9,.75f,.12f,1);assert(SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_B));
 QVR_TouchReset();assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_A));assert(!SDL_GameControllerGetButton(c,SDL_CONTROLLER_BUTTON_B));
 SDL_Surface*s=SDL_CreateRGBSurfaceWithFormat(0,960,480,32,SDL_PIXELFORMAT_RGBA32);assert(s);SDL_Renderer*r=SDL_CreateSoftwareRenderer(s);assert(r);menuMode=0;QVR_TouchDraw(r,960,480);
 const char*preview=SDL_getenv("QVR_TOUCH_PREVIEW");if(preview){SDL_RenderPresent(r);assert(SDL_SaveBMP(s,preview)==0);}
 SDL_DestroyRenderer(r);SDL_FreeSurface(s);SDL_GameControllerClose(c);SDL_Quit();puts("SDL controller, simultaneous fire/look/move, release, menu and glyph draw passed");return 0;
}

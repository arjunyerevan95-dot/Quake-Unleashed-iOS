#pragma once
#include "SDL.h"
#ifdef __cplusplus
extern "C" {
#endif
int QVR_TouchInit(SDL_Window *window);
int QVR_TouchDeviceIndex(void);
int QVR_TouchEvent(const SDL_Event *event, int menu);
void QVR_TouchDraw(SDL_Renderer *renderer, int width, int height);
void QVR_TouchReset(void);
#ifdef __cplusplus
}
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later
 * Private retail data stays in Documents/id1, never in the application bundle. */
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "SDL_syswm.h"
static SDL_Window *orientationWindow;
void QVR_IOSRequestLandscape(SDL_Window *window) {
 orientationWindow=window;
 SDL_SysWMinfo info;SDL_VERSION(&info.version);
 if(!window||!SDL_GetWindowWMInfo(window,&info))return;
 UIWindow *ui=info.info.uikit.window;
 [ui.rootViewController setNeedsUpdateOfSupportedInterfaceOrientations];
 if(ui.windowScene.activationState==UISceneActivationStateForegroundActive){
   UIWindowSceneGeometryPreferencesIOS *preferences=[[UIWindowSceneGeometryPreferencesIOS alloc] initWithInterfaceOrientations:UIInterfaceOrientationMaskLandscape];
   [ui.windowScene requestGeometryUpdateWithPreferences:preferences errorHandler:^(NSError *error){fprintf(stderr,"UNLEASHED landscape request: %s\n",error.localizedDescription.UTF8String);}];
 }
}
/* SDL sensors remain in portrait device axes. Map to the current landscape. */
float QVR_IOSLandscapeSign(void) {
 SDL_SysWMinfo info;SDL_VERSION(&info.version);
 if(!orientationWindow||!SDL_GetWindowWMInfo(orientationWindow,&info))return 0.f;
 UIInterfaceOrientation o=info.info.uikit.window.windowScene.interfaceOrientation;
 if(o==UIInterfaceOrientationLandscapeLeft)return 1.f;
 if(o==UIInterfaceOrientationLandscapeRight)return -1.f;
 return 0.f;
}
extern void QVR_IOSSuspend(void);
extern void QVR_IOSResume(void);
int QVR_IOSApplicationActive(void) { return UIApplication.sharedApplication.applicationState == UIApplicationStateActive; }
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Called before engine argument parsing; storage remains alive for process life. */
void QVR_IOSBootstrap(int *argc, char ***argv)
{
    @autoreleasepool {
        SDL_SetHint(SDL_HINT_ORIENTATIONS,"LandscapeLeft LandscapeRight");
        [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:nil usingBlock:^(NSNotification *note){ (void)note; QVR_IOSSuspend(); }];
        [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:nil usingBlock:^(NSNotification *note){ (void)note; QVR_IOSResume(); QVR_IOSRequestLandscape(orientationWindow); }];
        NSString *documents = NSSearchPathForDirectoriesInDomains(
            NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        NSString *frameworks = NSBundle.mainBundle.privateFrameworksPath;
        [[NSFileManager defaultManager] createDirectoryAtPath:
            [documents stringByAppendingPathComponent:@"id1"]
            withIntermediateDirectories:YES attributes:nil error:nil];
        if (chdir(documents.fileSystemRepresentation) != 0) {
            fprintf(stderr, "Cannot enter game Documents directory\n");
            abort();
        }
        freopen("unleashed-startup.log", "a", stderr);
        setvbuf(stderr, NULL, _IONBF, 0);
        freopen("unleashed-console.log", "a", stdout);
        setvbuf(stdout, NULL, _IONBF, 0);
        setenv("QVR_OSMESA_LIBRARY", [[frameworks stringByAppendingPathComponent:
            @"libOSMesa.dylib"] fileSystemRepresentation], 1);
        setenv("QVR_MOLTENVK_LIBRARY", [[frameworks stringByAppendingPathComponent:
            @"libMoltenVK.dylib"] fileSystemRepresentation], 1);
        static char *args[] = {"Unleashed", "-basedir", ".", "-width", "640",
            "-height", "360", "-window", "-game", "quakevr", "-flatscreenmode", "+vr_enabled", "0",
            "+vr_flat_holsters", "0", "+vr_flat_autoreload", "1", NULL};
        *argc = 17;
        *argv = args;
        fprintf(stderr, "UNLEASHED iOS launch candidate: private data %s/id1\n",
            documents.fileSystemRepresentation);
    }
}

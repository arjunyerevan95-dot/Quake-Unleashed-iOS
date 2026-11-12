/* SPDX-License-Identifier: GPL-2.0-or-later
 * Private retail data stays in Documents/id1, never in the application bundle. */
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Called before engine argument parsing; storage remains alive for process life. */
void QVR_IOSBootstrap(int *argc, char ***argv)
{
    @autoreleasepool {
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
            "-height", "360", "-window", "+vr_enabled", "0", NULL};
        *argc = 10;
        *argv = args;
        fprintf(stderr, "UNLEASHED iOS launch candidate: private data %s/id1\n",
            documents.fileSystemRepresentation);
    }
}

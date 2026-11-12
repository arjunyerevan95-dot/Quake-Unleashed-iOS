#import <UIKit/UIKit.h>
#include <stdlib.h>
#include "probe.h"

/* Asset-free diagnostic host, not a game launcher. Software readback presentation
 * deliberately avoids claiming a performant Metal swapchain has been implemented. */
@interface ProbeController : UIViewController
@property(nonatomic,strong) UITextView *text;
@property(nonatomic,strong) UIImageView *pixel;
@end
@implementation ProbeController
- (void)viewDidLoad {
 [super viewDidLoad]; self.view.backgroundColor=UIColor.blackColor;
 UIStackView *stack=[[UIStackView alloc] init];stack.axis=UILayoutConstraintAxisVertical;stack.spacing=16;
 stack.translatesAutoresizingMaskIntoConstraints=NO;[self.view addSubview:stack];
 [NSLayoutConstraint activateConstraints:@[
  [stack.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor constant:20],
  [stack.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor constant:-20],
  [stack.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:20],
  [stack.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-20]]];
 self.text=[[UITextView alloc] init];self.text.editable=NO;self.text.textColor=UIColor.whiteColor;
 self.text.backgroundColor=UIColor.blackColor;self.text.font=[UIFont monospacedSystemFontOfSize:16 weight:UIFontWeightRegular];
 self.text.text=@"UNLEASHED renderer probe\nNo game data needed.\nChoose strict capability test first. Experimental mode is NOT full GL4.3 support. Restart the app before a second test.";
 [stack addArrangedSubview:self.text];
 self.pixel=[[UIImageView alloc] init];self.pixel.contentMode=UIViewContentModeScaleToFill;
 [self.pixel.heightAnchor constraintEqualToConstant:60].active=YES;[stack addArrangedSubview:self.pixel];
 for(NSNumber *mode in @[@0,@1]){
  UIButton*b=[UIButton buttonWithType:UIButtonTypeSystem];b.tag=mode.integerValue;
  [b setTitle:mode.boolValue?@"Experimental feature test (version override)":@"Strict GL4.3 capability test" forState:UIControlStateNormal];
  [b addTarget:self action:@selector(run:) forControlEvents:UIControlEventTouchUpInside];[stack addArrangedSubview:b];
 }
}
- (void)run:(UIButton*)sender {
 for(UIView*v in sender.superview.subviews)if([v isKindOfClass:UIButton.class])((UIButton*)v).enabled=NO;
 self.text.text=@"Running engine shader compilation, program linking and textured GUI draw…";
 BOOL experimental=sender.tag!=0;
#ifdef QVR_SHADER_GATE
 setenv("QVR_SHADER_DIR", [[NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"shader-corpus"] UTF8String], 1);
#endif
 NSString*dir=NSBundle.mainBundle.privateFrameworksPath;
 dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED,0),^{
  qvru_probe_result result=qvru_osmesa_probe([[dir stringByAppendingPathComponent:@"libOSMesa.dylib"] UTF8String],[[dir stringByAppendingPathComponent:@"libMoltenVK.dylib"] UTF8String],experimental);
  NSString*report=[NSString stringWithUTF8String:result.detail];
  NSString*docs=NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject;
  [report writeToFile:[docs stringByAppendingPathComponent:@"renderer-probe.txt"] atomically:YES encoding:NSUTF8StringEncoding error:nil];
  NSData*data=[NSData dataWithBytes:result.rgba length:4];
  dispatch_async(dispatch_get_main_queue(),^{
   self.text.text=[report stringByAppendingString:@"\n\nDiagnostic only. The v3 gate tests 42 shader stages, 24 baseline program links and actual GUI textured draw/readback. Tile is a CPU-read-back pixel. No gameplay. No live Metal presentation or frame-rate validation yet. Restart before testing again."];
   if(result.passed){
    CGDataProviderRef provider=CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
    CGImageRef image=CGImageCreate(1,1,8,32,4,space,kCGImageAlphaLast|kCGBitmapByteOrderDefault,provider,NULL,NO,kCGRenderingIntentDefault);
    self.pixel.image=[UIImage imageWithCGImage:image];CGImageRelease(image);CGColorSpaceRelease(space);CGDataProviderRelease(provider);
   }
  });
 });
}
@end
@interface ProbeApp : UIResponder <UIApplicationDelegate>
@property(nonatomic,strong) UIWindow*window;
@end
@implementation ProbeApp
- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options{
 self.window=[[UIWindow alloc]initWithFrame:UIScreen.mainScreen.bounds];self.window.rootViewController=[[ProbeController alloc]init];[self.window makeKeyAndVisible];return YES;
}
@end
int main(int argc,char**argv){@autoreleasepool{return UIApplicationMain(argc,argv,nil,NSStringFromClass(ProbeApp.class));}}

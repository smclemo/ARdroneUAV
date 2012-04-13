//
//  TVOut.m
//  ARDroneEngine
//
//  Created by Frédéric D'HAEYER on 22/12/09.
//  Copyright 2009 Parrot. All rights reserved.
//
#import "TVOut.h"
#import "OpenGLSprite.h"

#ifdef INTERFACE_WITH_DEBUG

UIWindow* deviceWindow;
CGImageRef UIGetScreenImage();
MPTVOutWindow* tvoutWindow;
NSTimer *updateTimer;
UIImage *image;
UIImageView *mirrorView;
BOOL done;

@implementation MPVideoView (tvout)
- (void) addSubview: (UIView *) aView
{	
    [super addSubview:aView];
}
@end

extern Texture texture;
@implementation UIImage (tvout)
+ (UIImage *)imageWithScreenContents
{
    CGImageRef cgScreen = UIGetScreenImage();
    if (cgScreen) {
        UIImage *result = [UIImage imageWithCGImage:cgScreen];
        CGImageRelease(cgScreen);
        return result;
    }
    return nil;
}
@end

@interface TVOut ()
- (void) updateTVOut;
- (void) updateLoop;
@end

@implementation TVOut
- (void) startTVOut
{
	// you need to have a main window already open when you call start
	if (!tvoutWindow) {		
		deviceWindow = [[UIApplication sharedApplication] keyWindow];
		
		MPVideoView *vidView = [[MPVideoView alloc] initWithFrame: CGRectZero];	
		tvoutWindow = [[MPTVOutWindow alloc] initWithVideoView:vidView];
		[tvoutWindow makeKeyAndVisible];
		tvoutWindow.userInteractionEnabled = NO;
		
		mirrorView = [[UIImageView alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
		mirrorView.transform = CGAffineTransformScale(CGAffineTransformRotate(mirrorView.transform, M_PI * 1.5), 1.4, 1.4);
		mirrorView.center = vidView.center;
		
		[vidView addSubview: mirrorView];
		
		[deviceWindow makeKeyAndVisible];
		
		[NSThread detachNewThreadSelector:@selector(updateLoop) toTarget:self withObject:nil];
	}
}

- (void) stopTVOut
{
	done = YES;
	if (updateTimer) {
		[updateTimer invalidate];
		[updateTimer release];
		updateTimer = nil;
	}
	if (tvoutWindow) {
		[tvoutWindow release];
		tvoutWindow = nil;
	}
}

- (void) updateTVOut
{
	image = [UIImage imageWithScreenContents];
	mirrorView.image = image;
}

- (void) updateLoop
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    done = NO;
	
    while ( ! done )
    {
		[self performSelectorOnMainThread:@selector(updateTVOut) withObject:nil waitUntilDone:NO];
        [NSThread sleepForTimeInterval: (1.0/kFPS) ];
    }
    [pool release];
}

@end

#endif // INTERFACE_WITH_DEBUG
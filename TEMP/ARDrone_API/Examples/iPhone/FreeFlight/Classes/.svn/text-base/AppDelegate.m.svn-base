//
//  AppDelegate.m
//  FreeFlight
//
//  Created by Mykonos on 16/10/09.
//  Copyright Parrot SA 2009. All rights reserved.
//
#import "AppDelegate.h"
#import "EAGLView.h"
#import "MenuUpdater.h"

@interface AppDelegate (private)
- (void) disableDetection;
@end

@implementation AppDelegate
@synthesize window;
@synthesize menuController;

- (void) disableDetection
{
	ARDroneNavigationData data;
	[drone navigationData:&data];
	if(data.detection_type != ARDRONE_CAMERA_NONE)
	{
		ARDRONE_CAMERA_DETECTION_PARAM camParam;
		camParam.tag_size = 0;
		camParam.camera_type = ARDRONE_CAMERA_NONE;		
		NSLog(@"Disable detection\n");
		[drone executeCommandIn:ARDRONE_COMMAND_CAMERA_DETECTION withParameter:(void *)&camParam fromSender:nil];
	}
}

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	NSLog(@"app did finish launching");
	application.idleTimerDisabled = YES;

	// Setup the menu controller
	menuController.delegate = self;
	NSLog(@"menu controller view %@", menuController.view);
	
	// Setup the ARDrone 
	ARDroneHUDConfiguration hudconfiguration = {YES, NO};
	drone = [[ARDrone alloc] initWithFrame:menuController.view.frame withState:NO withDelegate:nil withHUDConfiguration:&hudconfiguration];
	[drone setScreenOrientationRight:YES];

	// Setup the OpenGL view
	glView = [[EAGLView alloc] initWithFrame:CGRectMake(menuController.view.frame.origin.x, menuController.view.frame.origin.y, menuController.view.frame.size.height, menuController.view.frame.size.width)];
	[glView changeState:NO];
	[glView setDrone:drone];
	
	was_in_game = NO;

	[menuController.view addSubview:glView];
	[menuController.view addSubview:drone.view];

	
	[window addSubview:menuController.view];
    [window makeKeyAndVisible];
}

- (void)changeState:(BOOL)inGame
{
	was_in_game = inGame;
	if(inGame)
		[drone setScreenOrientationRight:([[UIApplication sharedApplication] statusBarOrientation] == UIInterfaceOrientationLandscapeRight)];
	
	[glView changeState:inGame];
	[drone changeState:inGame];
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	// Become inactive
	if(was_in_game)
	{
		[drone changeState:NO];
		[glView changeState:NO];
	}
	else
	{
		[menuController changeState:NO];
	}
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	if(was_in_game)
	{
		[drone changeState:YES];
		[glView changeState:YES];
	}
	else
	{
		[menuController changeState:YES];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	if(was_in_game)
	{
		[drone changeState:NO];
		[glView changeState:NO];
	}
	else
	{
		[menuController changeState:NO];
	}
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	
}

- (BOOL)checkState
{
	BOOL result = NO;
	
	if(was_in_game)
	{
		result = [drone checkState];
	}
	else
	{
		//result = [menuController checkState];
	}
	
	return result;
}

@end

//
//  ARUpdaterAppDelegate.m
//  AR.Updater
//
//  Created by Robert Ryll on 10-05-14.
//  Copyright Playsoft 2010. All rights reserved.
//

#import "ARUpdaterAppDelegate.h"
#import "ARUpdaterViewController.h"

@implementation ARUpdaterAppDelegate

@synthesize window;
@synthesize viewController;

- (void)applicationDidFinishLaunching:(UIApplication *)application {    
	[[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
}

- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}

@end

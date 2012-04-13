//
//  MainViewController.h
//  ARDroneEngine
//
//  Created by Mykonos on 17/12/09.
//  Copyright 2009 Parrot SA. All rights reserved.
//

#include "ConstantsAndMacros.h"

#import <UIKit/UIKit.h>
#import "HUD.h"
#import "SettingsMenu.h"
#import "ARDrone.h"
#import "ARDroneProtocols.h"
#import "InternalProtocols.h"

@interface MainViewController : UIViewController 
{
@private
	/**
	 * Id of Game engine implementing protocol 
	 */
	id<ARDroneProtocolOut> gameEngine;
	
	BOOL bye;
	
	HUD			 *hud;
	SettingsMenu *menuSettings;
}

- (id) initWithFrame:(CGRect)frame withState:(BOOL)inGame withDelegate:(id<ARDroneProtocolOut>)delegate withNavdataDelegate:(id<NavdataProtocol>)_navdata_delegate withHUDConfiguration:(ARDroneHUDConfiguration*)hudconfiguration;
- (void)setScreenOrientationRight:(BOOL)right;
- (ARDroneEnemiesData*)humanEnemiesData;
- (ARDroneNavigationData*)navigationData;
- (ARDroneCamera*)droneCamera;
- (ARDroneDetectionCamera*)detectionCamera;
- (void)changeState:(BOOL)inGame;
- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender;
- (void)setWifiReachabled:(BOOL)reachabled;

@end

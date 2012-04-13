//
//  TVOut.h
//  ARDroneEngine
//
//  Created by Frédéric D'HAEYER on 22/12/09.
//  Copyright 2009 Parrot SA. All rights reserved.
//
#include "ConstantsAndMacros.h"

#ifdef INTERFACE_WITH_DEBUG

#define kUseBackgroundThread	YES

@interface TVOut : NSObject

- (void) startTVOut;
- (void) stopTVOut;
@end

@interface MPTVOutWindow : UIWindow
- (id)initWithVideoView:(id)fp8;
@end

@interface MPVideoView : UIView
- (id)initWithFrame:(struct CGRect)fp8;
@end

@interface UIImage (tvout)
+ (UIImage *)imageWithScreenContents;
@end
#endif // INTERFACE_WITH_DEBUG
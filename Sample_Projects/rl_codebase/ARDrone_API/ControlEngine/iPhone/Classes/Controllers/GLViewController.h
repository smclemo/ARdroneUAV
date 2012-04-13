/**
 *  @file GLViewController.h
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#include "ConstantsAndMacros.h"

#import "OpenGLVideo.h"
#import "OpenGLSprite.h"
#import "ARDrone.h"
#import "InternalProtocols.h"

@class OpenGLVideo;

@interface GLViewController : NSObject {
@private
	OpenGLVideo   *video;
	
	id delegate;
}

@property (nonatomic, assign) OpenGLVideo	*video;

- (id)initWithFrame:(CGRect)frame withDelegate:(id<NavdataProtocol>)delegate;
- (void)drawView;
@end

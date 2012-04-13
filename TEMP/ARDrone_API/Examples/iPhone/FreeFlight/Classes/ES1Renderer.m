//
//  ES1Renderer.m
//  FreeFlight
//
//  Created by Mykonos on 16/10/09.
//  Copyright Parrot SA 2009. All rights reserved.
//
#import <AudioToolbox/AudioToolbox.h>
#import "ES1Renderer.h"
#import "ARDrone.h"

//#define DISPLAY_CUBE
#ifdef DISPLAY_CUBE
const GLfloat cubeVertices[] = {
	
	// face de devant
	-1.0, 1.0, 1.0,            
	-1.0, -1.0, 1.0,          
	1.0, -1.0, 1.0,            
	1.0, 1.0, 1.0,             
	
	// haut
	-1.0, 1.0, -1.0,          
	-1.0, 1.0, 1.0,            
	1.0, 1.0, 1.0,             
	1.0, 1.0, -1.0,            
	
	// arrière
	1.0, 1.0, -1.0,        
	1.0, -1.0, -1.0,          
	-1.0, -1.0, -1.0,          
	-1.0, 1.0, -1.0,           
	
	// dessous
	-1.0, -1.0, 1.0,
	-1.0, -1.0, -1.0,
	1.0, -1.0, -1.0,
	1.0, -1.0, 1.0,
	
	// gauche
	-1.0, 1.0, -1.0,
	-1.0, 1.0, 1.0,
	-1.0, -1.0, 1.0,
	-1.0, -1.0, -1.0,
	
	// droit
	1.0, 1.0, 1.0,
	1.0, 1.0, -1.0,
	1.0, -1.0, -1.0,
	1.0, -1.0, 1.0
}; 

const GLshort cubeTextureCoords[] = {
	// devant
	0, 1,    
	0, 0,    
	1, 0,     
	1, 1,    
	
	// dessus
	0, 1,   
	0, 0,     
	1, 0,    
	1, 1,    
	
	// derrière
	0, 1,    
	0, 0,     
	1, 0,    
	1, 1,    
	
	// dessous
	0, 1,   
	0, 0,     
	1, 0,    
	1, 1,   
	
	// gauche
	0, 1,  
	0, 0,     
	1, 0,      
	1, 1,      
	
	// droite
	0, 1,     
	0, 0,      
	1, 0,      
	1, 1,      
};
#endif

@implementation ES1Renderer

// Create an ES 1.1 context
- (id) init
{
	if (self = [super init])
	{
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            [self release];
            return nil;
        }
		
		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffersOES(1, &defaultFramebuffer);
		glGenRenderbuffersOES(1, &colorRenderbuffer);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
	}
	
	return self;
}

- (void) render:(ARDrone*)drone
{	
	// This application only creates a single context which is already set current at this point.
	// This call is redundant, but needed if dealing with multiple contexts.
    [EAGLContext setCurrentContext:context];
    
	// This application only creates a single default framebuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
    glViewport(0, 0, backingWidth, backingHeight);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(drone != nil)
		[drone render];
	
#ifdef DISPLAY_CUBE
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	// dessin du cube
	glLoadIdentity();
	glScalef(0.2, 0.2, 0.2);
	glRotatef(25.0, 1.0, -1.0, 0.0);
	
	glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glTexCoordPointer(2, GL_SHORT, 0, cubeTextureCoords); // nouvelle ligne
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);          // nouvelle ligne
	
	glColor4f(0.0, 0.0, 0.8, 1.0); // R V B Alpha
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	glColor4f(0.0, 0.8, 0.0, 1.0); // R V B Alpha
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	
	glColor4f(0.8, 0.0, 0.0, 1.0); // R V B Alpha
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	
	glColor4f(0.8, 0.0, 0.8, 1.0); // R V B Alpha
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	
	glColor4f(0.8, 0.8, 0.8, 1.0); // R V B Alpha
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	
	glColor4f(0.8, 0.8, 0.0, 1.0); // R V B Alpha
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
#endif
	
	// This application only creates a single color renderbuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{	
	// Allocate color buffer backing based on the current layer size
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:layer];
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}

- (void) dealloc
{
	// Tear down GL
	if (defaultFramebuffer)
	{
		glDeleteFramebuffersOES(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}

	if (colorRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
	
	// Tear down context
	if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	
	[context release];
	context = nil;
	
	[super dealloc];
}

@end

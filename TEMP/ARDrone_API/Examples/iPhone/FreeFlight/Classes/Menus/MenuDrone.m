#import "MenuDrone.h"

@implementation MenuDrone

- (id)initWithController:(MenuController*)menuController
{
	if(self = [super initWithNibName:@"MenuDrone" bundle:nil])
	{
		NSLog(@"%s", __FUNCTION__);
		controller = menuController;
	}
	return self;
}

- (void)viewDidLoad
{
	NSLog(@"%s", __FUNCTION__);
	[self changeState:YES];
}

- (void)refresh:(unsigned int)frameCount
{
	
}

- (void)changeState:(BOOL)inGame
{
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	
}

- (BOOL)checkState
{
	return [drone checkState];
}

- (void) dealloc
{
	[drone release];
	drone = nil;
	[glView release];
	
	[super dealloc];
}

@end

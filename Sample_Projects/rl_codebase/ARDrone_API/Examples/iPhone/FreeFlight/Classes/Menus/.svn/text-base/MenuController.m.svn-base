#import "MenuController.h"
#import "MenuUpdater.h"

@implementation MenuController
@synthesize delegate;
@synthesize menuCurrent;
@synthesize screenOrientationRight;

- (void)viewDidLoad
{
	NSLog(@"view did load");
	[super viewDidLoad];
	
	// Initialize the menu class history and set the very first menu
	menuClassHistory = [[NSMutableArray arrayWithCapacity:5] retain];
	menuNextClass = [MenuUpdater class];
	self.menuCurrent = [[menuNextClass alloc] initWithController:self];
	[self.view addSubview:menuCurrent.view];
}

- (void)changeMenu:(Class)menuClass
{
	// Make sure there isn't a change in progress
	if([menuCurrent class] != menuNextClass)
	{
		return;
	}

	// Disable user interactions in the current menu to prevent its buttons to react to touch events
	menuCurrent.view.userInteractionEnabled = NO;

	// Take note of the class of the next menu
	// Note: the menu isn't instantly changed, modifications are performed when the sliders are closed
	if(menuClass)
	{
		[menuClassHistory addObject:menuNextClass];
		menuNextClass = menuClass;

		if([menuCurrent class] != menuNextClass)
		{
			// Unload the current menu
			[menuCurrent.view removeFromSuperview];
			
			// Load the next menu
			self.menuCurrent = [[menuNextClass alloc] initWithController:self];
			[self.view addSubview:menuCurrent.view];
		}
	}
	else
	{
		NSLog(@"move back to previous menu");
		if([menuClassHistory count] > 0)
		{
			menuNextClass = [menuClassHistory lastObject];
			[menuClassHistory removeLastObject];
		}
		else
		{
			// Unload the current menu
			[menuCurrent.view removeFromSuperview];
			[delegate changeState:YES];
		}
	}
}

- (void)changeState:(BOOL)inGame
{
	NSLog(@"%s", __FUNCTION__);
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	
}

- (BOOL)checkState
{
	return YES;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
	BOOL result = FALSE;

	if([delegate checkState] == NO)
		result = (interfaceOrientation == UIInterfaceOrientationLandscapeRight || interfaceOrientation == UIInterfaceOrientationLandscapeLeft);
	
	return result;
}

- (void)dealloc
{
	// Release the current menu
	[menuCurrent release];

	// Release the menu class history
	[menuClassHistory release];

	// Deallocate everthing else
	[super dealloc];
}

@end

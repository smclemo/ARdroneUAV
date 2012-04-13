#import <UIKit/UIKit.h>
#import "ARDroneProtocols.h"
#define framesPerSecond 30.0f

@class MenuController;

@protocol MenuProtocol

- (id)initWithController:(MenuController*)menuController;
- (void)refresh:(unsigned int)frameCount;

@end

@interface MenuController : UIViewController <ARDroneProtocolIn>
{
	id <ARDroneProtocolIn> delegate;
	
	NSMutableArray* menuClassHistory;
	UIViewController <MenuProtocol,ARDroneProtocolIn>* menuCurrent;
	Class menuNextClass;
}

@property (nonatomic, assign) BOOL screenOrientationRight;
@property (nonatomic, assign) id<ARDroneProtocolIn> delegate;
@property (nonatomic, retain) UIViewController <MenuProtocol,ARDroneProtocolIn>* menuCurrent;

- (void)changeMenu:(Class)menuClass;

@end

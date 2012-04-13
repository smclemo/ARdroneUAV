#include "ConstantsAndMacros.h"
#import "ARDrone.h"
#import "SettingsMenu.h"

@interface HUD : UIViewController <UIAccelerometerDelegate> {
    IBOutlet UILabel	 *messageBoxLabel;
    IBOutlet UILabel	 *batteryLevelLabel;
	
	IBOutlet UIImageView *batteryImageView;
	IBOutlet UIImageView *wifiImageView;
	IBOutlet UIImageView *joystickRightThumbImageView;
	IBOutlet UIImageView *joystickRightBackgroundImageView;
	IBOutlet UIImageView *joystickLeftThumbImageView;
	IBOutlet UIImageView *fingerPrintImageView;
	
	IBOutlet UIButton	 *backToMainMenuButton;
	IBOutlet UIButton    *settingsButton;
	IBOutlet UIButton    *switchScreenButton;
    IBOutlet UIButton    *takeOffButton;
	IBOutlet UIButton	 *emergencyButton;
	
	IBOutlet UIButton	 *joystickRightButton;
	IBOutlet UIButton	 *joystickLeftButton;	
	
	BOOL firePressed;
	BOOL settingsPressed;
	BOOL mainMenuPressed;
}

@property (nonatomic, assign) BOOL firePressed;
@property (nonatomic, assign) BOOL settingsPressed;
@property (nonatomic, assign) BOOL mainMenuPressed;

- (id)initWithFrame:(CGRect)frame withHUDConfiguration:(ARDroneHUDConfiguration)hudconfiguration withControlData:(ControlData*)data;
- (void)changeControlInterface:(BOOL)isAceControlLevel;
- (void)refreshInterface;
- (void)setMessageBox:(NSString*)str;
- (void)setTakeOff:(NSString*)str;
- (void)setEmergency:(NSString*)str;
- (void)setBattery:(int)percent;
- (void)setWifi:(int)percent;
- (void)setSpeed:(int)percent;
- (void)setAltitude:(int)percent;

- (IBAction)buttonPress:(id)sender forEvent:(UIEvent *)event;
- (IBAction)buttonRelease:(id)sender forEvent:(UIEvent *)event;
- (IBAction)buttonClick:(id)sender forEvent:(UIEvent *)event;
- (IBAction)buttonDrag:(id)sender forEvent:(UIEvent *)event;
@end

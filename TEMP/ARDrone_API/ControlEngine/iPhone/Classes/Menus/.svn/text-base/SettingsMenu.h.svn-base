#include "ConstantsAndMacros.h" 
#import "ARDrone.h"

typedef struct 
{
	BOOL  logsActivated;
} SettingsParams;

@interface SettingsMenu : UIViewController <UIScrollViewDelegate> {
	IBOutlet UISwitch *altituteLimitedSwitch;
    IBOutlet UISwitch *outdoorFlightSwitch;
    IBOutlet UISwitch *outdoorShellSwitch;
    IBOutlet UISwitch *logsSwitch;
   
	IBOutlet UIButton *clearLogsButton, *clearButton0, *clearButton1;
	IBOutlet UIButton *flatTrimButton0, *flatTrimButton1;
	
	IBOutlet UISegmentedControl *controlLevelSegmentedControl;	
	IBOutlet UISegmentedControl *enemyColorsSegmentedControl;	
	
    IBOutlet UILabel  *droneTiltMinLabel;
    IBOutlet UILabel  *droneTiltMaxLabel;
    IBOutlet UILabel  *droneTiltCurrentLabel;
    IBOutlet UISlider *droneTiltSlider;
	
	IBOutlet UILabel  *iPhoneTiltMinLabel;
    IBOutlet UILabel  *iPhoneTiltMaxLabel;
    IBOutlet UILabel  *iPhoneTiltCurrentLabel;
    IBOutlet UISlider *iPhoneTiltSlider;

	IBOutlet UILabel  *verticalSpeedMinLabel;
    IBOutlet UILabel  *verticalSpeedMaxLabel;
    IBOutlet UILabel  *verticalSpeedCurrentLabel;
	IBOutlet UISlider *verticalSpeedSlider;
	
	IBOutlet UILabel  *yawSpeedMinLabel;
    IBOutlet UILabel  *yawSpeedMaxLabel;
    IBOutlet UILabel  *yawSpeedCurrentLabel;
    IBOutlet UISlider *yawSpeedSlider;
	
	IBOutlet UISegmentedControl *trimSegmentedControl;	

	IBOutlet UITextField	*droneSSIDTextField;
	
	IBOutlet UILabel  *droneTrimRollMinLabel;
    IBOutlet UILabel  *droneTrimRollMaxLabel;
    IBOutlet UILabel  *droneTrimRollCurrentLabel;
    IBOutlet UISlider *droneTrimRollSlider;

	IBOutlet UILabel  *droneTrimPitchMinLabel;
    IBOutlet UILabel  *droneTrimPitchMaxLabel;
    IBOutlet UILabel  *droneTrimPitchCurrentLabel;
    IBOutlet UISlider *droneTrimPitchSlider;

	IBOutlet UILabel  *droneTrimYawMinLabel;
    IBOutlet UILabel  *droneTrimYawMaxLabel;
    IBOutlet UILabel  *droneTrimYawCurrentLabel;
    IBOutlet UISlider *droneTrimYawSlider;

	IBOutlet UILabel  *softwareVersion;
	IBOutlet UILabel  *droneHardVersion, *droneSoftVersion;
	IBOutlet UILabel  *dronePicHardVersion, *dronePicSoftVersion;
	IBOutlet UILabel  *droneMotor1HardVersion, *droneMotor1SoftVersion, *droneMotor1SupplierVersion;
	IBOutlet UILabel  *droneMotor2HardVersion, *droneMotor2SoftVersion, *droneMotor2SupplierVersion;
	IBOutlet UILabel  *droneMotor3HardVersion, *droneMotor3SoftVersion, *droneMotor3SupplierVersion;
	IBOutlet UILabel  *droneMotor4HardVersion, *droneMotor4SoftVersion, *droneMotor4SupplierVersion;

#ifdef INTERFACE_WITH_DEBUG
	IBOutlet UILabel  *tvOutLabel;
    IBOutlet UISwitch *tvoutSwitch;
#endif
}

- (id)initWithFrame:(CGRect)frame AndHUDConfiguration:(ARDroneHUDConfiguration)configuration withControlData:(ControlData*)data;
- (void)switchDisplay;
- (BOOL)isAceControlLevel;
- (void)configChanged;

- (IBAction)logsChanged:(id)sender;
- (IBAction)valueChanged:(id)sender;
- (IBAction)trimChanged:(id)sender;
- (IBAction)clearLogsButtonClick:(id)sender;
- (IBAction)clearButtonClick:(id)sender;
- (IBAction)flatTrimButtonClick:(id)sender;
- (IBAction)trimSelectChanged:(id)sender;
- (IBAction)okButtonClick:(id)sender;
- (IBAction)sliderRelease:(id)sender;

#ifdef INTERFACE_WITH_DEBUG
- (IBAction)tvoutChanged:(id)sender;
#endif
@end

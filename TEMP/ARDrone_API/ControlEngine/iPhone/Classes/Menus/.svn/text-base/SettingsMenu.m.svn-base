#import "SettingsMenu.h"
#import "TVOut.h"

#define REFRESH_TRIM_TIMEOUT	1
#define TRIM_MAX_RETRIES		3
#define ALTITUDE_LIMITED		3000
#define NO_ALTITUDE				10000
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	void saveSettings(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

typedef enum
{
	TRIM_STATE_IDLE,
	TRIM_STATE_INPROGRESS,
	TRIM_STATE_ENDED,
} TRIM_STATES;

typedef enum
{
	TRIM_TYPE_AUTO,
	TRIM_TYPE_MANUAL,	
} TRIM_TYPE;

struct tm *settings_atm = NULL;

void saveSettings(void)
{
	struct timeval tv;
	// Save backups of Settings in text
	gettimeofday(&tv,NULL);
	settings_atm = localtime(&tv.tv_sec);
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); //creates paths so that you can pull the app's path from it
	NSString *documentsDirectory = [paths objectAtIndex:0]; // gets the applications directory on the users iPhone

	const char *filename = [[documentsDirectory stringByAppendingFormat:@"/settings_%04d%02d%02d_%02d%02d%02d.txt",
					   settings_atm->tm_year+1900, settings_atm->tm_mon+1, settings_atm->tm_mday,
					   settings_atm->tm_hour, settings_atm->tm_min, settings_atm->tm_sec] cStringUsingEncoding:NSASCIIStringEncoding];
	FILE *file = fopen(filename, "w");
	if(file)
	{
		fprintf(file, "Drone Network SSID : %s\n", ardrone_control_config.ssid_single_player);
		fprintf(file, "Altitude Limited : %s\n", (ardrone_control_config.altitude_max == ALTITUDE_LIMITED) ? "ON" : "OFF");
		fprintf(file, "Outdoor Shell    : %s\n", ardrone_control_config.flight_without_shell ? "OFF" : "ON");
		fprintf(file, "Outdoor Flight   : %s\n", ardrone_control_config.outdoor ? "ON" : "OFF");
		fprintf(file, "Control level    : %s\n", ((CONTROL_LEVEL)ardrone_control_config.control_level == CONTROL_LEVEL_ACE) ? "ACE" : "BEGINNER");
		fprintf(file, "Enemy colors     : %s\n", (ardrone_control_config.enemy_colors == (int)ORANGE_GREEN) ? "GREEN" : ((ardrone_control_config.enemy_colors == (int)ORANGE_YELLOW) ? "YELLOW" : "BLUE"));
		fprintf(file, "Iphone Tilt      : %0.2f\n", ardrone_control_config.control_iphone_tilt * RAD_TO_DEG);
		fprintf(file, "Drone Tilt       : %0.2f\n", ardrone_control_config.euler_angle_max * RAD_TO_DEG);
		fprintf(file, "Vertical Speed   : %0.2f\n", (float)ardrone_control_config.control_vz_max);
		fprintf(file, "Yaw Speed        : %0.2f\n", ardrone_control_config.control_yaw * RAD_TO_DEG);				
		fclose(file);
	}
	else 
	{
		NSLog(@"%s not found", filename);
	}		
}

@interface SettingsMenu ()
BOOL manualTrimInProgress;
ControlData *controlData;
#ifdef INTERFACE_WITH_DEBUG
TVOut *tvout;
#endif
BOOL ssidChangeInProgress;

- (void) refresh;
- (void) refreshTimeout;
- (BOOL)textFieldShouldReturn:(UITextField *)theTextField;
@end

@implementation SettingsMenu
- (id)initWithFrame:(CGRect)frame AndHUDConfiguration:(ARDroneHUDConfiguration)configuration withControlData:(ControlData*)data
{
	NSLog(@"SettingsMenu frame => w : %f, h : %f", frame.size.width, frame.size.height);

#ifdef INTERFACE_WITH_DEBUG
	if(self = [super initWithNibName:@"SettingsMenuDebug" bundle:nil])
#else
	if(self = [super initWithNibName:@"SettingsMenu" bundle:nil])
#endif
	{
		controlData = data;
		manualTrimInProgress = NO;
		ssidChangeInProgress = NO;
		
		// Set parameters of scrollview
		UIScrollView *scrollView = (UIScrollView*)self.view;
		scrollView.alwaysBounceHorizontal = NO;
		scrollView.alwaysBounceVertical = YES;
		scrollView.clipsToBounds = YES;
		scrollView.contentSize = CGSizeMake(scrollView.frame.size.width, scrollView.frame.size.height);
		scrollView.frame = CGRectMake(frame.origin.x, frame.origin.y, frame.size.height, frame.size.width);		
		scrollView.delegate = self;
		scrollView.scrollEnabled = YES;
		scrollView.indicatorStyle = UIScrollViewIndicatorStyleWhite;
		
		// Initialize values (min, max, value)
		yawSpeedSlider.minimumValue = 40.0;
		yawSpeedSlider.maximumValue = 350.0;
		verticalSpeedSlider.minimumValue = 200.0;
		verticalSpeedSlider.maximumValue = 2000.0;
		droneTiltSlider.minimumValue = 5.0;
		droneTiltSlider.maximumValue = 30.0;
		iPhoneTiltSlider.minimumValue = 5.0;
		iPhoneTiltSlider.maximumValue = 50.0;
		droneTrimRollSlider.minimumValue = -15.0;
		droneTrimRollSlider.maximumValue = 15.0;
		droneTrimPitchSlider.minimumValue = -15.0;
		droneTrimPitchSlider.maximumValue = 15.0;
		droneTrimYawSlider.minimumValue = -15.0;
		droneTrimYawSlider.maximumValue = 15.0;

		yawSpeedMinLabel.text = [NSString stringWithFormat:@"%0.2f", yawSpeedSlider.minimumValue];
		yawSpeedMaxLabel.text = [NSString stringWithFormat:@"%0.2f", yawSpeedSlider.maximumValue];		
		verticalSpeedMinLabel.text = [NSString stringWithFormat:@"%0.2f", verticalSpeedSlider.minimumValue];
		verticalSpeedMaxLabel.text = [NSString stringWithFormat:@"%0.2f", verticalSpeedSlider.maximumValue];
		droneTiltMinLabel.text = [NSString stringWithFormat:@"%0.2f", droneTiltSlider.minimumValue];
		droneTiltMaxLabel.text = [NSString stringWithFormat:@"%0.2f", droneTiltSlider.maximumValue];		
		iPhoneTiltMinLabel.text = [NSString stringWithFormat:@"%0.2f", iPhoneTiltSlider.minimumValue];
		iPhoneTiltMaxLabel.text = [NSString stringWithFormat:@"%0.2f", iPhoneTiltSlider.maximumValue];		
		droneTrimRollMinLabel.text = [NSString stringWithFormat:@"%0.2f", droneTrimRollSlider.minimumValue];
		droneTrimRollMaxLabel.text = [NSString stringWithFormat:@"%0.2f", droneTrimRollSlider.maximumValue];		
		droneTrimPitchMinLabel.text = [NSString stringWithFormat:@"%0.2f", droneTrimPitchSlider.minimumValue];
		droneTrimPitchMaxLabel.text = [NSString stringWithFormat:@"%0.2f", droneTrimPitchSlider.maximumValue];		
		droneTrimYawMinLabel.text = [NSString stringWithFormat:@"%0.2f", droneTrimYawSlider.minimumValue];
		droneTrimYawMaxLabel.text = [NSString stringWithFormat:@"%0.2f", droneTrimYawSlider.maximumValue];		
		
		softwareVersion.text	= SOFTWARE_VERSION;
		logsSwitch.on = NO;
		
#ifdef INTERFACE_WITH_DEBUG
		// Create TV instance
		tvoutSwitch.on = NO;
		tvout = [[TVOut alloc] init];
#endif		
		trimSegmentedControl.selectedSegmentIndex = TRIM_TYPE_AUTO;
		
		[self refresh];
		
		[NSTimer scheduledTimerWithTimeInterval:REFRESH_TRIM_TIMEOUT target:self selector:@selector(refreshTimeout) userInfo:nil repeats:YES];
	}
	
	return self;
}

- (void)refreshTimeout
{
	static int numretries = 0;

	BOOL enabled = (controlData->bootstrap) ? NO : YES; 
	float alpha = (controlData->bootstrap) ? 0.5 : 1.0;
	
	altituteLimitedSwitch.enabled = outdoorFlightSwitch.enabled = outdoorShellSwitch.enabled = logsSwitch.enabled = enabled;
	altituteLimitedSwitch.alpha = outdoorFlightSwitch.alpha = outdoorShellSwitch.alpha = logsSwitch.alpha = alpha;
	controlLevelSegmentedControl.enabled = enemyColorsSegmentedControl.enabled =  trimSegmentedControl.enabled = droneSSIDTextField.enabled = enabled;
	controlLevelSegmentedControl.alpha = enemyColorsSegmentedControl.alpha = trimSegmentedControl.alpha = droneSSIDTextField.alpha = alpha;
	droneTiltSlider.enabled = iPhoneTiltSlider.enabled = verticalSpeedSlider.enabled = yawSpeedSlider.enabled = enabled;
	droneTiltSlider.alpha = iPhoneTiltSlider.alpha = verticalSpeedSlider.alpha = yawSpeedSlider.alpha = alpha;
	clearButton0.enabled = clearButton1.enabled = enabled;
	clearButton0.alpha = clearButton1.alpha = alpha;
	
	flatTrimButton0.enabled = flatTrimButton1.enabled = (((TRIM_TYPE)trimSegmentedControl.selectedSegmentIndex == TRIM_TYPE_AUTO) && enabled);
	flatTrimButton0.alpha = flatTrimButton1.alpha = ((((TRIM_TYPE)trimSegmentedControl.selectedSegmentIndex == TRIM_TYPE_AUTO) && enabled) ? 1.0 : 0.5);

	if(manualTrimInProgress)
	{
		if(controlData->manual_trim_enabled != controlData->manual_trim)
		{
			numretries++;
			if(numretries > TRIM_MAX_RETRIES)
			{
				numretries = 0;
				switch ((TRIM_TYPE)trimSegmentedControl.selectedSegmentIndex) 
				{
					case TRIM_TYPE_AUTO:
						trimSegmentedControl.selectedSegmentIndex = TRIM_TYPE_MANUAL;
						break;
						
					case TRIM_TYPE_MANUAL:
					default:
						trimSegmentedControl.selectedSegmentIndex = TRIM_TYPE_AUTO;
						break;
				}
				manualTrimInProgress = NO;
				trimSegmentedControl.enabled = TRUE;
				trimSegmentedControl.alpha = 1.0;
				[self refresh];
			}
		}
		else
		{
			numretries = 0;
			manualTrimInProgress = NO;
			trimSegmentedControl.enabled = TRUE;
			trimSegmentedControl.alpha = 1.0;			
			[self refresh];
		}
	}
	else
	{
		droneTrimPitchSlider.value = -controlData->trim_pitch;
		droneTrimRollSlider.value = controlData->trim_roll;
		droneTrimYawSlider.value = controlData->trim_yaw;
		[self refresh];
	}
}

- (void)configChanged
{
	enemyColorsSegmentedControl.selectedSegmentIndex = (ardrone_control_config.enemy_colors - 1);
	controlLevelSegmentedControl.selectedSegmentIndex = ardrone_control_config.control_level; 
	altituteLimitedSwitch.on = (ardrone_control_config.altitude_max == ALTITUDE_LIMITED) ? YES : NO;	
	outdoorFlightSwitch.on = ardrone_control_config.outdoor ? YES : NO;
	outdoorShellSwitch.on = ardrone_control_config.flight_without_shell ? YES : NO;
	droneTiltSlider.value = ardrone_control_config.euler_angle_max * RAD_TO_DEG;
	iPhoneTiltSlider.value = ardrone_control_config.control_iphone_tilt * RAD_TO_DEG;
	verticalSpeedSlider.value = ardrone_control_config.control_vz_max;
	yawSpeedSlider.value = ardrone_control_config.control_yaw * RAD_TO_DEG;
	
	// Update SSID AR.Drone network
	if(!ssidChangeInProgress && (strcmp(ardrone_control_config.ssid_single_player, "") != 0))
		droneSSIDTextField.text = [NSString stringWithCString:ardrone_control_config.ssid_single_player encoding:NSASCIIStringEncoding];
	
	// Update pic version number
	if(ardrone_control_config.pic_version != 0)
	{
		dronePicHardVersion.text = [NSString stringWithFormat:@"%.1f", (float)(ardrone_control_config.pic_version >> 24)];
		dronePicSoftVersion.text = [NSString stringWithFormat:@"%d.%d", (int)((ardrone_control_config.pic_version & 0xFFFFFF) >> 16),(int)(ardrone_control_config.pic_version & 0xFFFF)];
	}
	else
	{
		dronePicHardVersion.text = @"none";
		dronePicSoftVersion.text = @"none";
	}
	
	// update AR.Drone software version 
	if(strcmp(ardrone_control_config.num_version_soft, "") != 0)
		droneSoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.num_version_soft];
	else
		droneSoftVersion.text = @"none";
	
	// update AR.Drone hardware version 
	if(ardrone_control_config.num_version_mb != 0)
		droneHardVersion.text = [NSString stringWithFormat:@"%x.0", ardrone_control_config.num_version_mb];
	else
		droneHardVersion.text = @"none";

	// Update motor 1 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor1_soft, "") != 0)
		droneMotor1SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor1_soft];
	else
		droneMotor1SoftVersion.text = [NSString stringWithString:@"none"];

	if(strcmp(ardrone_control_config.motor1_hard, "") != 0)
		droneMotor1HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor1_hard];
	else
		droneMotor1HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor1_supplier, "") != 0)
		droneMotor1SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor1_supplier];
	else
		droneMotor1SupplierVersion.text = [NSString stringWithString:@"none"];
	
	// Update motor 2 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor2_soft, "") != 0)
		droneMotor2SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor2_soft];
	else
		droneMotor2SoftVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor2_hard, "") != 0)
		droneMotor2HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor2_hard];
	else
		droneMotor2HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor2_supplier, "") != 0)
		droneMotor2SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor2_supplier];
	else
		droneMotor2SupplierVersion.text = [NSString stringWithString:@"none"];
	
	// Update motor 3 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor3_soft, "") != 0)
		droneMotor3SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor3_soft];
	else
		droneMotor3SoftVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor3_hard, "") != 0)
		droneMotor3HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor3_hard];
	else
		droneMotor3HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor3_supplier, "") != 0)
		droneMotor3SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor3_supplier];
	else
		droneMotor3SupplierVersion.text = [NSString stringWithString:@"none"];
	
	// Update motor 4 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor4_soft, "") != 0)
		droneMotor4SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor4_soft];
	else
		droneMotor4SoftVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor4_hard, "") != 0)
		droneMotor4HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor4_hard];
	else
		droneMotor4HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor4_supplier, "") != 0)
		droneMotor4SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor4_supplier];
	else
		droneMotor4SupplierVersion.text = [NSString stringWithString:@"none"];

	[self refresh];
}

- (BOOL)isAceControlLevel
{
	return ((CONTROL_LEVEL)ardrone_control_config.control_level == CONTROL_LEVEL_ACE);
}

- (void)refresh
{
	if(logsSwitch.on == YES)
	{
		clearLogsButton.enabled = NO;
		clearLogsButton.alpha = 0.50;
	}
	else 
	{
		clearLogsButton.enabled = YES;
		clearLogsButton.alpha = 1.0;
	}

	yawSpeedCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", yawSpeedSlider.value];
	verticalSpeedCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", verticalSpeedSlider.value];
	droneTiltCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", droneTiltSlider.value];
	iPhoneTiltCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", iPhoneTiltSlider.value];
	droneTrimYawCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", ((droneTrimYawSlider.value < -0.01) || (droneTrimYawSlider.value > 0.01)) ? droneTrimYawSlider.value : 0.0];
	droneTrimPitchCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", ((droneTrimPitchSlider.value < -0.01) || (droneTrimPitchSlider.value > 0.01)) ? droneTrimPitchSlider.value : 0.0];
	droneTrimRollCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", ((droneTrimRollSlider.value < -0.01) || (droneTrimRollSlider.value > 0.01)) ? droneTrimRollSlider.value : 0.0];
	
	droneTrimRollSlider.enabled = droneTrimPitchSlider.enabled = droneTrimYawSlider.enabled = ((TRIM_TYPE)trimSegmentedControl.selectedSegmentIndex == TRIM_TYPE_MANUAL);
	droneTrimRollSlider.alpha = droneTrimPitchSlider.alpha = droneTrimYawSlider.alpha = (((TRIM_TYPE)trimSegmentedControl.selectedSegmentIndex == TRIM_TYPE_MANUAL) ? 1.0 : 0.5);
}

- (IBAction)valueChanged:(id)sender
{
	if(sender == enemyColorsSegmentedControl)
	{
		int value = enemyColorsSegmentedControl.selectedSegmentIndex + 1;
		ardrone_control_config.enemy_colors = value; 
		ardrone_at_set_enemy_colors(value);
	}
	else if(sender == controlLevelSegmentedControl)
	{
		int value = controlLevelSegmentedControl.selectedSegmentIndex;
		ardrone_control_config.control_level = value;
		ardrone_at_set_control_level((CONTROL_LEVEL)value);
	}
	else if(sender == altituteLimitedSwitch)
	{
		uint32_t value = (altituteLimitedSwitch.on == YES) ? ALTITUDE_LIMITED : NO_ALTITUDE;
		ardrone_control_config.altitude_max = value;
		ardrone_at_set_altitude_max(value);
	}
	else if(sender == outdoorFlightSwitch)
	{
		bool_t enabled = (outdoorFlightSwitch.on == YES);
		ardrone_control_config.outdoor = enabled;
		ardrone_at_set_outdoor(enabled);
	}
	else if(sender == outdoorShellSwitch)
	{
		bool_t enabled = (outdoorShellSwitch.on == YES);
		ardrone_control_config.flight_without_shell = enabled;
		ardrone_at_set_flight_without_shell(enabled);
	}
	else
	{
		[self refresh];		
	}
}

- (IBAction)sliderRelease:(id)sender
{
	if(sender == droneTiltSlider)
	{
		float value = droneTiltSlider.value;
		ardrone_control_config.euler_angle_max = value * DEG_TO_RAD;
		ardrone_at_set_max_angle(value);
	}
	else if(sender == iPhoneTiltSlider)
	{
		float value = iPhoneTiltSlider.value;
		ardrone_control_config.control_iphone_tilt = value * DEG_TO_RAD;
		ardrone_at_set_tilt(value);
	}
	else if(sender == verticalSpeedSlider)
	{
		float value = verticalSpeedSlider.value;
		ardrone_control_config.control_vz_max = value;
		ardrone_at_set_vert_speed(value);
	}
	else if(sender == yawSpeedSlider)
	{
		float value = yawSpeedSlider.value;
		ardrone_control_config.control_yaw = value * DEG_TO_RAD;
		ardrone_at_set_yaw(value);
	}
}

- (IBAction)trimChanged:(id)sender
{
	//	NSLog(@"%s", __FUNCTION__);
	controlData->trim_pitch = -droneTrimPitchSlider.value;
	controlData->trim_roll = droneTrimRollSlider.value;
	controlData->trim_yaw = droneTrimYawSlider.value;

	ardrone_at_set_manual_trims(controlData->trim_pitch, controlData->trim_roll, controlData->trim_yaw);

	[self refresh];
}

- (IBAction)logsChanged:(id)sender
{
	if(logsSwitch.on)
		saveSettings();
	setNavdataDemo(logsSwitch.on == NO);
	[self refresh];
}

#ifdef INTERFACE_WITH_DEBUG
- (IBAction)tvoutChanged:(id)sender
{
	controlData->showInterface = YES; //(tvoutSwitch.on == NO);
	if(tvoutSwitch.on)
		[tvout startTVOut];
	else
		[tvout stopTVOut];	
}
#endif

- (IBAction)trimSelectChanged:(id)sender
{
	if(manualTrimInProgress == NO)
	{
		trimSegmentedControl.enabled = NO;
		trimSegmentedControl.alpha = 0.5;
		manualTrimInProgress = YES;
		switch((TRIM_TYPE)trimSegmentedControl.selectedSegmentIndex)
		{
			case TRIM_TYPE_MANUAL:
			case TRIM_TYPE_AUTO:
				controlData->needSetManualTrim = TRUE;
				controlData->manual_trim = trimSegmentedControl.selectedSegmentIndex;
				break;
				
			default:
				break;
		}
	}
}

- (IBAction)clearLogsButtonClick:(id)sender
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
	if([paths count] > 0)
	{
		NSString *documentsDirectory = [paths objectAtIndex:0];
		NSFileManager *fileManager = [[NSFileManager alloc] init];
		NSArray *documentsDirectoryContents = [fileManager contentsOfDirectoryAtPath:documentsDirectory error:nil];
		for(int i = 0 ; i < [documentsDirectoryContents count] ; i++)
		{
			if([[documentsDirectoryContents objectAtIndex:i] hasPrefix:@"mesures"] || [[documentsDirectoryContents objectAtIndex:i] hasPrefix:@"settings"])
			{
				char filename[256];
				sprintf(filename, "%s/%s", [documentsDirectory cStringUsingEncoding:NSASCIIStringEncoding], [[documentsDirectoryContents objectAtIndex:i] cStringUsingEncoding:NSASCIIStringEncoding]);
				NSLog(@"- Remove %s", filename);
				remove(filename);
			}
		}
		[fileManager release];
	}
}

- (IBAction)clearButtonClick:(id)sender
{
//	NSLog(@"%s", __FUNCTION__);
	ardrone_control_config.euler_angle_max = ardrone_control_config_default.euler_angle_max;
	droneTiltSlider.value = ardrone_control_config.euler_angle_max * RAD_TO_DEG;
	ardrone_at_set_max_angle(droneTiltSlider.value);
	
	ardrone_control_config.control_iphone_tilt = ardrone_control_config_default.control_iphone_tilt;
	iPhoneTiltSlider.value = ardrone_control_config.control_iphone_tilt * RAD_TO_DEG;
	ardrone_at_set_tilt(iPhoneTiltSlider.value);
	
	ardrone_control_config.control_vz_max = ardrone_control_config_default.control_vz_max;
	verticalSpeedSlider.value = ardrone_control_config.control_vz_max;
	ardrone_at_set_vert_speed(verticalSpeedSlider.value);
	
	ardrone_control_config.control_yaw = ardrone_control_config_default.control_yaw;
	yawSpeedSlider.value = ardrone_control_config.control_yaw * RAD_TO_DEG;
	ardrone_at_set_yaw(yawSpeedSlider.value);
	
	ardrone_control_config.flight_without_shell = ardrone_control_config_default.flight_without_shell;
	outdoorShellSwitch.on = (ardrone_control_config.flight_without_shell ? YES : NO);
	ardrone_at_set_flight_without_shell(ardrone_control_config.flight_without_shell);

	ardrone_control_config.outdoor = ardrone_control_config_default.outdoor;
	outdoorFlightSwitch.on = (ardrone_control_config.outdoor ? YES : NO);
	ardrone_at_set_outdoor(ardrone_control_config.outdoor);
	
	ardrone_control_config.altitude_max = ardrone_control_config_default.altitude_max;
	altituteLimitedSwitch.on = ((ardrone_control_config.altitude_max == ALTITUDE_LIMITED) ? YES : NO);
	ardrone_at_set_altitude_max(ardrone_control_config.altitude_max);

	ardrone_control_config.enemy_colors = ardrone_control_config_default.enemy_colors;
	enemyColorsSegmentedControl.selectedSegmentIndex = (ardrone_control_config.enemy_colors - 1);
	ardrone_at_set_enemy_colors(ardrone_control_config.enemy_colors);
	
	ardrone_control_config.control_level = ardrone_control_config_default.control_level;
	controlLevelSegmentedControl.selectedSegmentIndex = ardrone_control_config.control_level;	
	ardrone_at_set_control_level((CONTROL_LEVEL)ardrone_control_config.control_level);
}

- (IBAction)flatTrimButtonClick:(id)sender
{
//	NSLog(@"%s", __FUNCTION__);
	ardrone_at_set_flat_trim();
}

- (IBAction)okButtonClick:(id)sender
{
//	NSLog(@"%s", __FUNCTION__);
	trimSegmentedControl.selectedSegmentIndex = TRIM_TYPE_AUTO;
	self.view.hidden = YES;
}

- (void)switchDisplay
{
	self.view.hidden = !self.view.hidden;
}

- (void)textFieldDidBeginEditing:(UITextField *)theTextField           // became first responder
{
//	NSLog(@"%s", __FUNCTION__);
	if(theTextField == droneSSIDTextField)
	{
		ssidChangeInProgress = NO;
	}
}
- (BOOL)textFieldShouldReturn:(UITextField *)theTextField 
{
//	NSLog(@"%s", __FUNCTION__);
	if(theTextField == droneSSIDTextField)
	{
		NSString *str = [NSString stringWithFormat:@"Your changes will be applied after rebooting the AR.Drone !\n" \
						 "\t- Quit application\n" \
						 "\t- Reboot your AR.Drone\n" \
						 "\t- Connect your iPhone on %s network\n" \
						 "\t- Launch application\n", [droneSSIDTextField.text cStringUsingEncoding:NSASCIIStringEncoding]];
		strcpy(ardrone_control_config.ssid_single_player, [droneSSIDTextField.text cStringUsingEncoding:NSASCIIStringEncoding]);
		ardrone_at_set_network_ssid(ardrone_control_config.ssid_single_player);
		
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"AR.Drone network SSID" message:str delegate:self cancelButtonTitle:@"Ok" otherButtonTitles:nil, nil];
		[alert show];
		[alert release];
	}

	[theTextField resignFirstResponder];
	return YES;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	NSLog(@"%s", __FUNCTION__);
	ssidChangeInProgress = YES;
}

- (void)dealloc
{
#ifdef INTERFACE_WITH_DEBUG
	[tvout release];
#endif
	[super dealloc];	
}
@end

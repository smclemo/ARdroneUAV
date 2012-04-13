#import "HUD.h"

// The radius of the touchable region.
#define JOYSTICK_RADIUS 64.0f

// How far from the center should the thumb be allowed to move?
// Used only for visual feedback not for velocity calculations.
#define THUMB_RADIUS 48.0f

// Ratio to activate control yaw and gaz
#define CONTROL_YAW_RATIO	(1 / 3.0)
#define CONTROL_GAZ_RATIO	(1 / 3.0)

#define ACCELERO_THRESHOLD	0.1
#define ACCELERO_NORM_THRESHOLD	0.0
#define ACCELERO_FASTMOVE_THRESHOLD	1.3

// Determine if a point within the boundaries of the joystick.
static bool_t isPointInCircle(CGPoint point, CGPoint center, float radius) {
	float dx = (point.x - center.x);
	float dy = (point.y - center.y);
	return (radius >= sqrt( (dx * dx) + (dy * dy) ));
}

static inline float sign(float value)
{
	float result = 1.0;
	if(value < 0)
		result = -1.0;
	
	return result;
}

static inline float Normalize(float x, float y, float z)
{
	return sqrt(x * x + y * y + z * z);
}

static inline float Clamp(float v, float min, float max)
{
	float result = v;
	if(v > max)
		result = max;
	else if(v < min)
		result = min;

	return result;
}

@interface HUD ()
ControlData *controlData;
ARDroneHUDConfiguration config;
BOOL _isAceControlLevel;
int speed;
int altitude;
float accelero[3];
float accelero_rotation[3][3];
UIAccelerationValue lastX, lastY, lastZ;
double lowPassFilterConstant, highPassFilterConstant;
BOOL accelerometer_started;
BOOL joystickRightButtonPress, joystickLeftButtonPress, leftButtonPress, rightButtonPress, upButtonPress, downButtonPress;
CGPoint joystickRightCurrentPosition, joystickRightInitialPosition;
CGPoint joystickLeftCurrentPosition, joystickLeftInitialPosition;
CGPoint velocity;
CGPoint rightCenter;
float tmp_phi, tmp_theta;
BOOL screenOrientationRight;
float alpha;
SystemSoundID plop_id;
FILE *mesures_file;

#if TARGET_IPHONE_SIMULATOR == 0
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;
#else
- (void)simulate_accelerometer:(id)sender;
#endif
- (void)setAcceleroRotationWithPhi:(float)phi withTheta:(float)theta withPsi:(float)psi;
- (void)refreshJoystickRight;
- (void)updateVelocity:(CGPoint)point;
- (void)refreshAltitude;
- (void)refreshSpeed;
- (void)hideInfos;
- (void)showInfos;
- (void)refreshControlInterface;
- (void)refreshFingerPrint:(CGPoint)point;
@end

@implementation HUD
@synthesize firePressed;
@synthesize settingsPressed;
@synthesize mainMenuPressed;

- (id)initWithFrame:(CGRect)frame withHUDConfiguration:(ARDroneHUDConfiguration)hudconfiguration withControlData:(ControlData*)data
{
	NSLog(@"HUD frame => w : %f, h : %f", data, frame.size.width, frame.size.height);
	if(self = [super initWithNibName:@"HUD" bundle:nil])
	{
		controlData = data;
		firePressed = NO;
		settingsPressed = NO;
		mainMenuPressed = NO;
		
		vp_os_memcpy(&config, &hudconfiguration, sizeof(ARDroneHUDConfiguration));
		
		_isAceControlLevel = YES;
		altitude = 0;
		speed = 0;
		rightCenter = CGPointZero;		
		lowPassFilterConstant = 0.2;
		highPassFilterConstant = (1.0 / 5.0) / ((1.0 / kAPS) + (1.0 / 5.0));
		
		joystickRightButtonPress = joystickLeftButtonPress = leftButtonPress = rightButtonPress = upButtonPress = downButtonPress = NO;
		joystickRightInitialPosition = CGPointZero;
		joystickRightCurrentPosition = CGPointZero;
		joystickLeftInitialPosition = CGPointZero;
		joystickLeftCurrentPosition = CGPointZero;
		
		accelero[0] = 0.0;
		accelero[1] = 0.0;
		accelero[2] = 0.0;
		
		tmp_phi = 0.0;
		tmp_theta = 0.0;
		
		[self setAcceleroRotationWithPhi:0.0 withTheta:0.0 withPsi:0.0];
		
		alpha = 1.0;
		
		accelerometer_started = NO;
#if TARGET_IPHONE_SIMULATOR == 1
		[NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0 / kAPS) target:self selector:@selector(simulate_accelerometer:) userInfo:nil repeats:YES];
#else
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval: (NSTimeInterval)(1.0 / kAPS)];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];
#endif		
		
#ifdef WRITE_DEBUG_ACCELERO	
		char filename[128];
		NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES) objectAtIndex:0];
		sprintf(filename, "%s/accelero_iphones_mesures.txt",
				[documentsDirectory cStringUsingEncoding:NSASCIIStringEncoding]);
		mesures_file = fopen(filename, "wb");
		fprintf(mesures_file, "ARDrone\nNumSamples;AccEnable;AccXbrut;AccYbrut;AccZbrut;AccX;AccY;AccZ;PhiAngle;ThetaAngle;AlphaAngle\n");
#endif
	}

	return self;
}

- (void) setAcceleroRotationWithPhi:(float)phi withTheta:(float)theta withPsi:(float)psi
{	
	accelero_rotation[0][0] = cosf(psi)*cosf(theta);
	accelero_rotation[0][1] = -sinf(psi)*cosf(phi) + cosf(psi)*sinf(theta)*sinf(phi);
	accelero_rotation[0][2] = sinf(psi)*sinf(phi) + cosf(psi)*sinf(theta)*cosf(phi);
	accelero_rotation[1][0] = sinf(psi)*cosf(theta);
	accelero_rotation[1][1] = cosf(psi)*cosf(phi) + sinf(psi)*sinf(theta)*sinf(phi);
	accelero_rotation[1][2] = -cosf(psi)*sinf(phi) + sinf(psi)*sinf(theta)*cosf(phi);
	accelero_rotation[2][0] = -sinf(theta);
	accelero_rotation[2][1] = cosf(theta)*sinf(phi);
	accelero_rotation[2][2] = cosf(theta)*cosf(phi);

#ifdef WRITE_DEBUG_ACCELERO	
	NSLog(@"Accelero rotation matrix changed :"); 
	NSLog(@"%0.1f %0.1f %0.1f", accelero_rotation[0][0], accelero_rotation[0][1], accelero_rotation[0][2]);
	NSLog(@"%0.1f %0.1f %0.1f", accelero_rotation[1][0], accelero_rotation[1][1], accelero_rotation[1][2]);
	NSLog(@"%0.1f %0.1f %0.1f", accelero_rotation[2][0], accelero_rotation[2][1], accelero_rotation[2][2]);	
#endif
}

#if TARGET_IPHONE_SIMULATOR == 1
- (void)simulate_accelerometer:(id)sender
{
	bool_t acceleroIsActive = FALSE;
	controlData->accelero_phi = 0.0;
	controlData->accelero_theta =  0.0;	
	sendControls(acceleroIsActive, leftButtonPress == YES, rightButtonPress == YES, upButtonPress == YES, downButtonPress == YES);	
}
#else
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{		
	static double highPassFilterX = 0.0, highPassFilterY = 0.0, highPassFilterZ = 0.0;
	bool_t acceleroIsActive = FALSE;
	float norm, alpha;
	float accelerox, acceleroy, acceleroz;
	highPassFilterX = highPassFilterConstant * (highPassFilterX + acceleration.x - lastX);
	highPassFilterY = highPassFilterConstant * (highPassFilterY + acceleration.y - lastY);
	highPassFilterZ = highPassFilterConstant * (highPassFilterZ + acceleration.z - lastZ);
	lastX = acceleration.x;
	lastY = acceleration.y;
	lastZ = acceleration.z;
	//printf("=======> %p\n", controlData);
	if(fabs(highPassFilterX) > ACCELERO_FASTMOVE_THRESHOLD || 
	   fabs(highPassFilterY) > ACCELERO_FASTMOVE_THRESHOLD || 
	   fabs(highPassFilterZ) > ACCELERO_FASTMOVE_THRESHOLD)
	{
		firePressed = YES;
	}
	else
	{
		firePressed = NO;
				
		if(screenOrientationRight)
		{
			accelerox = acceleration.x;
			acceleroy = -acceleration.y;
		}
		else
		{
			accelerox = -acceleration.x;
			acceleroy = acceleration.y;
		}
		acceleroz = -acceleration.z;
		
		// Initialize previous values of acceleros
		if(accelerometer_started == NO)
		{
			accelerometer_started = YES;
			accelero[0] = accelerox;
			accelero[1] = acceleroy;
			accelero[2] = acceleroz;
		}		

		// Apply low pass filter on acceleros
		accelero[0] = accelerox * lowPassFilterConstant + accelero[0] * (1.0 - lowPassFilterConstant);
		accelero[1] = acceleroy * lowPassFilterConstant + accelero[1] * (1.0 - lowPassFilterConstant);
		accelero[2] = acceleroz * lowPassFilterConstant + accelero[2] * (1.0 - lowPassFilterConstant);

		// Apply rotation matrix and Clamp
		accelerox = Clamp((accelero_rotation[0][0] * accelero[0]) + (accelero_rotation[0][1] * accelero[1]) + (accelero_rotation[0][2] * accelero[2]), -1.0, 1.0);
		acceleroy = Clamp((accelero_rotation[1][0] * accelero[0]) + (accelero_rotation[1][1] * accelero[1]) + (accelero_rotation[1][2] * accelero[2]), -1.0, 1.0);
		acceleroz = Clamp((accelero_rotation[2][0] * accelero[0]) + (accelero_rotation[2][1] * accelero[1]) + (accelero_rotation[2][2] * accelero[2]), -1.0, 1.0);
		
		// compute
		if(fabs(acceleroy) < ACCELERO_THRESHOLD)
		{
			if(accelerox >= 0.0)
				alpha = M_PI_2;
			else	
				alpha = -M_PI_2;
		}
		else
		{
			alpha = atan2(accelerox, acceleroy);
		}
		
		norm = Normalize(accelerox, acceleroy, 0.0);
		if(norm > ACCELERO_NORM_THRESHOLD)
		{
			float tmp = (norm - ACCELERO_NORM_THRESHOLD) * (norm - ACCELERO_NORM_THRESHOLD);
			controlData->accelero_phi = tmp * cosf(alpha);
			controlData->accelero_theta = -tmp * sinf(alpha);
		}
		else
		{
			controlData->accelero_phi = 0.0;
			controlData->accelero_theta = 0.0;		
		}
		
		if(_isAceControlLevel)
			acceleroIsActive = (joystickRightButtonPress == YES);
		else 
			acceleroIsActive = (joystickLeftButtonPress == YES);
		
#ifdef WRITE_DEBUG_ACCELERO
		static int numsamples = 0;
		fprintf(mesures_file, "%d;%d;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f;%0.2f\n", numsamples++, (int)acceleroIsActive, accelero[0], accelero[1], accelero[2], accelerox, acceleroy, acceleroz, controlData->accelero_phi * 180.0 / M_PI, controlData->accelero_theta * 180.0 / M_PI, (float)alpha  * 180.0 / M_PI);
#endif
		
		sendControls(acceleroIsActive, leftButtonPress == YES, rightButtonPress == YES, upButtonPress == YES, downButtonPress == YES);	
	}
}
#endif

- (void)refreshJoystickRight
{
	CGRect frame = joystickRightBackgroundImageView.frame;
	frame.origin = joystickRightCurrentPosition;
	joystickRightBackgroundImageView.frame = frame;
}    

- (void)refreshJoystickLeft
{
	CGRect frame = joystickLeftThumbImageView.frame;
	frame.origin = joystickLeftCurrentPosition;
	joystickLeftThumbImageView.frame = frame;
}    

- (void)refreshControlInterface
{
	UIImage *joystick_background, *joystick_thumb;
	CGRect frame;
	
	if(_isAceControlLevel)
	{
		joystickLeftButton.hidden = YES;
		joystickLeftThumbImageView.hidden = YES;
		joystickLeftButton.enabled = NO;
		joystick_background = [UIImage imageNamed:@"joystick_background.png"];
		joystick_thumb = [UIImage imageNamed:@"joystick_thumb.png"];
		frame = joystickRightBackgroundImageView.frame;
		frame.size = joystick_background.size;
		joystickRightBackgroundImageView.frame = frame;
	}
	else 
	{
		joystickLeftButton.hidden = controlData->showInterface ? NO : YES;
		joystickLeftThumbImageView.hidden = controlData->showInterface ? NO : YES;
		joystickLeftButton.enabled = YES;
		joystick_background = [UIImage imageNamed:@"joystick_background.png"];
		joystick_thumb = [UIImage imageNamed:@"joystick_thumb.png"];
		frame = joystickRightBackgroundImageView.frame;
		frame.size = joystick_background.size;
		joystickRightBackgroundImageView.frame = frame;
	}
	
	[joystickRightBackgroundImageView setImage:joystick_background];
	[joystickRightThumbImageView setImage:joystick_thumb];
	
	joystickRightInitialPosition = CGPointMake(rightCenter.x - (joystickRightBackgroundImageView.frame.size.width / 2), rightCenter.y - (joystickRightBackgroundImageView.frame.size.height / 2));
	joystickRightCurrentPosition = joystickRightInitialPosition;
	[self refreshJoystickRight];
}
	
- (void)changeControlInterface:(BOOL)isAceControlLevel
{
	if(_isAceControlLevel != isAceControlLevel)
	{
		_isAceControlLevel = isAceControlLevel;
		[self performSelectorOnMainThread:@selector(refreshControlInterface) withObject:nil waitUntilDone:YES];
	}
}
	
- (void)updateVelocity:(CGPoint)point 
{
	CGPoint nextpoint = CGPointMake(point.x, point.y);
	
	// Calculate distance and angle from the center.
	float dx = nextpoint.x - rightCenter.x;
	float dy = nextpoint.y - rightCenter.y;
	
	float distance = sqrt(dx * dx + dy * dy);
	float angle = atan2(dy, dx); // in radians
	
	// NOTE: Velocity goes from -1.0 to 1.0.
	// BE CAREFUL: don't just cap each direction at 1.0 since that
	// doesn't preserve the proportions.
	if (distance > JOYSTICK_RADIUS) {
		dx = cos(angle) * JOYSTICK_RADIUS;
		dy = sin(angle) *  JOYSTICK_RADIUS;
	}
	
	velocity = CGPointMake(dx / JOYSTICK_RADIUS, dy /JOYSTICK_RADIUS);
	
	// Constrain the thumb so that it stays within the joystick
	// boundaries.  This is smaller than the joystick radius in
	// order to account for the size of the thumb.
	if (distance > THUMB_RADIUS) {
		nextpoint.x = rightCenter.x + cos(angle) * THUMB_RADIUS;
		nextpoint.y = rightCenter.y + sin(angle) * THUMB_RADIUS;
	}
	
	// Update the thumb's position
	CGRect frame = joystickRightThumbImageView.frame;
	frame.origin.x = nextpoint.x - (joystickRightThumbImageView.frame.size.width / 2);
	frame.origin.y = nextpoint.y - (joystickRightThumbImageView.frame.size.height / 2);	
	joystickRightThumbImageView.frame = frame;
}
	
- (void)setMessageBox:(NSString*)str
{
	[messageBoxLabel performSelectorOnMainThread:@selector(setText:) withObject:str waitUntilDone:YES];
}

- (void)setTakeOff:(NSString*)str
{
	UIImage *image = [UIImage imageNamed:str];
	[takeOffButton setImage:image forState:UIControlStateNormal];
}

- (void)setEmergency:(NSString*)str
{
	UIImage *image = [UIImage imageNamed:str];
	[emergencyButton setImage:image forState:UIControlStateNormal];
}

- (void)hideInfos
{
	batteryLevelLabel.hidden = YES;
	batteryImageView.hidden = YES;	
	wifiImageView.hidden = YES;
}

- (void)showInfos
{
	batteryLevelLabel.hidden = NO;
	batteryImageView.hidden = NO;
	wifiImageView.hidden = NO;
}

- (void)setBattery:(int)percent
{
	if(percent < 0 || !controlData->showInterface)
	{
		[self performSelectorOnMainThread:@selector(hideInfos) withObject:nil waitUntilDone:YES];		
	}
	else
	{
		UIImage *image = [UIImage imageNamed:[NSString stringWithFormat:@"battery%d.png", ((percent < 10) ? 0 : (int)((percent / 33.4) + 1))]];
		[batteryImageView performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:YES];
		
		[self performSelectorOnMainThread:@selector(showInfos) withObject:nil waitUntilDone:YES];		
		
		[batteryLevelLabel performSelectorOnMainThread:@selector(setTextColor:) withObject:((percent < 10) ? [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.7] : [UIColor colorWithRed:0.0 green:0.65 blue:0.0 alpha:0.7]) waitUntilDone:YES];		
		[batteryLevelLabel performSelectorOnMainThread:@selector(setText:) withObject:[NSString stringWithFormat:@"%d %%", percent] waitUntilDone:YES];
	}
}

- (void)setWifi:(int)percent
{
	
}

- (void)setSpeed:(int)percent
{
	speed = percent;
	if(speed < 0)
		speed = 0;
	
	if (speed > 100)
		speed = 100;
	
	[self performSelectorOnMainThread:@selector(refreshSpeed) withObject:nil waitUntilDone:YES];
}

- (void)refreshSpeed
{

}

- (void)setAltitude:(int)percent
{
	altitude = percent;
	if(altitude < 0)
		altitude = 0;
	
	if (altitude > 100)
		altitude = 100;
	
	[self performSelectorOnMainThread:@selector(refreshAltitude) withObject:nil waitUntilDone:YES];
}

- (void)refreshAltitude
{
}

- (void)refreshInterface
{
	static bool_t old_showInterface = FALSE;
	if(old_showInterface != controlData->showInterface)
	{
		messageBoxLabel.alpha = controlData->showInterface ? 1.0 : 0.0;
		joystickRightThumbImageView.alpha = controlData->showInterface ? alpha : 0.0;
		joystickRightBackgroundImageView.alpha = controlData->showInterface ? alpha : 0.0;
		joystickLeftThumbImageView.alpha = controlData->showInterface ? alpha : 0.0;
		backToMainMenuButton.alpha = controlData->showInterface ? 1.0 : 0.0;
		settingsButton.alpha = controlData->showInterface ? 1.0 : 0.1;
		switchScreenButton.alpha = controlData->showInterface ? 1.0 : 0.1;
		takeOffButton.alpha = controlData->showInterface ? 1.0 : 0.1;
		emergencyButton.alpha = controlData->showInterface ? 1.0 : 0.1;
		old_showInterface = controlData->showInterface;
	}
}

- (IBAction)buttonPress:(id)sender forEvent:(UIEvent *)event 
{
	if(sender == joystickRightButton)
	{
		UITouch *touch = [[event touchesForView:joystickRightButton] anyObject];
		CGPoint point = [touch locationInView:self.view];
		
		// Start only if the first touch is within the pad's boundaries.
		// Allow touches to be tracked outside of the pad as long as the
		// screen continues to be pressed.
		BOOL joystickIsOutside =  ((point.x + (joystickRightBackgroundImageView.frame.size.width / 2) > (joystickRightButton.frame.origin.x + joystickRightButton.frame.size.width)) ||
								   (point.x - (joystickRightBackgroundImageView.frame.size.width / 2) < joystickRightButton.frame.origin.x) ||
								   (point.y + (joystickRightBackgroundImageView.frame.size.height / 2) > (joystickRightButton.frame.origin.y + joystickRightButton.frame.size.height)) ||
								   (point.y - (joystickRightBackgroundImageView.frame.size.height / 2) < joystickRightButton.frame.origin.y));
		
		if(joystickIsOutside)
		{
			AudioServicesPlaySystemSound(plop_id);
		}
		
		joystickRightButtonPress = YES;

		if(_isAceControlLevel)
		{
			float phi, theta;
			phi = (float)atan2f(accelero[1], accelero[2]);
			theta = -(float)atan2f(accelero[0] * cosf(phi), accelero[2]);
			
			[self setAcceleroRotationWithPhi:phi withTheta:theta withPsi:0.0];
		}
		
		joystickRightCurrentPosition.x = point.x - (joystickRightBackgroundImageView.frame.size.width / 2);
		joystickRightCurrentPosition.y = point.y - (joystickRightBackgroundImageView.frame.size.height / 2);
		
		joystickRightBackgroundImageView.alpha = joystickRightThumbImageView.alpha = (controlData->showInterface ? 1.0 : 0.0);

		// Refresh Joystick
		[self refreshJoystickRight];
		
		// Update center
		rightCenter = CGPointMake(joystickRightBackgroundImageView.frame.origin.x + (joystickRightBackgroundImageView.frame.size.width / 2), joystickRightBackgroundImageView.frame.origin.y + (joystickRightBackgroundImageView.frame.size.height / 2));
	
		// Update velocity
		[self updateVelocity:rightCenter];
	}
	else if(!_isAceControlLevel && (sender == joystickLeftButton))
	{
		// Start only if the first touch is within the pad's boundaries.
		// Allow touches to be tracked outside of the pad as long as the
		// screen continues to be pressed.			
		UITouch *touch = [[event touchesForView:joystickLeftButton] anyObject];
		CGPoint point = [touch locationInView:self.view];

		joystickLeftButtonPress = YES;
		
		if(!_isAceControlLevel)
		{
			float phi, theta;
			phi = (float)atan2f(accelero[1], accelero[2]);
			theta = -(float)atan2f(accelero[0] * cosf(phi), accelero[2]);
			[self setAcceleroRotationWithPhi:phi withTheta:theta withPsi:0.0];
		}
				
		joystickLeftThumbImageView.alpha = (controlData->showInterface ? 1.0 : 0.0);
	
		joystickLeftCurrentPosition.x = point.x - (joystickLeftThumbImageView.frame.size.width / 2);
		joystickLeftCurrentPosition.y = point.y - (joystickLeftThumbImageView.frame.size.height / 2);
		
		[self refreshJoystickLeft];
	}
}

- (IBAction)buttonRelease:(id)sender forEvent:(UIEvent *)event 
{
	if(sender == joystickRightButton)
	{
		if(joystickRightButtonPress)
		{			
			if(_isAceControlLevel)
			{
				[self setAcceleroRotationWithPhi:0.0 withTheta:0.0 withPsi:0.0];
			}
			
			// Reinitialize boolean value
			joystickRightButtonPress = leftButtonPress = rightButtonPress = upButtonPress = downButtonPress = NO;
			
			// Reinitialize progressive command
			inputYaw(0.0);
			inputGaz(0.0);
			// Reinitialize joystick position
			joystickRightCurrentPosition = joystickRightInitialPosition;
			joystickRightBackgroundImageView.alpha = joystickRightThumbImageView.alpha = (controlData->showInterface ? alpha : 0.0);
			
			// Refresh joystick
			[self refreshJoystickRight];
			
			// Update center
			rightCenter = CGPointMake(joystickRightBackgroundImageView.frame.origin.x + (joystickRightBackgroundImageView.frame.size.width / 2), joystickRightBackgroundImageView.frame.origin.y + (joystickRightBackgroundImageView.frame.size.height / 2));
			
			// reset joystick
			[self updateVelocity:rightCenter];
		}		
	}
	else if(!_isAceControlLevel && (sender == joystickLeftButton))
	{
		if(joystickLeftButtonPress)
		{
			joystickLeftButtonPress = NO;
					
			[self setAcceleroRotationWithPhi:0.0 withTheta:0.0 withPsi:0.0];
			
			// Reinitialize joystick position
			joystickLeftThumbImageView.alpha = (controlData->showInterface ? alpha : 0.0);

			joystickLeftCurrentPosition = joystickLeftInitialPosition;
			
			[self refreshJoystickLeft];
		}		
	}
}

- (IBAction)buttonDrag:(id)sender forEvent:(UIEvent *)event 
{
	if(sender == joystickRightButton)
	{
		UITouch *touch = [[event touchesForView:joystickRightButton] anyObject];
		CGPoint point = [touch locationInView:self.view];
	
		if (joystickRightButtonPress)
		{
			// input left if necessary
			leftButtonPress = ((rightCenter.x - point.x) > ((joystickRightBackgroundImageView.frame.size.width / 2) - (CONTROL_YAW_RATIO * joystickRightBackgroundImageView.frame.size.width)));

			// input right if necessary
			rightButtonPress = ((point.x - rightCenter.x) > ((joystickRightBackgroundImageView.frame.size.width / 2) - (CONTROL_YAW_RATIO * joystickRightBackgroundImageView.frame.size.width)));
			
			// input up if necessary
			upButtonPress = ((rightCenter.y - point.y) > ((joystickRightBackgroundImageView.frame.size.height / 2) - (CONTROL_GAZ_RATIO * joystickRightBackgroundImageView.frame.size.height)));
		
			// input down if necessary
			downButtonPress = ((point.y - rightCenter.y) > ((joystickRightBackgroundImageView.frame.size.height / 2) - (CONTROL_GAZ_RATIO * joystickRightBackgroundImageView.frame.size.height)));
		
			// Update joystick velocity
			[self updateVelocity:point];
		
			if(leftButtonPress)
			{
				float percent = ((rightCenter.x - point.x) - ((joystickRightBackgroundImageView.frame.size.width / 2) - (CONTROL_YAW_RATIO * joystickRightBackgroundImageView.frame.size.width))) / ((CONTROL_YAW_RATIO * joystickRightBackgroundImageView.frame.size.width));
				if(percent > 1.0)
					percent = 1.0;
				inputYaw(-percent);
			}
			else if(rightButtonPress)
			{
				float percent = ((point.x - rightCenter.x) - ((joystickRightBackgroundImageView.frame.size.width / 2) - (CONTROL_YAW_RATIO * joystickRightBackgroundImageView.frame.size.width))) / ((CONTROL_YAW_RATIO * joystickRightBackgroundImageView.frame.size.width));
				if(percent > 1.0)
					percent = 1.0;
				inputYaw(percent);
			}
			else
			{
				inputYaw(0.0);
			}	
			
			if(downButtonPress)
			{
				float percent = ((point.y - rightCenter.y) - ((joystickRightBackgroundImageView.frame.size.height / 2) - (CONTROL_GAZ_RATIO * joystickRightBackgroundImageView.frame.size.height))) / ((CONTROL_GAZ_RATIO * joystickRightBackgroundImageView.frame.size.height));
				if(percent > 1.0)
					percent = 1.0;
				inputGaz(-percent);
			}
			else if(upButtonPress)
			{
				float percent = ((rightCenter.y - point.y) - ((joystickRightBackgroundImageView.frame.size.height / 2) - (CONTROL_GAZ_RATIO * joystickRightBackgroundImageView.frame.size.height))) / ((CONTROL_GAZ_RATIO * joystickRightBackgroundImageView.frame.size.height));
				if(percent > 1.0)
					percent = 1.0;
				inputGaz(percent);
			}
			else
			{
				inputGaz(0.0);
			}
		}
	}
}

- (IBAction)buttonClick:(id)sender forEvent:(UIEvent *)event 
{
	static ARDRONE_VIDEO_CHANNEL channel = ARDRONE_VIDEO_CHANNEL_FIRST;
	if(sender == settingsButton)
		settingsPressed = YES;
	else if(sender == backToMainMenuButton)
		mainMenuPressed = YES;
	else if(sender == switchScreenButton)
	{
		if(channel++ == ARDRONE_VIDEO_CHANNEL_LAST)
			channel = ARDRONE_VIDEO_CHANNEL_FIRST;
		
		switchVideoChannel((int)channel);
		switch(channel)
		{
			case ARDRONE_VIDEO_CHANNEL_HORI:
			case ARDRONE_VIDEO_CHANNEL_VERT_IN_HORI:
				changeCameraDetection(ARDRONE_CAMERA_DETECTION_HORIZONTAL, 14.0);
				break;
				
			case ARDRONE_VIDEO_CHANNEL_VERT:
			case ARDRONE_VIDEO_CHANNEL_HORI_IN_VERT:
				changeCameraDetection(ARDRONE_CAMERA_DETECTION_VERTICAL, 14.0);
				break;
		}
	}
	else if(sender == takeOffButton)
		switchTakeOff();
	else if(sender == emergencyButton)
		emergency();
}

-(void)refreshFingerPrint:(CGPoint)point
{
	CGRect frame = fingerPrintImageView.frame; 
	frame.origin.x = point.x - (frame.size.width / 2);
	frame.origin.y = point.y - (frame.size.height / 2);
	fingerPrintImageView.frame = frame;
}
	
-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{	
	UITouch *touch = [[event allTouches] anyObject];
	fingerPrintImageView.hidden = (controlData->showInterface ? NO : YES);
	[self refreshFingerPrint:[touch locationInView:self.view]]; 
}
	
-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{	
	UITouch *touch = [[event allTouches] anyObject];
	[self refreshFingerPrint:[touch locationInView:self.view]]; 
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{	
	fingerPrintImageView.hidden = YES;
}

-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchesEnded:touches withEvent:event];
}

- (void)viewDidLoad
{
	switchScreenButton.hidden = (config.enableSwitchScreen == NO);
	backToMainMenuButton.hidden = (config.enableBackToMainMenu == NO);
	
	rightCenter = CGPointMake(joystickRightThumbImageView.frame.origin.x + (joystickRightThumbImageView.frame.size.width / 2), joystickRightThumbImageView.frame.origin.y + (joystickRightThumbImageView.frame.size.height / 2));
	joystickRightInitialPosition = CGPointMake(rightCenter.x - (joystickLeftThumbImageView.frame.size.width / 2), rightCenter.y - (joystickLeftThumbImageView.frame.size.height / 2));
	joystickLeftInitialPosition = joystickLeftThumbImageView.frame.origin;
	alpha = MIN(joystickRightBackgroundImageView.alpha, joystickRightThumbImageView.alpha);
	joystickRightBackgroundImageView.alpha = joystickRightThumbImageView.alpha = alpha;

    // Get the URL to the sound file to play
    CFURLRef plop_url  = CFBundleCopyResourceURL (CFBundleGetMainBundle(),
												   CFSTR ("plop"),
												   CFSTR ("wav"),
												   NULL);
	
    // Create a system sound object representing the sound file
    AudioServicesCreateSystemSoundID (plop_url, &plop_id);
	CFRelease(plop_url);
	
	[self setBattery:-1];
	[self setWifi:-1];
	[self setSpeed:50];
	[self setAltitude:0];
}

- (void) dealloc 
{
#ifdef WRITE_DEBUG_ACCELERO
	fclose(mesures_file);
#endif
	AudioServicesDisposeSystemSoundID (plop_id);
	[super dealloc];
}

@end

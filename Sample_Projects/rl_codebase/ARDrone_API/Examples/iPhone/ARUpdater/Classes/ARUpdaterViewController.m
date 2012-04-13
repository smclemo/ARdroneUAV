	//
	//  ARUpdaterViewController.m
	//  AR.Updater
	//
	//  Created by Robert Ryll on 10-05-14.
	//  Copyright Playsoft 2010. All rights reserved.
	//

#import "ARUpdaterViewController.h"
#include "plf.h"

@interface ARUpdaterViewController ()



enum {
	s_connect,
	s_check,
	s_send,
	s_install,
	s_restart,
	s_recheck
} state;

enum {
	ss_user,
	ss_app
} substate;

enum {
	r_none,
	r_progress,
	r_problem,
	r_fail,
	r_pass
};

UILabel *statusLabel;
UIImageView *stepPassImage[STEP_LINE_NR];
UIImageView *stepFailImage;
UIImageView *stepProblemImage;
UIActivityIndicatorView *stepIndicator;
UIButton *sendButton;
UIProgressView *sendProgressView;

NSInteger firmwareSize;
NSInteger sendedSize;
BOOL streamOpen;
BOOL end;

	//@property (nonatomic, retain) ARDrone *drone;

- (void)stateUpdate:(int)newState Result:(int)result Message:(NSString*)message;
- (void)stopSendWithStatus:(NSString *)statusString;
- (void)checkStream;
- (void)sendAction;

@end

char iphone_mac_address[] = "00:00:00:00:00:00";

@implementation ARUpdaterViewController

	//@synthesize drone;

@synthesize firmwarePath;
@synthesize firmwareVersion;
@synthesize firmwareFileName;
@synthesize droneFirmwareVersion, errorLog;
@synthesize localIP;

@synthesize checkRestartTimer;

@synthesize networkStream = _networkStream;
@synthesize fileStream = _fileStream;
@synthesize bufferOffset = _bufferOffset;
@synthesize bufferLimit = _bufferLimit;

#pragma mark init

- (void)loadView {
	CGRect frame = [[UIScreen mainScreen] applicationFrame];
	UIView *main = [[UIView alloc] initWithFrame:frame];
	self.view = main;
	
	NSString *plistPath = [[NSBundle mainBundle] pathForResource:@"Firmware" ofType:@"plist"];
	NSDictionary *plistDict = [NSDictionary dictionaryWithContentsOfFile:plistPath];
	self.firmwareFileName = [plistDict objectForKey:@"FirmwareFileName"];
	//self.firmwareVersion = [plistDict objectForKey:@"FirmwareVersion"];
	
	self.firmwarePath = [[NSBundle mainBundle] pathForResource:firmwareFileName ofType:@"plf"];
	plf_phdr plf_header;

	if(plf_get_header([self.firmwarePath cStringUsingEncoding:NSASCIIStringEncoding], &plf_header) != 0)
		memset(&plf_header, 0, sizeof(plf_phdr));
	
	self.firmwareVersion = [NSString stringWithFormat:@"%d.%d.%d", plf_header.p_ver, plf_header.p_edit, plf_header.p_ext];
	
	NSDictionary *firmwareAttributes = [[NSFileManager defaultManager] fileAttributesAtPath:firmwarePath traverseLink:YES];
	NSNumber *firmwareSizeNumber = [firmwareAttributes objectForKey:NSFileSize];
	firmwareSize = [firmwareSizeNumber unsignedIntegerValue];
	sendedSize = 0;
	
		//drone = [[ARDrone alloc] initWithDelegate:self];
	
	UIImageView *BG = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"BG.png"]];
	[self.view addSubview:BG];
	[BG release];
	
	UIImage *headerImage = [UIImage imageNamed:@"Header.png"];
	UIImageView *header = [[UIImageView alloc] initWithImage:headerImage];
	NSInteger headerWidth = headerImage.size.width / headerImage.size.height * IMG_H;
	header.frame = CGRectMake((SCREEN_W - headerWidth) / 2, LINE_H, headerWidth, IMG_H);
	[self.view addSubview:header];
	[headerImage release];
	[header release];
	
	UILabel *firmwareVersionLabel = [UILabel new];
	firmwareVersionLabel.backgroundColor = [UIColor clearColor];
	firmwareVersionLabel.frame = CGRectMake(0.0, 0.0, SCREEN_W, LINE_H);
	firmwareVersionLabel.textColor = NORMAL_COLOR;
	firmwareVersionLabel.textAlignment = UITextAlignmentRight;
	firmwareVersionLabel.text = [NSString stringWithFormat:@"v%@", firmwareVersion];
	[self.view addSubview:firmwareVersionLabel];
	[firmwareVersionLabel release];
	
	statusLabel = [UILabel new];
	statusLabel.backgroundColor = [UIColor clearColor];
	statusLabel.frame = CGRectMake(0.0, STATUS_Y, SCREEN_W, STATUS_LINE_NR * LINE_H);
	statusLabel.textAlignment = UITextAlignmentCenter;
	statusLabel.textColor = NORMAL_COLOR;
	statusLabel.numberOfLines = STATUS_LINE_NR;
	[self.view addSubview:statusLabel];
	
	NSString *steps[] = {
		NSLocalizedString(@"Connect", @""),
		NSLocalizedString(@"Check", @""),
		NSLocalizedString(@"Send", @""),
		NSLocalizedString(@"Update", @""),
		NSLocalizedString(@"Restart", @""),
		NSLocalizedString(@"Recheck", @"")
	};
	
	UILabel *stepLabel;
	for (NSInteger i = 0; i < STEP_LINE_NR; i++) {
		stepLabel = [UILabel new];
		stepLabel.backgroundColor = [UIColor clearColor];
		stepLabel.textColor = NORMAL_COLOR;
		stepLabel.frame = CGRectMake(LINE_H, STEP_LINE_Y + LINE_H * i, SCREEN_H - LINE_H, LINE_H);
		stepLabel.text = steps[i];
		[self.view addSubview:stepLabel];
		[stepLabel release];
	}
	
	UIImageView *stepEmptyImage;
	for (NSInteger i = 0; i < STEP_LINE_NR; i++) {
		stepEmptyImage = [UIImageView new];
		stepEmptyImage.frame = CGRectMake(0, STEP_LINE_Y + i * LINE_H, LINE_H, LINE_H);
		stepEmptyImage.image = [UIImage imageNamed:@"Empty.png"];
		[self.view addSubview:stepEmptyImage];
		[stepEmptyImage release];
	}
	
	sendButton = [UIButton buttonWithType:UIButtonTypeCustom];
	UIImage *buttonImage = [UIImage imageNamed:@"Button.png"];
	[sendButton setBackgroundImage:buttonImage forState:UIControlStateNormal];
	[sendButton setTitle:NSLocalizedString(@"Send file", @"Button title") forState:UIControlStateNormal];
	[sendButton setTitleColor:NORMAL_COLOR forState:UIControlStateNormal];
	[sendButton addTarget:self action:@selector(sendAction) forControlEvents:UIControlEventTouchDown];
	sendButton.frame = CGRectMake(SCREEN_W - buttonImage.size.width, STEP_LINE_Y + (STEP_LINE_NR * LINE_H - buttonImage.size.height) / 2, buttonImage.size.width, buttonImage.size.height);
	[buttonImage release];
	[self.view addSubview:sendButton];
	
	sendProgressView = [[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleBar];
	sendProgressView.frame = CGRectMake(LINE_H, STEP_LINE_Y - LINE_H, SCREEN_W - LINE_H * 2, LINE_H);
	[self.view addSubview:sendProgressView];
	
	stepFailImage = [UIImageView new];
	stepFailImage.image = [UIImage imageNamed:@"Fail.png"];
	stepFailImage.hidden = YES;
	[self.view addSubview:stepFailImage];		
	
	stepProblemImage = [UIImageView new];
	stepProblemImage.image = [UIImage imageNamed:@"Problem.png"];
	[self.view addSubview:stepProblemImage];		
	
	for (NSInteger i = 0; i < STEP_LINE_NR; i++) {
		stepPassImage[i] = [UIImageView new];
		stepPassImage[i].frame = CGRectMake(0, STEP_LINE_Y + i * LINE_H, LINE_H, LINE_H);
		stepPassImage[i].image = [UIImage imageNamed:@"Pass.png"];
		stepPassImage[i].hidden = YES;
		[self.view addSubview:stepPassImage[i]];
	}
	
	stepIndicator = [[UIActivityIndicatorView new] initWithActivityIndicatorStyle: UIActivityIndicatorViewStyleWhite];
	stepIndicator.hidesWhenStopped = YES;
	[self.view addSubview:stepIndicator];
	
	state = s_connect;
	substate = ss_app;
	end = NO;
	sendButton.hidden = YES;
	sendProgressView.hidden = YES;
	[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Launching with drone", @"")];
	[self performSelector:@selector(initConnection) withObject:nil afterDelay:1];
	//	[self checkRestart:nil];
	droneRestarted = NO;
	retryCounter = 0;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return (interfaceOrientation == UIInterfaceOrientationLandscapeRight || interfaceOrientation == UIInterfaceOrientationLandscapeLeft);
}

#pragma mark flow

-(void) initConnection
{
	self.localIP = [self deviceIPAdress];
	NSLog(@"local ip %@", localIP);
	if(![self checkFTP:@"version.txt"] && retryCounter < 15)
	{
		[self initConnection];
		retryCounter++;
	}
	else if(retryCounter < 15)
	{
		NSLog(@"get version text");
		retryCounter = 0;
		[self getVersionText];
	}
	else
	{
		[self stateUpdate:s_connect Result:r_fail Message:NSLocalizedString(@"Wifi not Reachabled", @"")];
	}
}
		 
- (void)stateUpdate:(int)newState Result:(int)result Message:(NSString*)message {
	statusLabel.text = message;
	stepProblemImage.hidden = YES;
	[stepIndicator stopAnimating];
	statusLabel.textColor = NORMAL_COLOR;
	if (newState < state) {
		for (NSInteger i = newState; i < state; i++) {
			stepPassImage[i].hidden = YES;
		}
	}
	if (newState > state) {
		stepPassImage[state].hidden = NO;
	}
	state = newState;
	switch (result) {
		case r_none:
			if (state == s_send) {
				substate = ss_user;
				sendButton.hidden = NO;
			}
			break;
		case r_progress:
			stepIndicator.frame = CGRectMake(INDICATOR_M, STEP_LINE_Y + state * LINE_H + INDICATOR_M, INDICATOR_S, INDICATOR_S);
			[stepIndicator startAnimating];
			if (state == s_send) {
				substate = ss_app;
				sendButton.hidden = YES;
				sendProgressView.hidden = NO;
				[sendProgressView setProgress:0.0];
				sendedSize = 0;
			}
			if (state == s_install) {
				sendProgressView.hidden = YES;
			}
			break;
		case r_problem:
			statusLabel.textColor = PROBLEM_COLOR;
			stepProblemImage.frame = CGRectMake(0, STEP_LINE_Y + state * LINE_H, LINE_H, LINE_H);
			stepProblemImage.hidden = NO;
			sendButton.hidden = YES;
			sendProgressView.hidden = YES;				
			break;
		case r_fail:
			end = YES;
			statusLabel.textColor = FAIL_COLOR;
			stepFailImage.frame = CGRectMake(0, STEP_LINE_Y + state * LINE_H, LINE_H, LINE_H);
			stepFailImage.hidden = NO;
			sendButton.hidden = YES;
			sendProgressView.hidden = YES;				
			break;
		case r_pass:
			end = YES;
			statusLabel.textColor = PASS_COLOR;
			stepPassImage[state].hidden = NO;
			break;
		default:
			break;
	}
}

- (void)updateFinishedSuccess:(BOOL)success {
	if (end) return;
	if (success) {
		substate = ss_user;
		[self stateUpdate:s_restart Result:r_none Message:NSLocalizedString(@"Drone updated succesfull, please restart drone", @"")];
	} else {
		[self stateUpdate:s_install Result:r_fail Message:NSLocalizedString(@"Update fail", @"")];
	}
}

- (NSString*)getErrorMessage:(NSInteger)errorNr {
	switch (errorNr) {
		case ERROR_BATTERY_LOW:
			return NSLocalizedString(@"Battery Low, Please charge your drone battery", @"");
		case ERROR_CANNOT_CONNECT_TO_TOY:
			return NSLocalizedString(@"Cannot connect to Toy", @"");
		case ERROR_WIFI_NOT_REACHABLED:
			return NSLocalizedString(@"Wifi not Reachabled", @"");
			break;
		default:
			return @"";
	}
}

- (void)errorMessage:(NSInteger)errorNr {
	if (end) return;
	
	if (errorNr != ERROR_NO_ERROR) {
		if ((state == s_connect) && (errorNr == ERROR_CANNOT_CONNECT_TO_TOY)) {
			[self stateUpdate:s_connect Result:r_problem Message:[NSString stringWithFormat:NSLocalizedString(@"Drone error: %@%@", @""), [self getErrorMessage:errorNr], @""]];
		} else if ((state == s_restart) && (substate == ss_user) && (errorNr == ERROR_CANNOT_CONNECT_TO_TOY)) {
			substate = ss_app;
			[self stateUpdate:s_restart Result:r_none Message:NSLocalizedString(@"Wait for reconnect", @"")];			
		} else {
			if ((state == s_send) && (substate == ss_app)) { //if drone error during sending file close FTP connection
				[self stopSendWithStatus:[NSString stringWithFormat:NSLocalizedString(@"Drone error: %@%@", @""), [self getErrorMessage:errorNr], @""]];
			}
			[self stateUpdate:state Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Drone error: %@%@", @""), [self getErrorMessage:errorNr], NSLocalizedString(@"\nPlease close application", @"")]];							
		}
	} else {
		if (state == s_connect) {
			[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Problem solved", @"")];
		}
		if ((state == s_restart) && (substate == ss_app)) {
			[self stateUpdate:s_restart Result:r_progress Message:NSLocalizedString(@"Reconnecting", @"")];				
		}
	}
}

- (void)droneConnected:(NSString*)droneFirmwareVersion2 {
	if (end) 
	{
		return;
	}
	if (state == s_connect) {
		[self stateUpdate:s_check Result:r_none Message:@""];
		switch ([firmwareVersion compare:droneFirmwareVersion2 options:NSNumericSearch]) {
			case -1:
#ifndef ALLOW_UPDATE_ALWAYS
				[self stateUpdate:s_check Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"You already have later firmware version", @"")]];
#else
				[self stateUpdate:s_send Result:r_none Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"You already have later firmware version", @"")]];
#endif
				break;
			case 0:
#ifndef ALLOW_UPDATE_ALWAYS
				[self stateUpdate:s_check Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"You already have the same firmware version", @"")]];
#else
				[self stateUpdate:s_send Result:r_none Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"You already have the same firmware version", @"")]];
#endif
				break;
			case 1:
				[self stateUpdate:s_send Result:r_none Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Later firmware version available", @"")]];
				break;
		}
	}
	if (state == s_restart) {
		[self stateUpdate:s_recheck Result:r_none Message:@""];
		if ([firmwareVersion isEqualToString:droneFirmwareVersion2]) {
			[self stateUpdate:s_recheck Result:r_pass Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Update succesfull", @"")]];
		} else {
			//check errors
			if([self checkFTP:@"err.log"])
				[self getErrorLog];
			else
				[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Drone firmware %@\n%@", @""), droneFirmwareVersion2, NSLocalizedString(@"Update fail", @"")]];
		}
	}
}

#pragma mark FTP

- (void)getVersionText
{
	assert(self.networkStream == nil);
	assert(self.fileStream == nil);
	streamOpen = NO;	
	
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:@"ftp://%@:%d/%@", self.localIP, 5551, @"version.txt"]];
		//	NSURL *url = [NSURL URLWithString:@"ftp://172.20.31.16:21/version.txt"];
	CFReadStreamRef ftpStream = CFReadStreamCreateWithFTPURL(NULL, (CFURLRef) url);
	assert(ftpStream != NULL);
	
	self.fileStream = (NSInputStream *) ftpStream;
	self.fileStream.delegate = self;
	[self.fileStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[self.fileStream open];
	
	CFRelease(ftpStream);
	
	if(state == s_connect)
		[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Launching with drone", @"")];
	else {
		[self stateUpdate:s_restart Result:r_pass Message:NSLocalizedString(@"Recheck", @"")];
		end = NO;
	}

	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		//sometimes (especially when previous file sending was interrupted) stream is not open without any feedback or error message
		//it's to check is stream for sure open and avoid app stuck
	[self performSelector:@selector(checkConnection) withObject:nil afterDelay:1];	
}

- (void)getErrorLog
{
	assert(self.networkStream == nil);
	assert(self.fileStream == nil);
	streamOpen = NO;	
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:@"ftp://%@:%d/%@", self.localIP, 5551, @"err.log"]];

		//	NSURL *url = [NSURL URLWithString:@"ftp://172.20.31.16:21/version.txt"];
	CFReadStreamRef ftpStream = CFReadStreamCreateWithFTPURL(NULL, (CFURLRef) url);
	assert(ftpStream != NULL);
	
	self.fileStream = (NSInputStream *) ftpStream;
	self.fileStream.delegate = self;
	[self.fileStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[self.fileStream open];
	
	CFRelease(ftpStream);
	
	if(state == s_connect)
		[self stateUpdate:s_connect Result:r_progress Message:NSLocalizedString(@"Launching with drone", @"")];
	else if(state == s_restart){
		[self stateUpdate:s_restart Result:r_pass Message:NSLocalizedString(@"Recheck", @"")];
		end = NO;
	}

	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		//sometimes (especially when previous file sending was interrupted) stream is not open without any feedback or error message
		//it's to check is stream for sure open and avoid app stuck
	[self performSelector:@selector(checkConnection) withObject:nil afterDelay:1];	
}



- (void)sendAction {	
	assert(self.networkStream == nil);
	assert(self.fileStream == nil);
	streamOpen = NO;
	
	self.fileStream = [NSInputStream inputStreamWithFileAtPath:firmwarePath];
	assert(self.fileStream != nil);
	[self.fileStream open];
	
		//NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, wifi_ardrone_ip, FTP_PORT, firmwareFileName]];
		//NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:FTP_ADDRESS, @"172.20.31.16", 21, firmwareFileName]];
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:@"ftp://%@:%d/%@.plf", self.localIP, 5551, firmwareFileName]];
		//	NSURL *url = [NSURL URLWithString:@"ftp://172.20.31.16:21/ardrone_update.plf"];
	CFWriteStreamRef ftpStream = CFWriteStreamCreateWithFTPURL(NULL, (CFURLRef) url);
	assert(ftpStream != NULL);
	
	self.networkStream = (NSOutputStream *) ftpStream;
		//	[self.networkStream setProperty:@"" forKey:(id)kCFStreamPropertyFTPUserName];
		//	[self.networkStream setProperty:@"" forKey:(id)kCFStreamPropertyFTPPassword];	
	self.networkStream.delegate = self;
	[self.networkStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[self.networkStream open];
	
	CFRelease(ftpStream);
	
	[self stateUpdate:s_send Result:r_progress Message:NSLocalizedString(@"Sending", @"")];
	[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		//sometimes (especially when previous file sending was interrupted) stream is not open without any feedback or error message
		//it's to check is stream for sure open and avoid app stuck
	[self performSelector:@selector(checkStream) withObject:nil afterDelay:1];
}

- (void) checkRestart:(NSTimer*)theTimer
{
	if(!droneRestarted)
	{
		if([self checkFTP:@"version.txt"])
		{
			[self performSelector:@selector(checkRestart:) withObject:nil afterDelay:1];
			return;
		}
		else {
			droneRestarted = YES;
			[self performSelector:@selector(checkRestart:) withObject:nil afterDelay:1];
			return;
		}
		
	}
	else
	{
		if([self checkFTP:@"version.txt"])
		{
			[self getVersionText];
			return;
		}	
		else 
		{
			[self performSelector:@selector(checkRestart:) withObject:nil afterDelay:1];
			return;
		}
	}
}

- (BOOL)checkFTP:(NSString *)fileName
{
	uint8_t         tmpbuffer[1];
	CFReadStreamRef ftpStream;
	NSURL *url = [NSURL URLWithString: [NSString stringWithFormat:@"ftp://%@:%d/%@", self.localIP, 5551, fileName]];
		//	NSURL *url = [NSURL URLWithString:@"ftp://172.20.31.16:21/version.txt"];
	ftpStream = CFReadStreamCreateWithFTPURL(NULL, (CFURLRef) url);
	if(ftpStream != NULL && CFReadStreamOpen(ftpStream))
	{
		int counter = 0;
		
		while (!CFReadStreamHasBytesAvailable(ftpStream)) {
			counter++;
			//check timeout
			if(counter > 8000)
			{
				if(state == s_restart)
					[self stateUpdate:s_restart Result:r_progress Message:NSLocalizedString(@"Reconnecting", @"")];
				CFReadStreamClose(ftpStream);
				CFRelease(ftpStream);
				ftpStream = NULL;
				return FALSE;
			}
		}
		if(CFReadStreamRead(ftpStream, tmpbuffer, 1) > 0)
		{
			CFReadStreamClose(ftpStream);
			CFRelease(ftpStream);
			ftpStream = NULL;
			return TRUE;	
		}
		else 
		{
			CFRelease(ftpStream);
			ftpStream = NULL;
			[self checkRestart:nil];
			return FALSE;			
		}
	}
	return FALSE;
}


- (void)checkConnection
{
	if (!streamOpen && (self.fileStream != nil)) {
		[self stopReceiveWithStatus:NSLocalizedString(@"Cannot open stream", @"")];
	}
}

- (void)checkStream {
	if (!streamOpen && (self.networkStream != nil)) {
		[self stopSendWithStatus:NSLocalizedString(@"Cannot open stream", @"")];
	}
}

- (void)stopSendWithStatus:(NSString *)statusString {
	if (self.networkStream != nil) {
		[self.networkStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		self.networkStream.delegate = nil;
		[self.networkStream close];
		self.networkStream = nil;
	}
	if (self.fileStream != nil) {
		[self.fileStream close];
		self.fileStream = nil;
	}
	streamOpen = NO;
	if (statusString == nil) {
			//		[self stateUpdate:s_install Result:r_progress Message:NSLocalizedString(@"Plf file sended successfully\nWait for update", @"")];
		[self stateUpdate:s_install Result:r_pass Message:NSLocalizedString(@"Plf file sended successfully\nWait for update", @"")];
			//		[drone updateDrone];
		substate = ss_user;
		[self stateUpdate:s_restart Result:r_none Message:NSLocalizedString(@"Drone updated succesfull, please restart drone", @"")];
			//		[self checkRestart:nil];
		[self performSelector:@selector(checkRestart:) withObject:nil afterDelay:1];
	} else {
		if (!end) {
			[self stateUpdate:s_send Result:r_fail Message:[NSString stringWithFormat:NSLocalizedString(@"Sending file error: %@%@", @""), statusString, NSLocalizedString(@"\nPlease close application", @"")]];
		}
	}
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
}

- (void)stopReceiveWithStatus:(NSString *)statusString {
	if (self.fileStream != nil) {
		[self.fileStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		self.fileStream.delegate = nil;
		[self.fileStream close];
		self.fileStream = nil;
	}
	streamOpen = NO;
	if (statusString == nil) {
			//		[self stateUpdate:s_install Result:r_progress Message:NSLocalizedString(@"Plf file sended successfully\nWait for update", @"")];
		
		
			//		[drone updateDrone];
	} else {
		if (!end) {
			[self stateUpdate:s_connect Result:r_fail Message:statusString];
		}
	}
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
}


	// Because buffer is declared as an array, you have to use a custom getter.  
	// A synthesised getter doesn't compile.
- (uint8_t *)buffer {
	return self->_buffer;
}

- (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)eventCode {
	assert(aStream == self.networkStream || aStream == self.fileStream );
	switch (eventCode) {
		case NSStreamEventHasBytesAvailable: {
			NSInteger       bytesRead;
			uint8_t         tmpbuffer[32768];
			
				// Pull some data off the network.
			bytesRead = [self.fileStream read:tmpbuffer maxLength:sizeof(tmpbuffer)];
			if (bytesRead == -1) {
					//					[self _stopReceiveWithStatus:@"Network read error"];
				[self errorMessage:ERROR_CANNOT_CONNECT_TO_TOY];
			} else if (bytesRead == 0) {			
					//					[self _stopReceiveWithStatus:nil];
				[self.fileStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
				self.fileStream.delegate = nil;
				[self.fileStream close];
				self.fileStream = nil;
				streamOpen = NO;
				if(state != s_recheck)
					[self droneConnected:self.droneFirmwareVersion];
			} else {
				if(state == s_recheck)
				{
					self.errorLog = [[NSString alloc] initWithBytes:tmpbuffer length:bytesRead encoding:NSUTF8StringEncoding];
					self.errorLog = [self.errorLog stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];

					NSArray* log = [self.errorLog componentsSeparatedByString:@"="];
					if([log count] > 1)
					{
						self.errorLog = [log objectAtIndex:1];
						[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", NSLocalizedString(@"Update fail", @""), self.errorLog]];
					}
					else if([log count] == 1)
					{
						self.errorLog = [log objectAtIndex:0];
						[self stateUpdate:s_recheck Result:r_fail Message:[NSString stringWithFormat:@"%@\n%@", NSLocalizedString(@"Update fail", @""), self.errorLog]];						
					}
				}
				else
				{
					//read version number				
					self.droneFirmwareVersion = [[NSString alloc] initWithBytes:tmpbuffer length:bytesRead encoding:NSUTF8StringEncoding];
					self.droneFirmwareVersion = [self.droneFirmwareVersion stringByTrimmingCharactersInSet:[NSCharacterSet newlineCharacterSet]];
				}
			}
			
		} break;
		case NSStreamEventHasSpaceAvailable:            
				// If we don't have any data buffered, go read the next chunk of data.
			if (self.bufferOffset == self.bufferLimit) {
				NSInteger bytesRead = [self.fileStream read:self.buffer maxLength:SEND_BUFFER_SIZE];                
				if (bytesRead == -1) {
					[self stopSendWithStatus:NSLocalizedString(@"File read error", @"")];
				} else if (bytesRead == 0) {
					[self stopSendWithStatus:nil]; //sending finished, success
				} else {
					self.bufferOffset = 0;
					self.bufferLimit = bytesRead;
				}
			}
				// If we're not out of data completely, send the next chunk.
			if (self.bufferOffset != self.bufferLimit) {
				NSInteger bytesWritten = [self.networkStream write:&self.buffer[self.bufferOffset] maxLength:self.bufferLimit - self.bufferOffset];
				assert(bytesWritten != 0);
				if (bytesWritten == -1) {
					[self stopSendWithStatus:NSLocalizedString(@"Network write error", @"")];
				} else {
					self.bufferOffset += bytesWritten;
					sendedSize += bytesWritten;
					[sendProgressView setProgress:1.0 * sendedSize / firmwareSize];
				}
			}
			break;
		case NSStreamEventErrorOccurred:
			if(state == s_connect)
			{
				[self stopReceiveWithStatus:NSLocalizedString(@"Cannot connect to Toy", @"")];
			}
			else
			{
				[self stopSendWithStatus:NSLocalizedString(@"Stream open error", @"")];
			}
			break;
		case NSStreamEventOpenCompleted:
			streamOpen = YES;
			break;
		case NSStreamEventNone:
		case NSStreamEventEndEncountered:
			break;
				//        case NSStreamEventHasBytesAvailable:
		default:
			assert(NO); //should never happen for the output stream
			break;
	}
}

- (NSString *)deviceIPAdress {
//	InitAddresses();
//	GetIPAddresses();
//	GetHWAddresses();
//	
//	/* 
//	 int i;
//	 NSString *deviceIP;
//	 for (i=0; i<MAXADDRS; ++i)
//	 {
//	 static unsigned long localHost = 0x7F000001;		// 127.0.0.1
//	 unsigned long theAddr;
//	 
//	 theAddr = ip_addrs[i];
//	 
//	 if (theAddr == 0) break;
//	 if (theAddr == localHost) continue;
//	 
//	 NSLog(@"%s %s %s\n", if_names[i], hw_addrs[i], ip_names[i]);
//	 }
//	 deviceIP = [NSString stringWithFormat:@"%s", ip_names[i]];
//	 */
//	
//		//this will get you the right IP from your device in format like 198.111.222.444. If you use the for loop above you will se that ip_names array will also contain localhost IP 127.0.0.1 that's why I don't use it. Eventualy this was code from mac that's why it uses arrays for ip_names as macs can have multiple IPs
//	ip_names[1][strlen(ip_names[1]) - 1] -= 1;
//	return [NSString stringWithFormat:@"%s", ip_names[1]];
	
	
	char drone_address[64];
	unsigned long theAddr;
	memset(drone_address, 0x0, sizeof(drone_address));
	
	while((theAddr = deviceIPAddress(WIFI_ITFNAME, iphone_mac_address)) == LOCALHOST) 
	{
		[NSThread sleepForTimeInterval:0.25];
	}
	
	struct in_addr drone_addr;
	drone_addr.s_addr = htonl( ntohl((in_addr_t)theAddr) - 1 );
	memcpy(drone_address, inet_ntoa(drone_addr), strlen(inet_ntoa(drone_addr)));
	return [NSString stringWithFormat:@"%s", drone_address];
}

#pragma mark memo

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
}

- (void)dealloc {
	[self stopSendWithStatus:NSLocalizedString(@"Stop", @"")];
		//	[drone release];
	for (NSInteger i = 0; i < STEP_LINE_NR; i++) {
		[stepPassImage[i] release];
	}
	[droneFirmwareVersion release];
	[stepProblemImage release];
	[stepFailImage release];
	[stepIndicator release];
	[sendButton release];
	[sendProgressView release];
	[statusLabel release];
	[_networkStream release];
	[_fileStream release];
	[super dealloc];
}

@end

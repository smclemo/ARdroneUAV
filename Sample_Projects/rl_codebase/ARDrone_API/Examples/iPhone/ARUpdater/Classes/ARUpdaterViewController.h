//
//  ARUpdaterViewController.h
//  AR.Updater
//
//  Created by Robert Ryll on 10-05-14.
//  Copyright Playsoft 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#include <CFNetwork/CFNetwork.h>
#include "wifi.h"
#include <netinet/in.h>
#include <arpa/inet.h>


	//#import "ARDrone.h"

@interface ARUpdaterViewController : UIViewController {
	NSString *firmwarePath;
	NSString *firmwareFileName;
	NSString *firmwareVersion;
	NSString *droneFirmwareVersion;
	NSString *errorLog;
	NSString *localIP;

	NSTimer *checkRestartTimer;
	
	CFReadStreamRef ftpStreamTest;
    NSOutputStream *_networkStream;
    NSInputStream *_fileStream;
    uint8_t _buffer[SEND_BUFFER_SIZE];
    size_t _bufferOffset;
    size_t _bufferLimit;
		//	ARDrone *drone;
	BOOL droneRestarted;
	NSInteger retryCounter;
}

@property (nonatomic, retain) NSString *firmwarePath;
@property (nonatomic, retain) NSString *firmwareFileName;
@property (nonatomic, retain) NSString *firmwareVersion;
@property (nonatomic, retain) NSString *droneFirmwareVersion;
@property (nonatomic, retain) NSString *localIP;
@property (nonatomic, retain) NSString *errorLog;

@property (nonatomic, retain) NSTimer *checkRestartTimer;

@property (nonatomic, retain) NSOutputStream *networkStream;
@property (nonatomic, retain) NSInputStream *fileStream;
@property (nonatomic, readonly) uint8_t *buffer;
@property (nonatomic, assign) size_t bufferOffset;
@property (nonatomic, assign) size_t bufferLimit;

- (void)updateFinishedSuccess:(BOOL)success;
- (void)errorMessage:(NSInteger)errorNr;
- (void)droneConnected:(NSString*)droneFirmwareVersion;
- (void)getVersionText;
- (void)getErrorLog;
- (NSString *)deviceIPAdress;
- (void)checkConnection;
- (void) checkRestart:(NSTimer*)theTimer;
- (BOOL)checkFTP:(NSString *)fileName;
- (void)stopReceiveWithStatus:(NSString *)statusString;

@end

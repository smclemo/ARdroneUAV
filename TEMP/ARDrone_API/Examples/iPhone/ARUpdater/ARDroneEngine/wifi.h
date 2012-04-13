/*
 *  wifi.h
 *  ARDroneEngine
 *
 *  Created by f.dhaeyer on 05/05/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#define MAXADDRS	32
#define LOCALHOST	0x7F000001 	// 127.0.0.1

#if TARGET_CPU_X86 == 1 // We are on iPhone simulator
#define WIFI_ITFNAME "en1"
#endif // TARGET_CPU_X86

#if TARGET_CPU_ARM == 1 // We are on real iPhone
#define WIFI_ITFNAME "en0"
#endif // TARGET_CPU_ARM

extern char *if_names[MAXADDRS];
extern char *ip_names[MAXADDRS];
extern char *hw_addrs[MAXADDRS];
extern unsigned long ip_addrs[MAXADDRS];


// Function prototypes
unsigned long deviceIPAddress(const char *itfName, char *hw_address);
void InitAddresses();
void FreeAddresses();
void GetIPAddresses();
void GetHWAddresses();
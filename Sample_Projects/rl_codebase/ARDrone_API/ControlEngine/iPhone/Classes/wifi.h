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
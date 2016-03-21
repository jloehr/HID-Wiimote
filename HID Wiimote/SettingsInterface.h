/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	SettingsInterface.h

Abstract:
	Shared types and values for the Settings Device Interface, provided by the driver and used by the user mode dll.

*/
#pragma once

#include <initguid.h>

// {8259315A-7B87-4D02-83A3-20CBCDAD7647}
DEFINE_GUID(GUID_DEVINTERFACE_HIDWIIMOTE,
	0x8259315a, 0x7b87, 0x4d02, 0x83, 0xa3, 0x20, 0xcb, 0xcd, 0xad, 0x76, 0x47);


#define IOCTL_WIIMOTE_TEST CTL_CODE(FILE_DEVICE_KEYBOARD, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

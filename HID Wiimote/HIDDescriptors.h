/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	HIDDescriptor.h

Abstract:
	Common Header file.
	Contains common includes and declarations.

*/
#pragma once

#include "HID.h"

#define DEFAULT_REPORT_ID 0x01

extern CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[];
extern CONST HID_DESCRIPTOR HIDDescriptor;
extern CONST BYTE HIDReportDescriptorSize;
extern CONST BYTE HIDReportSize;

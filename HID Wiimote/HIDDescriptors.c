/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	HIDDescriptor.c

Abstract:
	Contains the definition of several HID Descriptors
*/

#include "HIDDescriptors.h"

/*
extern CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[];
extern CONST HID_DESCRIPTOR HIDDescriptor;
extern CONST BYTE HIDReportDescriptorSize;
extern int foo = 1;
*/

#if defined MOUSE_DPAD

CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, DEFAULT_REPORT_ID,	   //	  REPORT_ID (1)  
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x02,                    //     REPORT_COUNT (2)
	0x81, 0x06,                    //     INPUT (Data,Var,Rel)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0xc0,                          //   END_COLLECTION
	0xc0                           // END_COLLECTION
};

CONST BYTE HIDReportSize = 5;

/*
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Report ID							    |
|--------|-------|--------|-------|--------|-------|--------|-------|
|		Button 1-3		  |xxxxxxxxxxxxxxxx Padding	xxxxxxxxxxxxxxxx|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								X-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Y-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|xxxxxxxxxxxxxxxxxxxxxxxxxxxxx Padding	xxxxxxxxxxxxxxxxxxxxxxxxxxxx|
*/

#else
#if defined MOUSE_IR

CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, DEFAULT_REPORT_ID,	   //	  REPORT_ID (1)  
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x02,              //     LOGICAL_MAXIMUM (767)
	0x75, 0x0A,                    //     REPORT_SIZE (10)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x06,                    //     REPORT_SIZE (6)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x09, 0x31,                    //     USAGE (Y)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0x3F, 0x02,              //     LOGICAL_MAXIMUM (525)
	0x75, 0x0A,                    //     REPORT_SIZE (10)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x06,                    //     REPORT_SIZE (6)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0xc0,                          //   END_COLLECTION
	0xc0                           // END_COLLECTION
};

CONST BYTE HIDReportSize = 6;

/*
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Report ID							    |
|--------|-------|--------|-------|--------|-------|--------|-------|
|		Button 1-3		  |xxxxxxxxxxxxxxxx Padding	xxxxxxxxxxxxxxxx|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								X-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								X-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Y-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Y-Axis								|
*/

#else

/* Old
CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, DEFAULT_REPORT_ID,	   //	  REPORT_ID (1)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //	  USAGE_MINIMUM (Button 1)
    0x29, 0x05,                    //     USAGE_MAXIMUM (Button 5)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x05,                    //     REPORT_COUNT (5)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x03,                    //     REPORT_SIZE (3)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x19, 0x06,                    //	  USAGE_MINIMUM (Button 6)
    0x29, 0x0A,                    //     USAGE_MAXIMUM (Button 10)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x05,                    //     REPORT_COUNT (5)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x02,                    //     REPORT_SIZE (2)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x19, 0x0B,                    //	  USAGE_MINIMUM (Button 11)
    0x29, 0x0B,                    //     USAGE_MAXIMUM (Button 11)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};
//*/
//* New

CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, DEFAULT_REPORT_ID,	   //	  REPORT_ID (1)  
    0x05, 0x01,                    //	  USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //	  USAGE (X)
    0x09, 0x31,                    //	  USAGE (Y)
    0x15, 0x81,                    //	  LOGICAL_MINIMUM (-127)
    0x25, 0x7F,					   //	  LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //	  REPORT_SIZE (8)
    0x95, 0x02,                    //	  REPORT_COUNT (2)
    0x81, 0x02,                    //	  INPUT (Data,Var,Abs)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //	  USAGE_MINIMUM (Button 1)
    0x29, 0x0D,                    //     USAGE_MAXIMUM (Button 13)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x0D,                    //     REPORT_COUNT (13)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x03,                    //     REPORT_SIZE (3)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //	  USAGE_PAGE (Generic Desktop)
	0x09, 0x33,                    //	  USAGE (RX)
    0x09, 0x34,                    //	  USAGE (RY)
	0x09, 0x35,                    //	  USAGE (RZ)
    0x15, 0x00,					   //	  LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,			   //	  LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //	  REPORT_SIZE (8)
    0x95, 0x03,                    //	  REPORT_COUNT (3)
    0x81, 0x02,                    //	  INPUT (Data,Var,Abs)
	0x05, 0x01,                    //	  USAGE_PAGE (Generic Desktop)
	0x09, 0x39,                    //     USAGE (Hat Swtich)
	0x15, 0x01,                    //     LOGICAL_MINIMUM (1)
	0x25, 0x08,                    //     LOGICAL_MAXIMUM (8)
	0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
	0x46, 0x3b, 0x01,              //     PHYSICAL_MAXIMUM (315)
	0x55, 0x00,                    //     UNIT_EXPONENT (0)
	0x65, 0x12,                    //     UNIT (SI Rot:Angular Pos)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x4a,                    //     INPUT (Data,Var,Abs,Wrap,Null)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

CONST BYTE HIDReportSize = 9;

/*
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Report ID							    |
|--------|-------|--------|-------|--------|-------|--------|-------|
|								X-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								Y-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|							Button 1-8								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|			  Button 9-13				   |xxxxxxx Padding xxxxxxxx|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								RX-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								RY-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								RZ-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Hat Switch						    |
*/


#endif
#endif

CONST HID_DESCRIPTOR HIDDescriptor = {
	0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0111, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
    { 
		0x22, // descriptor type 
		sizeof(HIDReportDescriptor) // total length of report descriptor
	}  
};

CONST BYTE HIDReportDescriptorSize = sizeof(HIDReportDescriptor);

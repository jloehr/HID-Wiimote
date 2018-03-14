/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	HIDDescriptor.c

Abstract:
	Contains the definition of several HID Descriptors
*/
#include "HIDDescriptors.h"

CONST HID_REPORT_DESCRIPTOR HIDReportDescriptor[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	0xa1, 0x01,                    // COLLECTION (Application)
/*
				   |-----------------------------|
				   |		   Gamepad			 |
				   |-----------------------------|

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
|								Z-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								RX-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								RY-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|								RZ-Axis								|
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Hat Switch						    |
|--------|-------|--------|-------|--------|-------|--------|-------|

*/
	0x85, GAMEPAD_REPORT_ID,	   //   REPORT_ID (GAMEPAD_REPORT_ID)  
	0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //   USAGE (X)
	0x09, 0x31,                    //   USAGE (Y)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00,			   //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x02,                    //   REPORT_COUNT (2)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x05, 0x09,                    //   USAGE_PAGE (Button)
	0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
	0x29, 0x0D,                    //   USAGE_MAXIMUM (Button 13)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x95, 0x0D,                    //   REPORT_COUNT (13)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x03,                    //   REPORT_SIZE (3)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
	0x09, 0x32,                    //   USAGE (Z)
	0x09, 0x33,                    //   USAGE (RX)
	0x09, 0x34,                    //   USAGE (RY)
	0x09, 0x35,                    //   USAGE (RZ)
	0x15, 0x00,					   //   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00,			   //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x04,                    //   REPORT_COUNT (4)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
	0x09, 0x39,                    //   USAGE (Hat Swtich)
	0x15, 0x01,                    //   LOGICAL_MINIMUM (1)
	0x25, 0x08,                    //   LOGICAL_MAXIMUM (8)
	0x35, 0x00,                    //   PHYSICAL_MINIMUM (0)
	0x46, 0x3b, 0x01,              //   PHYSICAL_MAXIMUM (315)
	0x55, 0x00,                    //   UNIT_EXPONENT (0)
	0x65, 0x14,                    //   UNIT (English Rotation: Angular Position)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x81, 0x4a,                    //   INPUT (Data,Var,Abs,Wrap,Null)
/*
				   |-----------------------------|
				   |		   Wiimote			 |
				   |-----------------------------|
*/	
	0xc0                           // END_COLLECTION
/*
	0x06, 0x00, 0xff,              //   USAGE_PAGE (Vendor Defined)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x11,                    //   REPORT_ID (11)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x12,                    //   REPORT_ID (12)
	0x95, 0x02,                    //   REPORT_COUNT (2)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x13,                    //   REPORT_ID (13)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x14,                    //   REPORT_ID (14)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x15,                    //   REPORT_ID (15)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x16,                    //   REPORT_ID (16)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x17,                    //   REPORT_ID (17)
	0x95, 0x06,                    //   REPORT_COUNT (6)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x18,                    //   REPORT_ID (18)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x19,                    //   REPORT_ID (19)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x1a,                    //   REPORT_ID (1A)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x91, 0x00,                    //   OUTPUT (Data,Ary,Abs)
	0x85, 0x20,                    //   REPORT_ID (20)
	0x95, 0x06,                    //   REPORT_COUNT (6)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x21,                    //   REPORT_ID (21)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x22,                    //   REPORT_ID (22)
	0x95, 0x04,                    //   REPORT_COUNT (4)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x30,                    //   REPORT_ID (30)
	0x95, 0x02,                    //   REPORT_COUNT (2)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x31,                    //   REPORT_ID (31)
	0x95, 0x05,                    //   REPORT_COUNT (5)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x32,                    //   REPORT_ID (32)
	0x95, 0x0a,                    //   REPORT_COUNT (10)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x33,                    //   REPORT_ID (33)
	0x95, 0x11,                    //   REPORT_COUNT (17)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x34,                    //   REPORT_ID (34)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x35,                    //   REPORT_ID (35)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x36,                    //   REPORT_ID (36)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x37,                    //   REPORT_ID (37)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x3d,                    //   REPORT_ID (3D)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x3e,                    //   REPORT_ID (3E)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0x85, 0x3f,                    //   REPORT_ID (3F)
	0x95, 0x15,                    //   REPORT_COUNT (21)
	0x09, 0x01,                    //   USAGE (Vendor Ussage 1)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0xc0,                          // END_COLLECTION
/*
				   |-----------------------------|
				   |		  IR-Mouse			 |
				   |-----------------------------|

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
|--------|-------|--------|-------|--------|-------|--------|-------|
*/
/*
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0x85, IRMOUSE_REPORT_ID,	   //   REPORT_ID (IRMOUSE_REPORT_ID)  
	0x05, 0x09,                    //   USAGE_PAGE (Button)
	0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //   USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //   REPORT_COUNT (3)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x05,                    //   REPORT_SIZE (5)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //   USAGE (X)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x03,              //   LOGICAL_MAXIMUM (1023)
	0x75, 0x0B,                    //   REPORT_SIZE (11)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x05,                    //   REPORT_SIZE (5)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
	0x09, 0x31,                    //   USAGE (Y)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x03,              //   LOGICAL_MAXIMUM (1023)
	0x75, 0x0B,                    //   REPORT_SIZE (11)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x05,                    //   REPORT_SIZE (5)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
	0xc0,                          // END_COLLECTION
/*
				   |-----------------------------|
				   |		 DPad-Mouse			 |
				   |-----------------------------|

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
|--------|-------|--------|-------|--------|-------|--------|-------|
*/	
/*
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0x85, DPADMOUSE_REPORT_ID,	   //   REPORT_ID (DPADMOUSE_REPORT_ID)  
	0x05, 0x09,                    //   USAGE_PAGE (Button)
	0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //   USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //   REPORT_COUNT (3)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x05,                    //   REPORT_SIZE (5)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //   USAGE (X)
	0x09, 0x31,                    //   USAGE (Y)
	0x15, 0x81,                    //   LOGICAL_MINIMUM (-127)
	0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x95, 0x02,                    //   REPORT_COUNT (2)
	0x81, 0x06,                    //   INPUT (Data,Var,Rel)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
	0xc0                           // END_COLLECTION
*/
};


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

CONST size_t HIDReportDescriptorSize = sizeof(HIDReportDescriptor);

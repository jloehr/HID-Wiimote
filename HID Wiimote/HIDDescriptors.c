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
    0x15, 0xff,                    //	  LOGICAL_MINIMUM (-1)
    0x25, 0x01,                    //	  LOGICAL_MAXIMUM (1)
    0x75, 0x02,                    //	  REPORT_SIZE (2)
    0x95, 0x02,                    //	  REPORT_COUNT (2)
    0x81, 0x02,                    //	  INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x04,                    //     REPORT_SIZE (4)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //	  USAGE_MINIMUM (Button 1)
    0x29, 0x07,                    //     USAGE_MAXIMUM (Button 7)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x07,                    //     REPORT_COUNT (7)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //	  USAGE_PAGE (Generic Desktop)
    0x09, 0x34,                    //	  USAGE (RY)
    0x09, 0x33,                    //	  USAGE (RX)
    0x15, 0x00,					   //	  LOGICAL_MINIMUM (00)
    0x25, 0x3F,					   //	  LOGICAL_MAXIMUM (63)
    0x75, 0x06,                    //	  REPORT_SIZE (6)
    0x95, 0x02,                    //	  REPORT_COUNT (2)
    0x81, 0x02,                    //	  INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x04,                    //     REPORT_SIZE (4)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};
//*/

/*
|--------|-------|--------|-------|--------|-------|--------|-------|
|							  Report ID							    |
|--------|-------|--------|-------|--------|-------|--------|-------|
|		X-Axis	 |		Y-Axis	  |xxxxxxxxxxxx Padding	xxxxxxxxxxxx|
|--------|-------|--------|-------|--------|-------|--------|-------|
|						Button 1-7							|xxxxxxx|
|--------|-------|--------|-------|--------|-------|--------|-------|
|				   RY-Axis						   |	RY-Axis
|--------|-------|--------|-------|--------|-------|--------|-------|
			 RY-Axis			  |xxxxxxxxxxxx Padding	xxxxxxxxxxxx|
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

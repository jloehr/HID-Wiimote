/*

Copyright (C) 2014 Julian Löhr
All rights reserved.

Filename:
	WiimoteToHIDParser.h

Abstract:
	Header file for WiimoteToHIDParser.c
*/

#ifndef _WIIMOTE_TO_HID_PARSER_H_
#define _WIIMOTE_TO_HID_PARSER_H_

#include "HIDWiimote.h"

VOID ParseWiimoteState( _In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, _Out_ BYTE RequestBuffer[4]);
VOID ParseWiimoteStateAsDPadMouse( _In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, _Out_ BYTE RequestBuffer[4]);

#endif
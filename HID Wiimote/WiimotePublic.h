/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimotePublic.h

Abstract:
	Contains Types and Definitions that are also available to User Mode Code.
*/
#pragma once

// LEDs
#define WIIMOTE_LEDS_FOUR (0x10)
#define WIIMOTE_LEDS_THREE (0x20)
#define WIIMOTE_LEDS_TWO (0x40)
#define WIIMOTE_LEDS_ONE (0x80)
#define WIIMOTE_LEDS_ALL (WIIMOTE_LEDS_FOUR | WIIMOTE_LEDS_THREE | WIIMOTE_LEDS_TWO | WIIMOTE_LEDS_ONE)

// Enumerations
typedef enum _WIIMOTE_EXTENSION { None, Nunchuck, BalanceBoard, ClassicController, WiiUProController, Guitar } WIIMOTE_EXTENSION;
typedef enum _WIIMOTE_DRIVER_MODE { Gamepad, PassThrough, IRMouse, DPadMouse, GamepadAndIRMouse } WIIMOTE_DRIVER_MODE;

// Settings
typedef struct _WIIMOTE_SETTINGS
{
	BOOLEAN XAxisEnabled;
	BOOLEAN YAxisEnabled;
	BOOLEAN MouseButtonsSwitched;
	BOOLEAN TriggerAndShoulderSwitched;
	BOOLEAN TriggerSplit;

} WIIMOTE_SETTINGS, * PWIIMOTE_SETTINGS;
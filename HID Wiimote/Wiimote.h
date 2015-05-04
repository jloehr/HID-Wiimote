/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Wiimote.h

Abstract:
	Header file for Wiimote.c
*/

#ifndef _WIIMOTE_H_
#define _WIIMOTE_H_

#include "HIDWiimote.h"

#pragma warning(disable:4201) //nameless struct/union

#define WIIMOTE_STATUSINFORMATION_INTERVAL (60000) // 60 Second

#define WIIMOTE_LEDS_FOUR (0xF0)
#define WIIMOTE_LEDS_THREE (0xE0)
#define WIIMOTE_LEDS_TWO (0xC0)
#define WIIMOTE_LEDS_ONE (0x80)


typedef struct _WIIMOTE_NUNCHUCK_STATE
{
	union
	{
		struct
		{
			BOOLEAN C;
			BOOLEAN Z;
		} Buttons;

		BOOLEAN BottonsRaw[2];
	};

	union
	{
		struct
		{
			BYTE X;
			BYTE Y;
		} AnalogStick;

		BYTE AnalogStickRaw[2];
	};

	union {
		struct
		{
			BYTE X;
			BYTE Y;
			BYTE Z;
		} Accelerometer;

		BYTE AccelerometerRaw[3];
	};

} WIIMOTE_NUNCHUCK_STATE, * PWIIMOTE_NUNCHUCK_STATE;


typedef struct _WIIMOTE_CLASSIC_CONTROLLER_STATE
{
	union
	{
		struct
		{
			BOOLEAN L;
			BOOLEAN R;
			BOOLEAN ZL;
			BOOLEAN ZR;
			BOOLEAN LH;
			BOOLEAN RH;
			BOOLEAN A;
			BOOLEAN B;
			BOOLEAN Y;
			BOOLEAN X;
			BOOLEAN Plus;
			BOOLEAN Minus;
			BOOLEAN Home;

			union {
				struct
				{
					BOOLEAN Up;
					BOOLEAN Down;
					BOOLEAN Left;
					BOOLEAN Right;
				} DPad;
				BOOLEAN DPadRaw[4];
			};

		} Buttons;

		BOOLEAN ButtonsRaw[15];
	};

	BYTE LeftTrigger;
	BYTE RightTrigger;

	union
	{
		struct
		{
			BYTE X;
			BYTE Y;
		} LeftAnalogStick;

		BYTE LeftAnalogStickRaw[2];
	};

	union
	{
		struct
		{
			BYTE X;
			BYTE Y;
		} RightAnalogStick;

		BYTE RightAnalogStickRaw[2];
	};

} WIIMOTE_CLASSIC_CONTROLLER_STATE, *PWIIMOTE_CLASSIC_CONTROLLER_STATE;

typedef struct _WIIMOTE_STATE
{
	union 
	{
		struct 
		{
			BOOLEAN A;
			BOOLEAN B;
			BOOLEAN One;
			BOOLEAN Two;
			BOOLEAN Plus;
			BOOLEAN Minus;
			BOOLEAN Home;

			union {
				struct
				{
					BOOLEAN Up;
					BOOLEAN Down;
					BOOLEAN Left;
					BOOLEAN Right;
				} DPad;
				BOOLEAN DPadRaw[4]; 
			};
		} CoreButtons;

		BOOLEAN CoreButtonsRaw[11];
	};

	union {
		struct
		{
			BYTE X;
			BYTE Y;
			BYTE Z;
		} Accelerometer;

		BYTE AccelerometerRaw[3];
	};


} WIIMTOE_STATE, *PWIIMOTE_STATE;

typedef struct _WIIMOTE_IR_POINT
{
	USHORT X;
	USHORT Y;
	
} WIIMOTE_IR_POINT, *PWIIMOTE_IR_POINT;


typedef struct _WIIMOTE_IR_STATE
{
	WIIMOTE_IR_POINT Point1;
	WIIMOTE_IR_POINT Point2;
	WIIMOTE_IR_POINT Point3;
	WIIMOTE_IR_POINT Point4;

} WIIMOTE_IR_STATE, *PWIIMOTE_IR_STATE;

typedef struct _WIIMOTE_DEVICE_CONTEXT
{
	enum WIIMOTE_EXTENSION_TYPE { None, Nunchuck, ClassicController, WiiUProController } Extension;
	WIIMTOE_STATE State;
	BYTE CurrentReportMode;

	union
	{
		WIIMOTE_NUNCHUCK_STATE NunchuckState;
		WIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState;
	};

#ifdef MOUSE_IR
	WIIMOTE_IR_STATE IRState;
#endif

	WDFTIMER StatusInformationTimer;

} WIIMOTE_DEVICE_CONTEXT, * PWIIMOTE_DEVICE_CONTEXT;

NTSTATUS PrepareWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS StartWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS StopWiimote(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS ProcessReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_ PVOID ReadBuffer, _In_ size_t ReadBufferSize);

EVT_WDF_TIMER StatusInformationTimerExpired;

#endif
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
#define WIIMTOE_LEDS_ALL (WIIMOTE_LEDS_FOUR | WIIMOTE_LEDS_THREE | WIIMOTE_LEDS_TWO | WIIMOTE_LEDS_ONE)

#define WIIMOTE_IR_POINTS 4
#define WIIMOTE_IR_POINTS_BUFFER_SIZE 5
#define WIIMOTE_IR_POINT_X_MAX 1023
#define WIIMOTE_IR_POINT_Y_MAX 767

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

typedef struct _WIIMOTE_BALANCE_BOARD_STATE
{
	union
	{
		struct
		{
			USHORT TopRight;
			USHORT BottomRight;
			USHORT TopLeft;
			USHORT BottomLeft;
		} Sensor;

		USHORT SensorRaw[4];
	};

} WIIMOTE_BALANCE_BOARD_STATE, *PWIIMOTE_BALANCE_BOARD_STATE;


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

typedef struct _WIIMOTE_GUITAR_STATE
{
	union
	{
		struct
		{
			BOOLEAN Green;
			BOOLEAN Red;
			BOOLEAN Yellow;
			BOOLEAN Blue;
			BOOLEAN Orange;
			BOOLEAN Plus;
			BOOLEAN Minus;
			BOOLEAN Up;
			BOOLEAN Down;
		} Buttons;

		BOOLEAN ButtonsRaw[9];
	};

	BYTE WhammyBar;
	BYTE TouchBar;

	union
	{
		struct
		{
			BYTE X;
			BYTE Y;
		} AnalogStick;

		BYTE AnalogStickRaw[2];
	};

} WIIMOTE_GUITAR_STATE, *PWIIMOTE_GUITAR_STATE;

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


	BYTE BatteryFlag;

} WIIMTOE_STATE, *PWIIMOTE_STATE;

typedef struct _WIIMOTE_IR_POINT
{
	USHORT X;
	USHORT Y;
	
} WIIMOTE_IR_POINT, *PWIIMOTE_IR_POINT;


typedef struct _WIIMOTE_IR_STATE
{
	WIIMOTE_IR_POINT Points[WIIMOTE_IR_POINTS_BUFFER_SIZE][WIIMOTE_IR_POINTS];
	BYTE PointsBufferPointer;

} WIIMOTE_IR_STATE, *PWIIMOTE_IR_STATE;

typedef struct _WIIMOTE_DEVICE_CONTEXT
{
	enum WIIMOTE_EXTENSION_TYPE { None, Nunchuck, BalanceBoard, ClassicController, WiiUProController, Guitar } Extension;
	WIIMTOE_STATE State;
	BYTE CurrentReportMode;

	union
	{
		WIIMOTE_NUNCHUCK_STATE NunchuckState;
		WIIMOTE_BALANCE_BOARD_STATE BalanceBoardState;
		WIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState;
		WIIMOTE_GUITAR_STATE GuitarState;
	};

	WIIMOTE_IR_STATE IRState;

	WDFTIMER BatteryLevelLEDUpdateTimer;

} WIIMOTE_DEVICE_CONTEXT, * PWIIMOTE_DEVICE_CONTEXT;

NTSTATUS PrepareWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS StartWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS StopWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS ResetToNullState(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS ProcessReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_reads_bytes_(ReadBufferSize) PVOID ReadBuffer, _In_ SIZE_T ReadBufferSize);

EVT_WDF_TIMER BatteryLevelLEDUpdateTimerExpired;

#endif

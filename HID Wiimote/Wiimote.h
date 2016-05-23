/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Wiimote.h

Abstract:
	Header file for Wiimote.c
*/
#pragma once

#include "HIDWiimote.h"

#include "WiimotePublic.h"
#include "WiimoteSettings.h"

#pragma warning(disable:4201) //nameless struct/union

#define WIIMOTE_STATUSINFORMATION_INTERVAL (60000) // 60 Second

#define WIIMOTE_IR_POINTS 4
#define WIIMOTE_IR_POINTS_BUFFER_SIZE 5
#define WIIMOTE_IR_POINT_X_MAX 1023
#define WIIMOTE_IR_POINT_Y_MAX 767

typedef struct _WIIMOTE_NUNCHUCK_STATE
{
	struct
	{
		BOOLEAN C;
		BOOLEAN Z;
	} Buttons;

	struct
	{
		BYTE X;
		BYTE Y;
	} AnalogStick;

	struct
	{
		BYTE X;
		BYTE Y;
		BYTE Z;
	} Accelerometer;

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

	union
	{
		struct
		{
			USHORT TopRight[3];
			USHORT BottomRight[3];
			USHORT TopLeft[3];
			USHORT BottomLeft[3];
		} Calibration;

		USHORT CalibrationRaw[4][3];
	};

} WIIMOTE_BALANCE_BOARD_STATE, *PWIIMOTE_BALANCE_BOARD_STATE;


typedef struct _WIIMOTE_CLASSIC_CONTROLLER_STATE
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

		struct
		{
			BOOLEAN Up;
			BOOLEAN Down;
			BOOLEAN Left;
			BOOLEAN Right;
		} DPad;

	} Buttons;

	BYTE LeftTrigger;
	BYTE RightTrigger;

	struct
	{
		BYTE X;
		BYTE Y;
	} LeftAnalogStick;

	struct
	{
		BYTE X;
		BYTE Y;
	} RightAnalogStick;

} WIIMOTE_CLASSIC_CONTROLLER_STATE, *PWIIMOTE_CLASSIC_CONTROLLER_STATE;

typedef struct _WIIMOTE_GUITAR_STATE
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

	BYTE WhammyBar;
	BYTE TouchBar;

	struct
	{
		BYTE X;
		BYTE Y;
	} AnalogStick;

} WIIMOTE_GUITAR_STATE, *PWIIMOTE_GUITAR_STATE;

typedef struct _WIIMOTE_STATE
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

		struct
		{
			BOOLEAN Up;
			BOOLEAN Down;
			BOOLEAN Left;
			BOOLEAN Right;
		} DPad;
	} CoreButtons;

	struct
	{
		BYTE X;
		BYTE Y;
		BYTE Z;
	} Accelerometer;

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
	BYTE CurrentReportMode;
	BYTE BatteryLevel;
	BYTE LEDState;

	WIIMOTE_SETTINGS Settings;
	WIIMOTE_EXTENSION Extension;
	WIIMTOE_STATE State;
	WIIMOTE_IR_STATE IRState;

	union
	{
		WIIMOTE_NUNCHUCK_STATE NunchuckState;
		WIIMOTE_BALANCE_BOARD_STATE BalanceBoardState;
		WIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState;
		WIIMOTE_GUITAR_STATE GuitarState;
	};

	WDFTIMER BatteryLevelLEDUpdateTimer;

} WIIMOTE_DEVICE_CONTEXT, * PWIIMOTE_DEVICE_CONTEXT;

NTSTATUS WiimotePrepare(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS WiimoteStart(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS WiimoteStop(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS WiimoteResetToNullState(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS WiimoteProcessReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_reads_bytes_(ReadBufferSize) PVOID ReadBuffer, _In_ SIZE_T ReadBufferSize);

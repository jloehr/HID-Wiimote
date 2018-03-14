/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	WiimoteState.h

Abstract:
	Header file for WiimoteState.c
*/
#pragma once

#include "HIDWiimote.h"

// --- Wii Remote --- //
typedef struct _WIIMOTE_WII_REMOTE_STATE
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

} WIIMOTE_WII_REMOTE_STATE, *PWIIMOTE_WII_REMOTE_STATE;

// --- IR --- //
#define WIIMOTE_IR_POINTS 4
#define WIIMOTE_IR_POINTS_BUFFER_SIZE 5
#define WIIMOTE_IR_POINT_X_MAX 1023
#define WIIMOTE_IR_POINT_Y_MAX 767

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

// --- Nunchuck --- //
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

} WIIMOTE_NUNCHUCK_STATE, *PWIIMOTE_NUNCHUCK_STATE;

// --- Balance Board --- //
#define WIIMOTE_BALANCE_BOARD_TOP_RIGHT		0
#define WIIMOTE_BALANCE_BOARD_BOTTOM_RIGHT	1
#define WIIMOTE_BALANCE_BOARD_TOP_LEFT		2
#define WIIMOTE_BALANCE_BOARD_BOTTOM_LEFT	3
#define WIIMOTE_BALANCE_BOARD_SENSOR_POINTS	4

#define WIIMOTE_BALANCE_BOARD_CALIBRATION_RECORDS 3

typedef struct _WIIMOTE_BALANCE_BOARD_STATE
{
	USHORT Sensor[4];
	USHORT Calibration[WIIMOTE_BALANCE_BOARD_SENSOR_POINTS][WIIMOTE_BALANCE_BOARD_CALIBRATION_RECORDS];

} WIIMOTE_BALANCE_BOARD_STATE, *PWIIMOTE_BALANCE_BOARD_STATE;

// --- Classic Controller (Pro) / Wii U Pro Controller --- //
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

// --- Guitar --- //
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

	BYTE TouchBar;
	BYTE WhammyBar;

	struct
	{
		BYTE X;
		BYTE Y;
	} AnalogStick;

} WIIMOTE_GUITAR_STATE, *PWIIMOTE_GUITAR_STATE;

// --- Wiimote State --- //
#pragma warning(push)
#pragma warning(disable:4201) //nameless struct/union
typedef struct _WIIMOTE_STATE
{
	WIIMOTE_WII_REMOTE_STATE WiiRemoteState;
	WIIMOTE_IR_STATE IRState;

	union
	{
		WIIMOTE_NUNCHUCK_STATE NunchuckState;
		WIIMOTE_BALANCE_BOARD_STATE BalanceBoardState;
		WIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState;
		WIIMOTE_GUITAR_STATE GuitarState;

	};

} WIIMOTE_STATE, *PWIIMOTE_STATE;
#pragma warning(pop)

VOID WiimoteStateResetToNullState(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS WiimoteStateUpdate(_In_ PDEVICE_CONTEXT DeviceContext, _In_reads_bytes_(DataSize) PUCHAR Data, _In_ size_t DataSize);

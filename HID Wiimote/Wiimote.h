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

typedef struct _WIIMOTE_DEVICE_CONTEXT
{
	WIIMTOE_STATE State;
	BYTE CurrentReportMode;

	WDFTIMER StatusInformationTimer;

} WIIMOTE_DEVICE_CONTEXT, * PWIIMOTE_DEVICE_CONTEXT;

NTSTATUS PrepareWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS StartWiimote(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS StopWiimote(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS ProcessReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_ PVOID ReadBuffer, _In_ size_t ReadBufferSize);

EVT_WDF_TIMER StatusInformationTimerExpired;

#endif
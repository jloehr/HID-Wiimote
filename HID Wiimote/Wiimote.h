/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	Wiimote.h

Abstract:
	Header file for Wiimote.c
*/
#pragma once

#include "HIDWiimote.h"

#include "DeviceInterfacePublic.h"
#include "WiimoteState.h"
#include "WiimoteSettings.h"

#define WIIMOTE_STATUSINFORMATION_INTERVAL (60000) // 60 Second

typedef struct _WIIMOTE_DEVICE_CONTEXT
{
	BYTE CurrentReportMode;
	BYTE BatteryLevel;
	BYTE LEDState;

	WIIMOTE_SETTINGS Settings;
	WIIMOTE_EXTENSION Extension;
	WIIMOTE_STATE State;

	WDFTIMER BatteryLevelLEDUpdateTimer;

} WIIMOTE_DEVICE_CONTEXT, * PWIIMOTE_DEVICE_CONTEXT;

NTSTATUS WiimotePrepare(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS WiimoteStart(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS WiimoteStop(_In_ PDEVICE_CONTEXT DeviceContext);

VOID WiimoteReset(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS WiimoteProcessReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_reads_bytes_(ReadBufferSize) PVOID ReadBuffer, _In_ SIZE_T ReadBufferSize);

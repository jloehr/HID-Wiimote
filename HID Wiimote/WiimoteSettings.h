/*

Copyright (C) 2017 Julian L�hr
All rights reserved.

Filename:
	WiimoteSettings.h

Abstract:
	Header file for WiimoteSettings.c
*/
#pragma once

#include "HIDWiimote.h"

#include "DeviceInterfacePublic.h"

VOID
WiimoteSettingsLoad(
	_In_ PDEVICE_CONTEXT DeviceContext
	);

VOID
WiimoteSettingsSetDriverMode(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ WIIMOTE_DRIVER_MODE DriverMode
	);

VOID
WiimoteSettingsSetEnableWiimoteXAxisAccelerometer(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetEnableWiimoteYAxisAccelerometer(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetSwapMouseButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetSwapTriggerAndShoulder(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetSplitTriggerAxis(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetMapTriggerAsAxis(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
);

VOID
WiimoteSettingsSetMapTriggerAsButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
);

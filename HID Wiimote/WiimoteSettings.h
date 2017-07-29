/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteSettings.h

Abstract:
	Header file for WiimoteSettings.c
*/
#pragma once

#include "HIDWiimote.h"

#include "WiimoteSettingsPublic.h"
#include "WiimotePublic.h"

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
WiimoteSettingsSetSwitchMouseButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetSwitchTriggerAndShoulder(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetSplitTriggerAxis(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

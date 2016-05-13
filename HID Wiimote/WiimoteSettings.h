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
WiimoteSettingsSetXAxisEnabled(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetYAxisEnabled(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetMouseButtonsSwitched(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetTriggerAndShoulderSwitched(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

VOID
WiimoteSettingsSetTriggerSplit(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	);

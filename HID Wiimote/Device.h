/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Device.h

Abstract:
	Header file for Device.c

*/
#pragma once

#include "HIDWiimote.h"

#include "DeviceInterface.h"
#include "Wiimote.h"
#include "Bluetooth.h"
#include "HID.h"

typedef struct _DEVICE_CONTEXT 
{
	WDFDEVICE Device;
	WDFIOTARGET IoTarget;

	PDEVICE_INTERFACE_CONTEXT SettingsInterfaceContext;
	BLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	HID_DEVICE_CONTEXT HIDContext;
	WIIMOTE_DEVICE_CONTEXT WiimoteContext;

	HID_MINIPORT_ADDRESSES HIDMiniportAddresses;

} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

	
EVT_WDF_DRIVER_DEVICE_ADD DeviceAdd;

EVT_WDF_DEVICE_PREPARE_HARDWARE PrepareHardware;
EVT_WDF_DEVICE_D0_ENTRY DeviceD0Entry;

EVT_WDF_DEVICE_D0_EXIT DeviceD0Exit;
EVT_WDF_DEVICE_RELEASE_HARDWARE ReleaseHardware;

VOID SetHIDMiniportAddresses(_In_ PDEVICE_CONTEXT DeviceContext, _In_ PHID_MINIPORT_ADDRESSES Addresses);
NTSTATUS SignalDeviceIsGone(_In_ PDEVICE_CONTEXT DeviceContext);

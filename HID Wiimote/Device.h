/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Device.h

Abstract:
	Header file for Device.c

*/
#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "HIDWiimote.h"

#include "Wiimote.h"
#include "Bluetooth.h"
#include "HID.h"

typedef struct _DEVICE_CONTEXT 
{
	WDFDEVICE Device;
	WDFIOTARGET IoTarget;

	PRAW_DEVICE_CONTEXT RawDeviceContext;

	BLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	HID_DEVICE_CONTEXT HIDContext;
	WIIMOTE_DEVICE_CONTEXT WiimoteContext;

	HID_MINIPORT_ADDRESSES HIDMiniportAddresses;

} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext);

	
EVT_WDF_DRIVER_DEVICE_ADD DeviceAdd;

EVT_WDF_DEVICE_PREPARE_HARDWARE PrepareHardware;
EVT_WDF_DEVICE_D0_ENTRY DeviceD0Entry;

EVT_WDF_DEVICE_D0_EXIT DeviceD0Exit;
EVT_WDF_DEVICE_RELEASE_HARDWARE ReleaseHardware;

VOID SetHIDMiniportAddresses(_In_ PDEVICE_CONTEXT DeviceContext, _In_ PHID_MINIPORT_ADDRESSES Addresses);
NTSTATUS SignalDeviceIsGone(_In_ PDEVICE_CONTEXT DeviceContext);


#endif
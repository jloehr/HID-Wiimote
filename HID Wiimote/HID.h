/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	HID.h

Abstract:
	Header file for HID.c
*/
#pragma once

#include "HIDWiimote.h"

#include <hidport.h>

#include "ReadIoControlBuffer.h"

typedef struct _HID_DEVICE_CONTEXT 
{
	WDFQUEUE DefaultIOQueue;
	READ_IO_CONTROL_BUFFER ReadBuffer;

	USHORT VendorID;
	USHORT ProductID;

	BOOLEAN GamepadAndIRMouseReportToggleFlag;

} HID_DEVICE_CONTEXT, * PHID_DEVICE_CONTEXT;

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

NTSTATUS HIDPrepare(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS HIDCreateQueues(_In_ WDFDEVICE Device, _In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS HIDRelease(_In_ PDEVICE_CONTEXT DeviceContext);

VOID HIDWiimoteStateUpdated(_In_ PDEVICE_CONTEXT DeviceContext);

#define IOCTL_WIIMOTE_ADDRESSES CTL_CODE(FILE_DEVICE_KEYBOARD, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	DeviceInterface.h

Abstract:
	Header file for DeviceInterface.c

*/
#pragma once

#include "HIDWiimote.h"

#include "ReadIoControlBuffer.h"

typedef struct _DEVICE_INTERFACE_CONTEXT
{
	WDFDEVICE InterfaceDevice;
	PDEVICE_CONTEXT Parent;

	WDFQUEUE DefaultIOQueue;
	READ_IO_CONTROL_BUFFER ReadBuffer;

} DEVICE_INTERFACE_CONTEXT, * PDEVICE_INTERFACE_CONTEXT;

NTSTATUS DeviceInterfaceCreate(_In_ PDEVICE_CONTEXT ParentDeviceContext);
NTSTATUS DeviceInterfaceRelease(_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext);

VOID DeviceInterfaceWiimoteStateUpdated(_In_ PDEVICE_CONTEXT DeviceContext);

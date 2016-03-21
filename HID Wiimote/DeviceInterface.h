/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	DeviceInterface.h

Abstract:
	Header file for DeviceInterface.c

*/
#pragma once

#include "HIDWiimote.h"

typedef struct _DEVICE_INTERFACE_CONTEXT
{
	WDFDEVICE InterfaceDevice;
	PDEVICE_CONTEXT Parent;

	WDFQUEUE DefaultIOQueue;

} DEVICE_INTERFACE_CONTEXT, * PDEVICE_INTERFACE_CONTEXT;

NTSTATUS CreateDeviceInterface(_In_ PDEVICE_CONTEXT ParentDeviceContext);
NTSTATUS ReleaseDeviceInterface(_In_ PDEVICE_CONTEXT ParentDeviceContext);

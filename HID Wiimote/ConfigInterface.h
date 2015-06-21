/*

Copyright (C) 2015 Julian Löhr
All rights reserved.

Filename:
ConfigInterface.h

Abstract:
Header file for ConfigInterface.c

*/

#ifndef _CONFIG_INTERFACE_H_
#define _CONFIG_INTERFACE_H_

#include "HIDWiimote.h"

typedef struct _CONFIG_INTERFACE_CONTEXT
{
	WDFQUEUE IoQueue;

} CONFIG_INTERFACE_CONTEXT, *PCONFIG_INTERFACE_CONTEXT;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL ConfigInterfaceIoDeviceControlCallback;

NTSTATUS CreateConfigInterfaceQueues(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS ReleaseConfigInterface(_In_ PDEVICE_CONTEXT DeviceContext);

#endif
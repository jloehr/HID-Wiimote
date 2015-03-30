/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Driver.c

Abstract:
	Contains the entry code for the Kernel Mode driver.

*/

#include "Driver.h"

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_DRIVER_CONFIG Config;

	// Initialize Config
	WDF_DRIVER_CONFIG_INIT(&Config, DeviceAdd);

	// Create Driver Object
	Status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &Config, WDF_NO_HANDLE);

	return Status;
}
/*

Copyright (C) 2015 Julian Löhr
All rights reserved.

Filename:
ConfigInterface.h

Abstract:
	Communication via the Interface with User Mode to configure Device

*/

#include "ConfigInterface.h"

#include "Device.h"
#include "RawDevice.h"

#define IOCTL_WIIMOTE_TEST CTL_CODE(FILE_DEVICE_KEYBOARD, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

NTSTATUS 
CreateConfigInterfaceQueues(_In_ PDEVICE_CONTEXT DeviceContext)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_IO_QUEUE_CONFIG IoQueueConfig;
	PCONFIG_INTERFACE_CONTEXT ConfigInterface = &(DeviceContext->ConfigInterfaceContext);

	WDF_IO_QUEUE_CONFIG_INIT(&IoQueueConfig, WdfIoQueueDispatchParallel);

	IoQueueConfig.EvtIoDeviceControl = ConfigInterfaceIoDeviceControlCallback;

	Status = WdfIoQueueCreate(DeviceContext->Device, &IoQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &ConfigInterface->IoQueue);
	if (!NT_SUCCESS(Status))
	{
		Trace("Failed to create Config Interface Queue: 0x%x\n", Status);
		return Status;
	}

	// Set it as Forward Queue for the Raw PDO
	DeviceContext->RawDeviceContext->ForwardQueue = ConfigInterface->IoQueue;

	return Status;
}

NTSTATUS 
ReleaseConfigInterface(_In_ PDEVICE_CONTEXT DeviceContext)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST TmpRequest;

	while (NT_SUCCESS(Status = (WdfIoQueueRetrieveNextRequest(DeviceContext->ConfigInterfaceContext.IoQueue, &TmpRequest))))
	{
		WdfRequestComplete(TmpRequest, STATUS_DEVICE_REMOVED);
	}

	if (Status == STATUS_NO_MORE_ENTRIES)
	{
		Status = STATUS_SUCCESS;
	}

	return Status;
}

VOID
ConfigInterfaceIoDeviceControlCallback(
IN WDFQUEUE      Queue,
IN WDFREQUEST    Request,
IN size_t        OutputBufferLength,
IN size_t        InputBufferLength,
IN ULONG         IoControlCode
)
{
	//NTSTATUS Status = STATUS_SUCCESS;
	//PDEVICE_CONTEXT DeviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	switch (IoControlCode)
	{
	case IOCTL_WIIMOTE_TEST:
		Trace("Hoi");
		WdfRequestComplete(Request, STATUS_SUCCESS);
		break;
	default:
		WdfRequestComplete(Request, STATUS_NOT_IMPLEMENTED);
	}
}
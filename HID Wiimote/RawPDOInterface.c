/*

Copyright (C) 2015 Julian Löhr
All rights reserved.

Filename:
RawPDOInterface.c

Abstract:
	Contains Code regarding the Raw PDO to open an interface from User Space

*/
#include "RawPDOInterface.h"

#include "Device.h"

static ULONG RawPDOInstanceCounter = 0;

NTSTATUS
CreateRawPDO(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG InstanceNumber = RawPDOInstanceCounter++;
	PWDFDEVICE_INIT DeviceInit = NULL;
	WDF_OBJECT_ATTRIBUTES Attributes;
	WDFDEVICE RawDevice;
	PRAW_DEVICE_CONTEXT RawDeviceContext;
	WDF_IO_QUEUE_CONFIG IncomingQueueConfig;
	WDF_DEVICE_STATE DeviceState;
	WDF_DEVICE_PNP_CAPABILITIES PnpCapabilities;
	WDFSTRING WdfString;
	UNICODE_STRING UnicodeString;
	DECLARE_CONST_UNICODE_STRING(DeviceID, HIDWIIMOTE_RAWPDO_DEVICE_ID);
	DECLARE_UNICODE_STRING_SIZE(StringBuffer, 512);
	DECLARE_CONST_UNICODE_STRING(DeviceLocation, L"HIDWiimote\0");


	// Assing all the Values!
	DeviceInit = WdfPdoInitAllocate(DeviceContext->Device);
	if (DeviceInit == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Status = WdfPdoInitAssignRawDevice(DeviceInit, &GUID_DEVCLASS_HIDWIIMOTE_RAWPDO);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Security Settings 
	Status = WdfDeviceInitAssignSDDLString(DeviceInit, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RWX_RES_RWX);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}
	
	// Device/Hardware ID
	Status = WdfPdoInitAssignDeviceID(DeviceInit, &DeviceID);
	if (!NT_SUCCESS(Status))
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Instance ID
	Status = RtlUnicodeStringPrintf(&StringBuffer, L"%02d", InstanceNumber);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}
	
	Status = WdfPdoInitAssignInstanceID(DeviceInit, &StringBuffer);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Default Device Name shown
	Status = RtlUnicodeStringPrintf(&StringBuffer, L"HID Wiimote Interface %02d", InstanceNumber);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	Status = WdfPdoInitAddDeviceText(DeviceInit, &StringBuffer,	&DeviceLocation, 0x409);
	if (!NT_SUCCESS(Status))
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	WdfPdoInitSetDefaultLocale(DeviceInit, 0x409);

	// Configure Device Context
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attributes, RAW_DEVICE_CONTEXT);

	WdfPdoInitAllowForwardingRequestToParent(DeviceInit);

	// Create Raw PDO
	Status = WdfDeviceCreate(&DeviceInit, &Attributes, &RawDevice);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Initialize Raw Context
	RawDeviceContext = GetRawDeviceContext(RawDevice);
	RawDeviceContext->RawDevice = RawDevice;
	RawDeviceContext->InstanceNumber = InstanceNumber;

	//Create IO Device Control Queue
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&IncomingQueueConfig, WdfIoQueueDispatchSequential);

	IncomingQueueConfig.EvtIoDeviceControl = RawPDODeviceControlCallback;

	Status = WdfIoQueueCreate(RawDevice, &IncomingQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &RawDeviceContext->IncomingQueue);
	if (!NT_SUCCESS(Status)) 
	{
		Trace("Failed to create IO Device Control Queue for Raw PDO: 0x%x\n", Status);
		return Status;
	}

	// Plug'n'Play Settings
	WDF_DEVICE_PNP_CAPABILITIES_INIT(&PnpCapabilities);

	PnpCapabilities.Removable = WdfTrue;
	PnpCapabilities.SurpriseRemovalOK = WdfTrue;
	PnpCapabilities.NoDisplayInUI = WdfTrue;

	PnpCapabilities.Address = InstanceNumber;
	PnpCapabilities.UINumber = InstanceNumber;

	WdfDeviceSetPnpCapabilities(RawDevice, &PnpCapabilities);

	//Hide it in Device Manager
	WDF_DEVICE_STATE_INIT(&DeviceState);
	DeviceState.DontDisplayInUI = WdfTrue;
	WdfDeviceSetDeviceState(RawDevice, &DeviceState);
	

	// Create Device Interface
	Status = WdfDeviceCreateDeviceInterface(RawDevice, &GUID_DEVINTERFACE_HIDWIIMOTE, NULL);
	if (!NT_SUCCESS(Status))
	{
		Trace("Failed to Create Device Interface for the Raw PDO");
		WdfDeviceInitFree(DeviceInit); 
		WdfObjectDelete(RawDevice);
		return Status;
	}

	Status = WdfStringCreate(NULL, WDF_NO_OBJECT_ATTRIBUTES, &WdfString);
	if (NT_SUCCESS(Status)) 
	{
		Status = WdfDeviceRetrieveDeviceInterfaceString(RawDevice, &GUID_DEVINTERFACE_HIDWIIMOTE, NULL, WdfString);
		if (!NT_SUCCESS(Status))
		{
			Trace("Failed to retrieve Interface String");
		}
		else
		{
			WdfStringGetUnicodeString(WdfString, &UnicodeString);
			Trace("Raw PDO Interface String: %wZ", UnicodeString);
		}
	}

	//	Make Static Child
	Status = WdfFdoAddStaticChild(DeviceContext->Device, RawDevice);
	if (!NT_SUCCESS(Status)) 
	{
		WdfDeviceInitFree(DeviceInit);
		WdfObjectDelete(RawDevice);
		return Status;
	}

	// Save the Raw Device Context in our normal Context
	DeviceContext->RawDeviceContext = RawDeviceContext;

	return STATUS_SUCCESS;
}

NTSTATUS 
ReleaseRawPDO(_In_ PDEVICE_CONTEXT DeviceContext)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST TmpRequest;

	//Mark our Raw PDO missing
	Status = WdfPdoMarkMissing(DeviceContext->RawDeviceContext->RawDevice);
	if (!NT_SUCCESS(Status))
	{
		Trace("Error marking Raw PDO missing");
	}

	//Complete remaining Requests
	while (NT_SUCCESS(Status = (WdfIoQueueRetrieveNextRequest(DeviceContext->RawDeviceContext->IncomingQueue, &TmpRequest))))
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
RawPDODeviceControlCallback(
IN WDFQUEUE      Queue,
IN WDFREQUEST    Request,
IN size_t        OutputBufferLength,
IN size_t        InputBufferLength,
IN ULONG         IoControlCode
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	//WDF_REQUEST_FORWARD_OPTIONS ForwardOptions;
	//PRAW_DEVICE_CONTEXT RawDeviceContext = GetRawDeviceContext(WdfIoQueueGetDevice(Queue));

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength); 
	//UNREFERENCED_PARAMETER(IoControlCode);
	
	Trace("Recieved IOCTL on Raw PDO: 0x%8X", IoControlCode);
	WdfRequestComplete(Request, Status);

	/*
	WDF_REQUEST_FORWARD_OPTIONS_INIT(&ForwardOptions);
	Status = WdfRequestForwardToParentDeviceIoQueue(Request, RawDeviceContext->ForwardQueue, &ForwardOptions);
	if (!NT_SUCCESS(Status))
	{
		WdfRequestComplete(Request, Status);
	}
	*/
}
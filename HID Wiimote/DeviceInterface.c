/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	DeviceInterface.c

Abstract:
	Contains code for the RawPDO used as Device Interface.
	Handles all incomming IOCTLs.

*/
#include "DeviceInterface.h"

#include "SettingsInterface.h"
#include "Device.h"

// {745a17a0-74d3-11d0-b6fe-00a0c90f57da}
DEFINE_GUID(GUID_DEVCLASS_HID,
	0x745a17a0, 0x74d3, 0x11d0, 0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda);


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_INTERFACE_CONTEXT, GetDeviceInterfaceContext);

NTSTATUS CreateSettingsInterfaceQueues(_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext);
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL SettingsInterfaceDeviceControlCallback;

NTSTATUS CreateDeviceInterface(_In_ PDEVICE_CONTEXT ParentDeviceContext)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWDFDEVICE_INIT DeviceInit;
	WDF_OBJECT_ATTRIBUTES Attributes;
	WDFDEVICE InterfaceDevice;
	PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext;
	WDF_DEVICE_PNP_CAPABILITIES PnpCapabilities;
	WDF_DEVICE_STATE DeviceState;

	DECLARE_CONST_UNICODE_STRING(DeviceID, L"{398128C0-7830-4B53-BA84-1E02F05EFFB0}\\HIDWiimoteInterface");
	DECLARE_CONST_UNICODE_STRING(InstanceID, L"0");
	DECLARE_CONST_UNICODE_STRING(DeviceText, L"HID Wiimote Settings Interface");
	DECLARE_CONST_UNICODE_STRING(DeviceLocation, L"Somewhere in Kernel Space");

	// Allocate and setup DeviceInit
	DeviceInit = WdfPdoInitAllocate(ParentDeviceContext->Device);
	if (DeviceInit == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Assing Device Class, what icon and category it will be shown in Device Manager
	Status = WdfPdoInitAssignRawDevice(DeviceInit, &GUID_DEVCLASS_HID);
	if (!NT_SUCCESS(Status))
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Device ID
	Status = WdfPdoInitAssignDeviceID(DeviceInit, &DeviceID);
	if (!NT_SUCCESS(Status))
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Instance ID (unique to its parent)
	Status = WdfPdoInitAssignInstanceID(DeviceInit, &InstanceID);
	if (!NT_SUCCESS(Status))
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Default Device Name and Location shown in Device Manager
	Status = WdfPdoInitAddDeviceText(DeviceInit, &DeviceText, &DeviceLocation, 0x409);
	if (!NT_SUCCESS(Status))
	{
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	WdfPdoInitSetDefaultLocale(DeviceInit, 0x409);

	// Set our Device Interface Context
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attributes, DEVICE_INTERFACE_CONTEXT);

	// Create the raw PDO
	Status = WdfDeviceCreate(&DeviceInit, &Attributes, &InterfaceDevice);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error creating raw PDO for Settings Interface", Status);
		WdfDeviceInitFree(DeviceInit);
		return Status;
	}

	// Initialize Raw Context
	DeviceInterfaceContext = GetDeviceInterfaceContext(InterfaceDevice);
	DeviceInterfaceContext->InterfaceDevice = InterfaceDevice;
	DeviceInterfaceContext->Parent = ParentDeviceContext;

	// Plug'n'Play Settings
	WDF_DEVICE_PNP_CAPABILITIES_INIT(&PnpCapabilities);

	PnpCapabilities.Removable = WdfTrue;
	PnpCapabilities.SurpriseRemovalOK = WdfTrue;
	PnpCapabilities.NoDisplayInUI = WdfTrue;

	WdfDeviceSetPnpCapabilities(InterfaceDevice, &PnpCapabilities);

	//Hide it in Device Manager
	WDF_DEVICE_STATE_INIT(&DeviceState);
	DeviceState.DontDisplayInUI = WdfTrue;
	WdfDeviceSetDeviceState(InterfaceDevice, &DeviceState);

	// Create Device Interface
	Status = WdfDeviceCreateDeviceInterface(InterfaceDevice, &GUID_DEVINTERFACE_HIDWIIMOTE, NULL);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error creating raw PDO for Settings Interface", Status);
		WdfObjectDelete(InterfaceDevice);
		return Status;
	}

	// Make Static Child
	Status = WdfFdoAddStaticChild(ParentDeviceContext->Device, InterfaceDevice);
	if (!NT_SUCCESS(Status))
	{
		WdfObjectDelete(InterfaceDevice);
		return Status;
	}

	// Everything is fine, so save the Interface Context in our parent/main context
	ParentDeviceContext->SettingsInterfaceContext = DeviceInterfaceContext;

	// Create Queues
	Status = CreateSettingsInterfaceQueues(DeviceInterfaceContext);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error Creating Settings Interface Queues", Status);
		return Status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS CreateSettingsInterfaceQueues(_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_IO_QUEUE_CONFIG DefaultQueueConfig; 
	
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&DefaultQueueConfig, WdfIoQueueDispatchSequential);

	DefaultQueueConfig.EvtIoDeviceControl = SettingsInterfaceDeviceControlCallback;

	Status = WdfIoQueueCreate(DeviceInterfaceContext->InterfaceDevice, &DefaultQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &DeviceInterfaceContext->DefaultIOQueue);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Failed to create Default IO Queue for Settings Interface Device", Status);
		return Status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS ReleaseDeviceInterface(_In_ PDEVICE_CONTEXT ParentDeviceContext)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ParentDeviceContext);

	return Status;
}

VOID
SettingsInterfaceDeviceControlCallback(
	IN WDFQUEUE      Queue,
	IN WDFREQUEST    Request,
	IN size_t        OutputBufferLength,
	IN size_t        InputBufferLength,
	IN ULONG         IoControlCode
	)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);
	UNREFERENCED_PARAMETER(IoControlCode);

	switch (IoControlCode)
	{
	case IOCTL_WIIMOTE_TEST:
		Trace("Test IOCTL: %#010x", IoControlCode);
		WdfRequestComplete(Request, STATUS_SUCCESS);
		break;
	default:
		Trace("Something Else: %#010x", IoControlCode);
		WdfRequestComplete(Request, STATUS_NOT_IMPLEMENTED);
	}
}

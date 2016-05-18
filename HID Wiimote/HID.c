/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	HID.c

Abstract:
	Contains IOQueues and HID specific functions.
	Handles all HID Requests and translates the Wiimote State into HID reports.
*/
#include "HID.h"

#include "Device.h"
#include "Bluetooth.h"
#include "HIDDescriptors.h"
#include "WiimoteToHIDParser.h"

EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL HIDInternalDeviceControlCallback;
EVT_READ_IO_CONTROL_BUFFER_FILL_BUFFER HIDFillReadBufferCallback;

VOID ProcessGetDeviceDescriptor(_In_ WDFREQUEST Request);
VOID ProcessGetReportDescriptor(_In_ WDFREQUEST Request);
VOID ProcessGetDeviceAttributes(_In_ WDFREQUEST Request,_In_ PHID_DEVICE_CONTEXT HIDContext);
VOID ForwardReadReportRequest(_In_ WDFREQUEST Request, _In_ PDEVICE_CONTEXT DeviceContext);
VOID ProcessAddresses(_In_ WDFREQUEST Request, _In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS HIDPrepare(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	Status = GetVendorAndProductID(DeviceContext->IoTarget, &(DeviceContext->HIDContext.VendorID), &(DeviceContext->HIDContext.ProductID));
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS 
HIDCreateQueues(
	_In_ WDFDEVICE Device, 
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status;
	WDF_IO_QUEUE_CONFIG QueueConfig;
	PHID_DEVICE_CONTEXT HIDContext = &DeviceContext->HIDContext;

	// Create Default Queue
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&QueueConfig, WdfIoQueueDispatchParallel);
	QueueConfig.EvtIoInternalDeviceControl = HIDInternalDeviceControlCallback;

	Status = WdfIoQueueCreate(Device, &QueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &(HIDContext->DefaultIOQueue));
	if(!NT_SUCCESS(Status))
	{
		TraceStatus("Creating DefaultIOQueue failed", Status);
		return Status;
	}

	// Create Read Buffer Queue
	Status = ReadIoControlBufferCreate(&HIDContext->ReadBuffer, DeviceContext->Device, DeviceContext, HIDFillReadBufferCallback, sizeof(HID_GAMEPAD_REPORT));
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Creating HID Read Buffer failed", Status);
		return Status;
	}

	return Status;
}

PHID_DEVICE_CONTEXT
GetHIDContext(
	_In_ WDFQUEUE Queue
	)
{
	return &(GetDeviceContext(WdfIoQueueGetDevice(Queue))->HIDContext);
}

VOID
HIDInternalDeviceControlCallback(
	_In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
	)
{
	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	switch(IoControlCode)
	{
	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		ProcessGetDeviceDescriptor(Request);
		break;
	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		ProcessGetDeviceAttributes(Request, GetHIDContext(Queue));
		break;
	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		ProcessGetReportDescriptor(Request);
		break;
	case IOCTL_HID_READ_REPORT:
		ForwardReadReportRequest(Request, GetDeviceContext(WdfIoQueueGetDevice(Queue)));
		break;
	case IOCTL_WIIMOTE_ADDRESSES:
		ProcessAddresses(Request, GetDeviceContext(WdfIoQueueGetDevice(Queue)));
		break;
	default:
		Trace("Unknown IOCTL: %x", IoControlCode);
		WdfRequestComplete(Request, STATUS_NOT_SUPPORTED);
	}
}

VOID
ProcessGetDeviceDescriptor(
	_In_ WDFREQUEST Request
	)
{
	NTSTATUS Status;
    WDFMEMORY Memory;
	
	// Get Memory
	Status = WdfRequestRetrieveOutputMemory(Request, &Memory);
	if(!NT_SUCCESS(Status))
	{
		WdfRequestComplete(Request, Status);
		return;
	}

	// Copy 
	Status = WdfMemoryCopyFromBuffer(Memory, 0, (PVOID)&HIDDescriptor, HIDDescriptor.bLength);
	if(!NT_SUCCESS(Status))
	{
		WdfRequestComplete(Request, Status);
		return;
	}

	// Complete Request
	WdfRequestCompleteWithInformation(Request, Status, HIDDescriptor.bLength);
}

VOID
ProcessGetDeviceAttributes(
	_In_ WDFREQUEST Request,
	_In_ PHID_DEVICE_CONTEXT HIDContext
	)
{
	NTSTATUS Status;
    PHID_DEVICE_ATTRIBUTES DeviceAttributes = NULL;
	size_t DeviceAttributesSize = sizeof(HID_DEVICE_ATTRIBUTES);

	// Get Buffer
	Status = WdfRequestRetrieveOutputBuffer(Request, DeviceAttributesSize, (PVOID *)&DeviceAttributes, NULL);
	if(!NT_SUCCESS(Status))
	{
		WdfRequestComplete(Request, Status);
		return;
	}

	// Fill out
	DeviceAttributes->Size = (ULONG)DeviceAttributesSize;
	DeviceAttributes->VendorID = HIDContext->VendorID;
	DeviceAttributes->ProductID = HIDContext->ProductID;
	DeviceAttributes->VersionNumber = 0x0000;
	
	// Complete Request
	WdfRequestCompleteWithInformation(Request, Status, DeviceAttributesSize);
}

VOID
ProcessGetReportDescriptor(
	_In_ WDFREQUEST Request
	)
{
	NTSTATUS Status;
    WDFMEMORY Memory;
	size_t ReportDescriptorSize = HIDReportDescriptorSize;

	// Get Memory
	Status = WdfRequestRetrieveOutputMemory(Request, &Memory);
	if(!NT_SUCCESS(Status))
	{
		WdfRequestComplete(Request, Status);
		return;
	}

	// Copy 
	Status = WdfMemoryCopyFromBuffer(Memory, 0, (PVOID)&HIDReportDescriptor, ReportDescriptorSize);
	if(!NT_SUCCESS(Status))
	{
		WdfRequestComplete(Request, Status);
		return;
	}

	// Complete Request
	WdfRequestCompleteWithInformation(Request, Status, ReportDescriptorSize);
}

VOID 
ForwardReadReportRequest(
	_In_ WDFREQUEST Request,
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	ReadIoControlBufferForwardRequest(&DeviceContext->HIDContext.ReadBuffer, Request);
}


NTSTATUS 
HIDRelease(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PHID_DEVICE_CONTEXT HIDContext = &(DeviceContext->HIDContext);

	ReadIoControlBufferFlush(&HIDContext->ReadBuffer);

	return Status;
}

VOID 
HIDFillReadBufferCallback(
	_In_ PDEVICE_CONTEXT DeviceContext, 
	_Inout_updates_all_(BufferSize) PVOID Buffer,
	_In_ size_t BufferSize, 
	_Out_ PSIZE_T BytesWritten)
{
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext = &(DeviceContext->WiimoteContext);
	BYTE * RequestBuffer = (PUCHAR)Buffer;

	RequestBuffer[0] = GAMEPAD_REPORT_ID;
#if defined MOUSE_DPAD
	ParseWiimoteStateAsDPadMouse(WiimoteContext, RequestBuffer + 1);
#else
#if defined MOUSE_IR
	ParseWiimoteStateAsIRMouse(WiimoteContext, RequestBuffer + 1);
#else 
	ParseWiimoteState(WiimoteContext, RequestBuffer + 1);
#endif
#endif

	(*BytesWritten) = BufferSize;
}

VOID 
HIDWiimoteStateUpdated(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	ReadIoControlBufferDispatchRequest(&(DeviceContext->HIDContext.ReadBuffer));
}

VOID
ProcessAddresses(
	_In_ WDFREQUEST Request,
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PHID_MINIPORT_ADDRESSES Addresses = NULL;

	Trace("Adresses recieved");

	Status = WdfRequestRetrieveInputBuffer(Request, sizeof(HID_MINIPORT_ADDRESSES), (PVOID *)&Addresses, NULL);
	if(!NT_SUCCESS(Status))
	{
		TraceStatus("Couldn't retrieve Input Buffer", Status);
		WdfRequestComplete(Request, STATUS_SUCCESS);
		return;
	}

	SetHIDMiniportAddresses(DeviceContext, Addresses);

	WdfRequestComplete(Request, STATUS_SUCCESS);
}

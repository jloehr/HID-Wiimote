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
#include "WiimoteSettings.h"

typedef VOID DRIVER_MODE_SETTING_SETTER(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WIIMOTE_DRIVER_MODE Value);
typedef VOID BOOLEAN_SETTING_SETTER(_In_ PDEVICE_CONTEXT DeviceContext, _In_ BOOLEAN Value);


// {7D180E63-2CAC-4112-B3D4-C42275CA497E}
DEFINE_GUID(GUID_DEVCLASS_HIDWIIMOTE,
	0x7d180e63, 0x2cac, 0x4112, 0xb3, 0xd4, 0xc4, 0x22, 0x75, 0xca, 0x49, 0x7e);

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_INTERFACE_CONTEXT, GetDeviceInterfaceContext);

NTSTATUS CreateSettingsInterfaceQueues(_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext);
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL DeviceInterfaceDeviceControlCallback;
EVT_READ_IO_CONTROL_BUFFER_FILL_BUFFER DeviceInterfaceFillReadBufferCallback;

VOID ProcessGetState(_In_ WDFREQUEST Request, _In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext);
VOID ForwardReadStatusRequest(_In_ WDFREQUEST Request, _In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext);
VOID ProcessSetDriverModeSetting(_In_ WDFREQUEST Request, _In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext, _In_ DRIVER_MODE_SETTING_SETTER DriverModeSettingSetter);
VOID ProcessSetBooleanSetting(_In_ WDFREQUEST Request, _In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext, _In_ BOOLEAN_SETTING_SETTER BooleanSettingSetter);

VOID FillStateIoControlData(_In_ PWIIMOTE_STATE_IOCTL_DATA StateData, _In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext);
VOID FillStatusIoControlData(_In_ PWIIMOTE_STATUS_IOCTL_DATA StatusData, _In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext);

NTSTATUS DeviceInterfaceCreate(_In_ PDEVICE_CONTEXT ParentDeviceContext)
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

	// Assing custom Device Class
	Status = WdfPdoInitAssignRawDevice(DeviceInit, &GUID_DEVCLASS_HIDWIIMOTE);
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
	
	// Default IO Queue
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&DefaultQueueConfig, WdfIoQueueDispatchSequential);

	DefaultQueueConfig.EvtIoDeviceControl = DeviceInterfaceDeviceControlCallback;

	Status = WdfIoQueueCreate(DeviceInterfaceContext->InterfaceDevice, &DefaultQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &DeviceInterfaceContext->DefaultIOQueue);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Failed to create Default IO Queue for Settings Interface Device", Status);
		return Status;
	}

	// Read Buffer Queue
	Status = ReadIoControlBufferCreate(
		&DeviceInterfaceContext->ReadBuffer, 
		DeviceInterfaceContext->InterfaceDevice, 
		DeviceInterfaceContext->Parent, 
		DeviceInterfaceFillReadBufferCallback,
		sizeof(WIIMOTE_STATUS_IOCTL_DATA));
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Creating Device Interface Read Buffer failed", Status);
		return Status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS 
DeviceInterfaceRelease(
	_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	ReadIoControlBufferFlush(&DeviceInterfaceContext->ReadBuffer);

	return Status;
}

VOID DeviceInterfaceWiimoteStateUpdated(_In_ PDEVICE_CONTEXT DeviceContext)
{
	ReadIoControlBufferDispatchRequest(&DeviceContext->SettingsInterfaceContext->ReadBuffer);
}

VOID
DeviceInterfaceDeviceControlCallback(
	IN WDFQUEUE      Queue,
	IN WDFREQUEST    Request,
	IN size_t        OutputBufferLength,
	IN size_t        InputBufferLength,
	IN ULONG         IoControlCode
	)
{
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext = GetDeviceInterfaceContext(WdfIoQueueGetDevice(Queue));

	switch (IoControlCode)
	{
	case IOCTL_WIIMOTE_GET_STATE:
		ProcessGetState(Request, DeviceInterfaceContext);
		break;
	case IOCTL_WIIMOTE_READ_STATUS:
		ForwardReadStatusRequest(Request, DeviceInterfaceContext);
		break;
	case IOCTL_WIIMOTE_SET_MODE:
		ProcessSetDriverModeSetting(Request, DeviceInterfaceContext, WiimoteSettingsSetDriverMode);
		break;
	case IOCTL_WIIMOTE_SET_XAXIS:
		ProcessSetBooleanSetting(Request, DeviceInterfaceContext, WiimoteSettingsSetXAxisEnabled);
		break;
	case IOCTL_WIIMOTE_SET_YAXIS:
		ProcessSetBooleanSetting(Request, DeviceInterfaceContext, WiimoteSettingsSetYAxisEnabled);
		break;
	case IOCTL_WIIMOTE_SET_MOUSEBUTTONSWITCHED:
		ProcessSetBooleanSetting(Request, DeviceInterfaceContext, WiimoteSettingsSetMouseButtonsSwitched);
		break;
	case IOCTL_WIIMOTE_SET_TRIGGERAMDSHOULDERSWITCHED:
		ProcessSetBooleanSetting(Request, DeviceInterfaceContext, WiimoteSettingsSetTriggerAndShoulderSwitched);
		break;
	case IOCTL_WIIMOTE_SET_TRIGGERSPLIT:
		ProcessSetBooleanSetting(Request, DeviceInterfaceContext, WiimoteSettingsSetTriggerSplit);
		break;
	default:
		Trace("Devcice Interface recieved unknown IOCTL: %#010x", IoControlCode);
		WdfRequestComplete(Request, STATUS_NOT_IMPLEMENTED);
	}
}

VOID 
ProcessGetState(
	_In_ WDFREQUEST Request,
	_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_STATE_IOCTL_DATA StateData = NULL;
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext = &(DeviceInterfaceContext->Parent->WiimoteContext);

	Trace("Processing IOCTL_WIIMOTE_GET_STATE");

	Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(WIIMOTE_STATE_IOCTL_DATA), &StateData, NULL);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error retrieving State Buffer", Status);
		WdfRequestComplete(Request, Status);
		return;
	}

	FillStateIoControlData(StateData, WiimoteContext);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, sizeof(WIIMOTE_STATE_IOCTL_DATA));
}

VOID 
ForwardReadStatusRequest(
	_In_ WDFREQUEST Request, 
	_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext
	)
{
	ReadIoControlBufferForwardRequest(&DeviceInterfaceContext->ReadBuffer, Request);
}

VOID
DeviceInterfaceFillReadBufferCallback(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_Inout_updates_all_(BufferSize) PVOID Buffer,
	_In_ size_t BufferSize, 
	_Out_ PSIZE_T BytesWritten)
{
	UNREFERENCED_PARAMETER(BufferSize);

	FillStatusIoControlData((PWIIMOTE_STATUS_IOCTL_DATA)Buffer, &DeviceContext->WiimoteContext);

	(*BytesWritten) = sizeof(WIIMOTE_STATUS_IOCTL_DATA);
}

VOID
FillStateIoControlData(
	_In_ PWIIMOTE_STATE_IOCTL_DATA StateData,
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext
	)
{
	RtlZeroMemory(StateData, sizeof(WIIMOTE_STATE_IOCTL_DATA));

	StateData->Mode = WiimoteContext->Mode;
	StateData->Settings = WiimoteContext->Settings;

	FillStatusIoControlData(&(StateData->Status), WiimoteContext);
}

VOID
FillStatusIoControlData(
	_In_ PWIIMOTE_STATUS_IOCTL_DATA StatusData,
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext
	)
{
	RtlZeroMemory(StatusData, sizeof(WIIMOTE_STATUS_IOCTL_DATA));

	StatusData->Extension = WiimoteContext->Extension;
	StatusData->BatteryLevel = WiimoteContext->BatteryLevel;
	StatusData->LEDs = WiimoteContext->LEDState;
}
VOID ProcessSetDriverModeSetting(
	_In_ WDFREQUEST Request, 
	_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext, 
	_In_ DRIVER_MODE_SETTING_SETTER DriverModeSettingSetter
	)
{
	NTSTATUS Status;
	PWIIMOTE_DRIVER_MODE RequestedMode;

	Status = WdfRequestRetrieveInputBuffer(Request, sizeof(WIIMOTE_DRIVER_MODE), &RequestedMode, NULL);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error retrieving Driver Mode Io Control Mode Input Buffer", Status);
		WdfRequestComplete(Request, Status);
		return;
	}

	DriverModeSettingSetter(DeviceInterfaceContext->Parent, *RequestedMode);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, sizeof(WIIMOTE_DRIVER_MODE));
}

VOID ProcessSetBooleanSetting(
	_In_ WDFREQUEST Request, 
	_In_ PDEVICE_INTERFACE_CONTEXT DeviceInterfaceContext, 
	_In_ BOOLEAN_SETTING_SETTER BooleanSettingSetter
	)
{
	NTSTATUS Status;
	PBOOLEAN RequestedValue;

	Status = WdfRequestRetrieveInputBuffer(Request, sizeof(BOOLEAN), &RequestedValue, NULL);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error retrieving Driver Mode Io Control Mode Input Buffer", Status);
		WdfRequestComplete(Request, Status);
		return;
	}

	BooleanSettingSetter(DeviceInterfaceContext->Parent, *RequestedValue);

	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, sizeof(BOOLEAN));

}

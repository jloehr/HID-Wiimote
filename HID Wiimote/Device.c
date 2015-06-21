/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Device.c

Abstract:
	Contains all system callbacks regarding a devices pnp and power states.
*/


#include "Device.h"

#include "RawPDOInterface.h"

NTSTATUS
DeviceAdd(
	_In_    WDFDRIVER		Driver,
	_Inout_ PWDFDEVICE_INIT	DeviceInit
    )
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_OBJECT_ATTRIBUTES Attributes;
	WDF_PNPPOWER_EVENT_CALLBACKS PnpPowerCallbacks;
	WDFDEVICE Device;
	PDEVICE_CONTEXT DevContext;

	UNREFERENCED_PARAMETER(Driver);

	Trace("Device Added");

	// Driver is filter
	WdfFdoInitSetFilter(DeviceInit);

	// Configure PnP Functions
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&PnpPowerCallbacks);

	PnpPowerCallbacks.EvtDevicePrepareHardware = PrepareHardware;
	PnpPowerCallbacks.EvtDeviceReleaseHardware = ReleaseHardware;

	PnpPowerCallbacks.EvtDeviceD0Entry = DeviceD0Entry;
	PnpPowerCallbacks.EvtDeviceD0Exit = DeviceD0Exit;

	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &PnpPowerCallbacks);

	// Configure Device Context
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&Attributes, DEVICE_CONTEXT); 

	//Create Device Object
	Status = WdfDeviceCreate(&DeviceInit, &Attributes, &Device);
	
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Initialize DeviceContext
	DevContext = GetDeviceContext(Device);
	DevContext->Device = Device;

	// Create IO Queue
	Status = CreateQueues(Device, &(DevContext->HIDContext));
	if(!NT_SUCCESS(Status))
	{
		Trace("Device Added Error On CreateQueues Result: 0x%x", Status);
		return Status;
	}

	Status = CreateRawPDO(DevContext); 
	if (!NT_SUCCESS(Status))
	{
		Trace("Failed to Create Raw PDO: 0x%x", Status);
		return Status;
	}
	
	Trace("Device Added Result: %#02X", Status);

	return Status;
}

NTSTATUS
PrepareHardware(
	_In_  WDFDEVICE Device,
	_In_  WDFCMRESLIST ResourcesRaw,
	_In_  WDFCMRESLIST ResourcesTranslated
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;

	UNREFERENCED_PARAMETER(ResourcesRaw);
	UNREFERENCED_PARAMETER(ResourcesTranslated);
	
	Trace("PrepareHardware");
	
	DeviceContext = GetDeviceContext(Device);
	DeviceContext->IoTarget = WdfDeviceGetIoTarget(Device);

	//Initialize Bluetooth
	Status = PrepareBluetooth(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	
	//Initialize HID
	Status = PrepareHID(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	
	//Initialize Wiimote
	Status = PrepareWiimote(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	
	Trace("PrepareHardware Result: 0x%x", Status);

	return Status;
}

NTSTATUS 
DeviceD0Entry(
	_In_  WDFDEVICE Device,
	_In_  WDF_POWER_DEVICE_STATE PreviousState
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;

	UNREFERENCED_PARAMETER(PreviousState);
	
	Trace("Device D0 Entry");

	DeviceContext = GetDeviceContext(Device);

	RtlSecureZeroMemory(&(DeviceContext->HIDMiniportAddresses), sizeof(HID_MINIPORT_ADDRESSES));

	Status = OpenChannels(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	Trace("Device D0 Entry Result: 0x%x", Status);

	return Status;
}

NTSTATUS 
DeviceD0Exit(
	_In_  WDFDEVICE Device,
	_In_  WDF_POWER_DEVICE_STATE TargetState
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;

	UNREFERENCED_PARAMETER(TargetState);

	Trace("Exit D0; Target: %x", TargetState);
	
	DeviceContext = GetDeviceContext(Device);

	//Suspend Wiimote
	Status = StopWiimote(DeviceContext);
	if (!NT_SUCCESS(Status))
	{
		Trace("Error Stopping Wiimote");
	}

	//Close BluetoothConnection
	Status = CloseChannels(DeviceContext);
	if (!NT_SUCCESS(Status))
	{
		Trace("Error Closing Bluetooth Channels");
	}
	
	Trace("Exit D0 Result: 0x%x", Status);

	return Status;
}

NTSTATUS
ReleaseHardware(
	_In_  WDFDEVICE Device,
	_In_  WDFCMRESLIST ResourcesTranslated
)
{ 
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;
	
	UNREFERENCED_PARAMETER(ResourcesTranslated);

	Trace("Releasee Hardware");

	DeviceContext = GetDeviceContext(Device);

	Status = ReleaseHID(DeviceContext);
	if (!NT_SUCCESS(Status))
	{
		Trace("Error Releasing HID Resources");
	}

	Status = ReleaseRawPDO(DeviceContext);
	if (!NT_SUCCESS(Status))
	{
		Trace("Error Releasing Raw PDO Resources");
	}

	Trace("Releasee Hardware Result: 0x%x", Status);

	return Status;
}


VOID 
SetHIDMiniportAddresses(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ PHID_MINIPORT_ADDRESSES Addresses
	)
{
	DeviceContext->HIDMiniportAddresses.FDO = Addresses->FDO;
	DeviceContext->HIDMiniportAddresses.HidNotifyPresence = Addresses->HidNotifyPresence;
}

NTSTATUS
SignalDeviceIsGone(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	if((DeviceContext->HIDMiniportAddresses.FDO != NULL) && (DeviceContext->HIDMiniportAddresses.HidNotifyPresence != NULL))
	{
		Status = (DeviceContext->HIDMiniportAddresses.HidNotifyPresence)((DeviceContext->HIDMiniportAddresses.FDO), FALSE);
		Trace("Signaling Device is Gone: 0x%x", Status);
	}

	return Status;
}
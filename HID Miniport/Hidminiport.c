/*

Copyright (C) 2014 Julian Löhr
All rights reserved.

Filename:
	hidminiport.c

Abstract:
	Actual miniport driver for HIDClass. Passes through all IRPs. Additonally sends down a custiom IOCTL, with the actual FDO and HidNotifyPrcense address.

*/

#include "hidminiport.h"

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status;
	HID_MINIDRIVER_REGISTRATION HIDMinidriverRegistration;
    ULONG i;

	//Trace("Driver Entry");

	//Set Function Pointers for IRPs
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = PassThrough;
    }

	//Special for PnP
	DriverObject->MajorFunction[IRP_MJ_PNP] = PnPPassThrough;

    DriverObject->DriverExtension->AddDevice = AddDevice;
    DriverObject->DriverUnload = Unload;

	//Register as HID Minidriver
    RtlZeroMemory(&HIDMinidriverRegistration, sizeof(HIDMinidriverRegistration));

    HIDMinidriverRegistration.Revision            = HID_REVISION;
    HIDMinidriverRegistration.DriverObject        = DriverObject;
    HIDMinidriverRegistration.RegistryPath        = RegistryPath;
    HIDMinidriverRegistration.DeviceExtensionSize = 0;
    HIDMinidriverRegistration.DevicesArePolled = FALSE;

    Status = HidRegisterMinidriver(&HIDMinidriverRegistration);
    if (!NT_SUCCESS(Status) ){
        return Status;
    }

    return Status;
}


NTSTATUS
AddDevice(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PDEVICE_OBJECT FunctionalDeviceObject
    )
{
	PAGED_CODE();

    UNREFERENCED_PARAMETER(DriverObject);
	
	//Trace("Add Device");

    FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}

NTSTATUS
PassThrough(
    _In_    PDEVICE_OBJECT  DeviceObject,
    _Inout_ PIRP            Irp
    )
{
	//Trace("Pass Through");

    IoCopyCurrentIrpStackLocationToNext(Irp);
    return IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
}

/*
NTSTATUS SendAddressesCompeted(
  _In_      PDEVICE_OBJECT DeviceObject,
  _In_      PIRP Irp,
  _In_opt_  PVOID Context
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Trace("Send Addresses Completed");

	if(Context != NULL)
	{
		ExFreePoolWithTag(Context, IOCTL_POOL_TAG);
	}

	IoFreeIrp(Irp);

	return STATUS_SUCCESS;
}
*/

NTSTATUS
SendAddresses(
    _In_    PDEVICE_OBJECT  DeviceObject
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PIRP NewIrp;
	PIO_STATUS_BLOCK StatusBlock = NULL;
	HID_MINIPORT_ADDRESSES AddressesBuffer;

	UNREFERENCED_PARAMETER(DeviceObject);

	Trace("Sending Addresses!");

	/*AddressesBuffer = (PHID_MINIPORT_ADDRESSES)ExAllocatePoolWithTag(NonPagedPool, sizeof(HID_MINIPORT_ADDRESSES), IOCTL_POOL_TAG);
	if(AddressesBuffer == NULL)
	{
		Trace("Couldn't allocate Addresses Buffer");
		return STATUS_SUCCESS;
	}
	
	RtlSecureZeroMemory(AddressesBuffer, sizeof(HID_MINIPORT_ADDRESSES));
	*/
	AddressesBuffer.FDO = DeviceObject;
	AddressesBuffer.HidNotifyPresence = HidNotifyPresence;

	NewIrp = IoBuildDeviceIoControlRequest(IOCTL_WIIMOTE_ADDRESSES, GET_NEXT_DEVICE_OBJECT(DeviceObject), &AddressesBuffer, sizeof(HID_MINIPORT_ADDRESSES), NULL, 0, TRUE, NULL, StatusBlock);
	if(NewIrp == NULL)
	{
		Trace("Couldn't Build IRP");
		return Status;
	}

	//IoSetCompletionRoutine(NewIrp, SendAddressesCompeted, AddressesBuffer, TRUE, TRUE, TRUE);

	Status = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), NewIrp);
	if(!NT_SUCCESS(Status))
	{
		if(Status == STATUS_PENDING)
		{
			Trace("SendAddress IoCallDriver is Pending!");
		}
		else
		{
			Trace("SendAddress IoCallDriver: 0x%x", Status);
		}

		return Status;
	}
	
	
	Trace("Sending Addresses returned!");

	return Status;
}

NTSTATUS
PnPPassThrough(
    _In_    PDEVICE_OBJECT  DeviceObject,
    _Inout_ PIRP            Irp
    )
{
	NTSTATUS Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION StackLocation = IoGetCurrentIrpStackLocation(Irp);
	UCHAR MinorFunction = StackLocation->MinorFunction;
	//Trace("MinorFunction: %x", MinorFunction);

    IoCopyCurrentIrpStackLocationToNext(Irp);
    Status = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	
	if(MinorFunction == IRP_MN_START_DEVICE)
	{
		Trace("Device Start");

		Status = SendAddresses(DeviceObject);
		if(!NT_SUCCESS(Status))
		{
			return Status;
		}
	}

	return Status;
}



VOID
Unload(
    _In_ PDRIVER_OBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE ();

    return;
}


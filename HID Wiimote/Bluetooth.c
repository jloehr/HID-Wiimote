/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Bluetooth.c

Abstract:
	Contains all Bluetooth relevant functions.
	Like establishing the connection, reading and writing,
	closing the connection to the device and Bluetooth error handling.
*/

#include "Bluetooth.h"
#include "Device.h"

NTSTATUS 
GetVendorAndProductID(
	_In_ WDFIOTARGET IoTarget, 
	_Out_ USHORT * VendorID, 
	_Out_ USHORT * ProductID
	)
{	
    NTSTATUS Status = STATUS_SUCCESS;
	WDF_MEMORY_DESCRIPTOR EnumInfoMemDescriptor;
	BTH_ENUMERATOR_INFO EnumInfo;

	RtlZeroMemory(&EnumInfo, sizeof(EnumInfo));
	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&EnumInfoMemDescriptor, &EnumInfo, sizeof(EnumInfo));

	Status = WdfIoTargetSendInternalIoctlSynchronously(
		IoTarget, 
		NULL, 
		IOCTL_INTERNAL_BTHENUM_GET_ENUMINFO, 
		NULL, 
		&EnumInfoMemDescriptor, 
		NULL, 
		NULL);
	 
    if (!NT_SUCCESS(Status)) 
	{
		return Status;
    }

	(*ProductID) = EnumInfo.Pid;
	(*VendorID) = EnumInfo.Vid;

	return Status;
}

NTSTATUS 
PrepareBluetooth(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
    NTSTATUS Status = STATUS_SUCCESS;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);
	WDF_MEMORY_DESCRIPTOR  DeviceInfoMemDescriptor;
	BTH_DEVICE_INFO	DeviceInfo;

	BluetoothContext->ControlChannelHandle = NULL;
	BluetoothContext->InterruptChannelHandle = NULL;

	//Get Interfaces
	Status = WdfFdoQueryForInterface(
		DeviceContext->Device, 
		&GUID_BTHDDI_PROFILE_DRIVER_INTERFACE, 
		(PINTERFACE)(&(BluetoothContext->ProfileDriverInterface)), 
		sizeof(BluetoothContext->ProfileDriverInterface), 
		BTHDDI_PROFILE_DRIVER_INTERFACE_VERSION_FOR_QI, 
		NULL); 
	
	if (!NT_SUCCESS(Status))
    {
        return Status;
    }

	//Get BluetoothAdress
	RtlZeroMemory(&DeviceInfo, sizeof(DeviceInfo));
	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&DeviceInfoMemDescriptor, &DeviceInfo, sizeof(DeviceInfo));

	Status = WdfIoTargetSendInternalIoctlSynchronously(
		DeviceContext->IoTarget, 
		NULL, 
		IOCTL_INTERNAL_BTHENUM_GET_DEVINFO, 
		NULL, 
		&DeviceInfoMemDescriptor, 
		NULL, 
		NULL);
	 
    if (!NT_SUCCESS(Status)) 
	{
		return Status;
    }

	BluetoothContext->DeviceAddress = DeviceInfo.address;

	return Status;
}

NTSTATUS
CreateRequest(
	_In_ WDFDEVICE Device,
	_In_ WDFIOTARGET IoTarget,
	_Out_ WDFREQUEST * Request
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES Attributes;

    WDF_OBJECT_ATTRIBUTES_INIT(&Attributes);
    Attributes.ParentObject = Device;

	Status = WdfRequestCreate(&Attributes, IoTarget, Request);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
CreateBuffer(
	_In_ WDFREQUEST Request,
	_In_ size_t BufferSize,
	_Out_ WDFMEMORY * Memory,
	_Out_opt_ PVOID * Buffer
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES Attributes;

	WDF_OBJECT_ATTRIBUTES_INIT(&Attributes);
	Attributes.ParentObject = Request;
	
	Status = WdfMemoryCreate(&Attributes, NonPagedPool, BUFFER_POOL_TAG, BufferSize, Memory, Buffer);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
CreateRequestAndBuffer(
	_In_ WDFDEVICE Device,
	_In_ WDFIOTARGET IoTarget,
	_In_ size_t BufferSize,
	_Out_ WDFREQUEST * Request,
	_Out_ WDFMEMORY * Memory,
	_Out_opt_ PVOID * Buffer
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	Status = CreateRequest(Device, IoTarget, Request);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	Status = CreateBuffer((*Request), BufferSize, Memory, Buffer);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(*Request);
		(*Request) = NULL;
		return Status;
	}

	return Status;

}

NTSTATUS
PrepareRequest(
	_In_ WDFIOTARGET IoTarget,
	_In_ PBRB BRB,
	_In_ WDFREQUEST Request
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
    WDF_OBJECT_ATTRIBUTES MemoryAttributes;
    WDFMEMORY Memory = NULL;

    WDF_OBJECT_ATTRIBUTES_INIT(&MemoryAttributes);
    MemoryAttributes.ParentObject = Request;

	Status = WdfMemoryCreatePreallocated(
        &MemoryAttributes,
        BRB,
		sizeof(*BRB),
        &Memory
        );

	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
		
	Status = WdfIoTargetFormatRequestForInternalIoctlOthers(
        IoTarget,
        Request,
        IOCTL_INTERNAL_BTH_SUBMIT_BRB,
        Memory, //OtherArg1
        NULL, //OtherArg1Offset
        NULL, //OtherArg2
        NULL, //OtherArg2Offset
        NULL, //OtherArg4
        NULL  //OtherArg4Offset
        );
	
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
SendBRB(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_opt_ WDFREQUEST OptRequest,
	_In_ PBRB BRB,
	_In_ PFN_WDF_REQUEST_COMPLETION_ROUTINE	CompletionRoutine
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;

	if(OptRequest == NULL)
	{
		Status = CreateRequest(DeviceContext->Device, DeviceContext->IoTarget, &Request);
		if(!NT_SUCCESS(Status))
		{
			return Status;
		}
	}
	else
	{
		Request = OptRequest;
	}

	Status = PrepareRequest(DeviceContext->IoTarget, BRB, Request);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return Status;
	}

	WdfRequestSetCompletionRoutine(
		Request,
		CompletionRoutine,
		BRB
		);

	if(!WdfRequestSend(
		Request,
		DeviceContext->IoTarget,
		WDF_NO_SEND_OPTIONS
		))
	{
        Status = WdfRequestGetStatus(Request);
		WdfObjectDelete(Request);
		return Status;
	}

	return Status;
}

NTSTATUS
SendBRBSynchronous(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_opt_ WDFREQUEST OptRequest,
	_In_ PBRB BRB
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_REQUEST_SEND_OPTIONS SendOptions;
	WDFREQUEST Request;

	if(OptRequest == NULL)
	{
		Status = CreateRequest(DeviceContext->Device, DeviceContext->IoTarget, &Request);
		if(!NT_SUCCESS(Status))
		{
			return Status;
		}
	}
	else
	{
		Request = OptRequest;
	}

	Status = PrepareRequest(DeviceContext->IoTarget, BRB, Request);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return Status;
	}

	Status = WdfRequestAllocateTimer(Request);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return Status;
	}

	WDF_REQUEST_SEND_OPTIONS_INIT(&SendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS | WDF_REQUEST_SEND_OPTION_TIMEOUT);
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&SendOptions, SYNCHRONOUS_CALL_TIMEOUT);

	WdfRequestSend(
		Request,
		DeviceContext->IoTarget,
		&SendOptions
		);
	
	Status = WdfRequestGetStatus(Request);

	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return Status;
	}

	return Status;
}

VOID
CleanUpCompletedRequest(
	_In_ WDFREQUEST Request,
	_In_  WDFIOTARGET IoTarget,
	_In_  WDFCONTEXT Context
	)
{
	PDEVICE_CONTEXT DeviceContext;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	PBRB UsedBRB;

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->BluetoothContext);
	UsedBRB = (PBRB)Context;

	WdfObjectDelete(Request);
	BluetoothContext->ProfileDriverInterface.BthFreeBrb(UsedBRB);
}


VOID 
L2CAPCallback(
	_In_  PVOID Context, 
	_In_  INDICATION_CODE Indication, 
	_In_  PINDICATION_PARAMETERS Parameters
	)
{
	//WDF_DEVICE_STATE NewDeviceState;
	PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)Context;

	//UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Parameters);
	
	Trace("L2CAP Channel Callback");
	Trace("Indication: %u", Indication);
	

	if(Indication == IndicationRemoteDisconnect)
	{
		//Wiimote has disconnected.
		//Code has to be added to signal the PnP-Manager that the device is gone.
		
		Trace("Disconnect");
		Trace("Parameter: %u; %u", Parameters->Parameters.Disconnect.Reason, Parameters->Parameters.Disconnect.CloseNow);
	
		ResetToNullState(DeviceContext);
		SignalDeviceIsGone(DeviceContext);

		//WDF_DEVICE_STATE_INIT (&NewDeviceState);

		//HidNotifyPresence(WdfDeviceWdmGetDeviceObject(DeviceContext->Device), FALSE);
		
		//WdfDeviceGetDeviceState(DeviceContext->Device, &NewDeviceState);
		//NewDeviceState.Removed = WdfTrue;
		//WdfDeviceSetDeviceState(DeviceContext->Device, &NewDeviceState);
		//WdfPdoMarkMissing(DeviceContext->Device);
	}
}

NTSTATUS 
OpenChannel(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_opt_ PBRB PreAllocatedBRB,
	_In_ BYTE PSM,
	_In_opt_ PFNBTHPORT_INDICATION_CALLBACK ChannelCallback,
	_In_ PFN_WDF_REQUEST_COMPLETION_ROUTINE ChannelCompletion
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);
	PBRB_L2CA_OPEN_CHANNEL BRBOpenChannel;

	//Create or reuse BRB
	if(PreAllocatedBRB == NULL)
	{
		BRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)BluetoothContext->ProfileDriverInterface.BthAllocateBrb(BRB_L2CA_OPEN_CHANNEL, BLUETOOTH_POOL_TAG);
		if (BRBOpenChannel == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}
	}
	else
	{
		BluetoothContext->ProfileDriverInterface.BthReuseBrb(PreAllocatedBRB, BRB_L2CA_OPEN_CHANNEL);
		BRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)PreAllocatedBRB;
	}
	
	//Fill BRB
	BRBOpenChannel->BtAddress = BluetoothContext->DeviceAddress;
	BRBOpenChannel->Psm = PSM; //0x13
	BRBOpenChannel->ChannelFlags = 0;
	BRBOpenChannel->ConfigOut.Flags = 0;
    BRBOpenChannel->ConfigOut.Mtu.Max = L2CAP_DEFAULT_MTU;
    BRBOpenChannel->ConfigOut.Mtu.Min = L2CAP_MIN_MTU;
    BRBOpenChannel->ConfigOut.Mtu.Preferred = L2CAP_DEFAULT_MTU;
	BRBOpenChannel->ConfigOut.FlushTO.Max = L2CAP_DEFAULT_FLUSHTO;
	BRBOpenChannel->ConfigOut.FlushTO.Min = L2CAP_MIN_FLUSHTO;
	BRBOpenChannel->ConfigOut.FlushTO.Preferred = L2CAP_DEFAULT_FLUSHTO;
	BRBOpenChannel->ConfigOut.ExtraOptions = 0;
	BRBOpenChannel->ConfigOut.NumExtraOptions = 0;
	BRBOpenChannel->ConfigOut.LinkTO = 0;

    BRBOpenChannel->IncomingQueueDepth = 50;
    BRBOpenChannel->ReferenceObject = (PVOID) WdfDeviceWdmGetDeviceObject(DeviceContext->Device);
   
	if(ChannelCallback != NULL)
	{
		BRBOpenChannel->CallbackFlags = CALLBACK_DISCONNECT;                                                   
		BRBOpenChannel->Callback = ChannelCallback; //L2CAPCallback;
		BRBOpenChannel->CallbackContext = (PVOID)DeviceContext;
	}

	//SendBRB
	Status = SendBRB(DeviceContext, NULL, (PBRB)BRBOpenChannel, ChannelCompletion);
	if(!NT_SUCCESS(Status))
	{
		BluetoothContext->ProfileDriverInterface.BthFreeBrb((PBRB)BRBOpenChannel);
		return Status;
	}

	return Status;
}

VOID 
ControlChannelCompletion(
	_In_  WDFREQUEST Request,
	_In_  WDFIOTARGET IoTarget,
	_In_  PWDF_REQUEST_COMPLETION_PARAMS Params,
	_In_  WDFCONTEXT Context
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	PBRB_L2CA_OPEN_CHANNEL UsedBRBOpenChannel;

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->BluetoothContext);
	UsedBRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)Context;

	Status = Params->IoStatus.Status;
	
	Trace("Control Channel Result: 0x%x", Status);

	if(!NT_SUCCESS(Status))
	{
		CleanUpCompletedRequest(Request, IoTarget, Context);
		if(Status == STATUS_IO_TIMEOUT)
		{
			SignalDeviceIsGone(DeviceContext);
		}
		else 
		{
			WdfDeviceSetFailed(DeviceContext->Device, WdfDeviceFailedNoRestart);
		}

		return;
	}

	BluetoothContext->ControlChannelHandle = UsedBRBOpenChannel->ChannelHandle;
	CleanUpCompletedRequest(Request, IoTarget, Context);
	
	// Open Interrupt Channel
	OpenChannel(DeviceContext, NULL, 0x13, L2CAPCallback, InterruptChannelCompletion);
}

VOID 
InterruptChannelCompletion(
	_In_  WDFREQUEST Request,
	_In_  WDFIOTARGET IoTarget,
	_In_  PWDF_REQUEST_COMPLETION_PARAMS Params,
	_In_  WDFCONTEXT Context
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	PBRB_L2CA_OPEN_CHANNEL UsedBRBOpenChannel;

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->BluetoothContext);
	UsedBRBOpenChannel = (PBRB_L2CA_OPEN_CHANNEL)Context;

	Status = Params->IoStatus.Status;
	
	Trace("Interrupt Channel Result: 0x%x", Status);

	if(!NT_SUCCESS(Status))
	{
		CleanUpCompletedRequest(Request, IoTarget, Context);
		if(Status == STATUS_IO_TIMEOUT)
		{
			SignalDeviceIsGone(DeviceContext);
		}
		else 
		{
			WdfDeviceSetFailed(DeviceContext->Device, WdfDeviceFailedNoRestart);
		}

		return;
	}

	BluetoothContext->InterruptChannelHandle = UsedBRBOpenChannel->ChannelHandle;
	CleanUpCompletedRequest(Request, IoTarget, Context);
	
	// Start Wiimote functionality
	StartWiimote(DeviceContext);
}

NTSTATUS
OpenChannels(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	return OpenChannel(DeviceContext, NULL, 0x11, NULL, ControlChannelCompletion);
}

NTSTATUS 
CloseChannel(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ L2CAP_CHANNEL_HANDLE ChannelHandle
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);
	PBRB_L2CA_CLOSE_CHANNEL BRBCloseChannel;

	if(ChannelHandle == NULL)
	{	
		Trace("Close Channel: Handle is NULL");

		return Status;
	}

	BRBCloseChannel = (PBRB_L2CA_CLOSE_CHANNEL)BluetoothContext->ProfileDriverInterface.BthAllocateBrb(BRB_L2CA_CLOSE_CHANNEL, BLUETOOTH_POOL_TAG);
	if (BRBCloseChannel == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	BRBCloseChannel->BtAddress = BluetoothContext->DeviceAddress;
	BRBCloseChannel->ChannelHandle = ChannelHandle;

	Status = SendBRBSynchronous(DeviceContext, NULL, (PBRB)BRBCloseChannel);
	BluetoothContext->ProfileDriverInterface.BthFreeBrb((PBRB)BRBCloseChannel);

	return Status;
}

NTSTATUS
CloseChannels(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{	
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);

	CloseChannel(DeviceContext, BluetoothContext->InterruptChannelHandle);
	BluetoothContext->InterruptChannelHandle = NULL;

	CloseChannel(DeviceContext, BluetoothContext->ControlChannelHandle);
	BluetoothContext->ControlChannelHandle = NULL;

	return STATUS_SUCCESS;
}

NTSTATUS 
TransferToDevice(
	_In_ PDEVICE_CONTEXT DeviceContext, 
	_In_ WDFREQUEST Request, 
	_In_ WDFMEMORY Memory,
	_In_ BOOLEAN Synchronous
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);
	PBRB_L2CA_ACL_TRANSFER BRBTransfer;
	size_t BufferSize;
	
	if(BluetoothContext->InterruptChannelHandle == NULL)
	{
		return STATUS_INVALID_HANDLE;
	}

	// Now get an BRB and fill it
	BRBTransfer = (PBRB_L2CA_ACL_TRANSFER)BluetoothContext->ProfileDriverInterface.BthAllocateBrb(BRB_L2CA_ACL_TRANSFER, BLUETOOTH_POOL_TAG);
	if (BRBTransfer == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	BRBTransfer->BtAddress = BluetoothContext->DeviceAddress;
	BRBTransfer->ChannelHandle = BluetoothContext->InterruptChannelHandle;
	BRBTransfer->TransferFlags = ACL_TRANSFER_DIRECTION_OUT;
	BRBTransfer->BufferMDL = NULL;
	BRBTransfer->Buffer = WdfMemoryGetBuffer(Memory, &BufferSize);
	BRBTransfer->BufferSize = (ULONG)BufferSize;

	//Send
	if(Synchronous)
	{
		Status = SendBRBSynchronous(DeviceContext, Request, (PBRB)BRBTransfer);
		BluetoothContext->ProfileDriverInterface.BthFreeBrb((PBRB)BRBTransfer);
		if(!NT_SUCCESS(Status))
		{
			return Status;
		}
	}
	else
	{
		Status = SendBRB(DeviceContext, Request, (PBRB)BRBTransfer, TransferToDeviceCompletion);	
		if(!NT_SUCCESS(Status))
		{
			BluetoothContext->ProfileDriverInterface.BthFreeBrb((PBRB)BRBTransfer);
			return Status;
		}
	}

	return Status;
}

VOID 
TransferToDeviceCompletion(
	_In_  WDFREQUEST Request,
	_In_  WDFIOTARGET IoTarget,
	_In_  PWDF_REQUEST_COMPLETION_PARAMS Params,
	_In_  WDFCONTEXT Context
	)
{
	UNREFERENCED_PARAMETER(Params);

	CleanUpCompletedRequest(Request, IoTarget, Context);
}

NTSTATUS
ReadFromDevice(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ WDFREQUEST Request,
	_In_ PBRB_L2CA_ACL_TRANSFER BRB,
	_In_ PVOID ReadBuffer,
	_In_ size_t ReadBufferSize
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);

	if(BluetoothContext->InterruptChannelHandle == NULL)
	{
		return STATUS_INVALID_HANDLE;
	}

	BRB->BtAddress = BluetoothContext->DeviceAddress;
	BRB->ChannelHandle = BluetoothContext->InterruptChannelHandle;
	BRB->TransferFlags = ACL_TRANSFER_DIRECTION_IN | ACL_SHORT_TRANSFER_OK;
	BRB->BufferMDL = NULL;
	BRB->Buffer = ReadBuffer;
	BRB->BufferSize = (ULONG)ReadBufferSize;

	Status = SendBRB(DeviceContext, Request, (PBRB)BRB, ReadFromDeviceCompletion);
	if(!NT_SUCCESS(Status))
	{
		Trace("SendBRB Failed 0x%x", Status);
		return Status;
	}

	return Status;
}

VOID
ReadFromDeviceCompletion(
	_In_  WDFREQUEST Request,
	_In_  WDFIOTARGET IoTarget,
	_In_  PWDF_REQUEST_COMPLETION_PARAMS Params,
	_In_  WDFCONTEXT Context
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_CONTEXT DeviceContext;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext;
	PVOID ReadBuffer;
	size_t ReadBufferSize;
	PBRB_L2CA_ACL_TRANSFER BRB;
	WDF_REQUEST_REUSE_PARAMS  RequestReuseParams;

	DeviceContext = GetDeviceContext(WdfIoTargetGetDevice(IoTarget));
	BluetoothContext = &(DeviceContext->BluetoothContext);
	BRB = (PBRB_L2CA_ACL_TRANSFER)Context;

	Status = Params->IoStatus.Status;

	//Trace("ReadFromDeviceCompletion Result: %#02X", Status);

	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return;
	}

	ReadBuffer = BRB->Buffer;
	ReadBufferSize = BRB->BufferSize;

	//Trace("RawBuffer: %08x", (*(UINT64 * )ReadBuffer));
	//Trace("BufferSize: %d - RemainingBufferSize: %d", BRB->BufferSize, BRB->RemainingBufferSize);

	//Call Wiimote Read Callback
	Status = ProcessReport(DeviceContext, ReadBuffer, (ReadBufferSize - BRB->RemainingBufferSize));
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return;
	}

	//Reset all Object for reuse
	BluetoothContext->ProfileDriverInterface.BthReuseBrb((PBRB)BRB, BRB_L2CA_ACL_TRANSFER);

	WDF_REQUEST_REUSE_PARAMS_INIT(&RequestReuseParams, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
	Status = WdfRequestReuse(Request, &RequestReuseParams);
	if(!NT_SUCCESS(Status))
	{
		WdfObjectDelete(Request);
		return;
	}

	RtlSecureZeroMemory(ReadBuffer, ReadBufferSize);

	//Send out new Read
	Status = ReadFromDevice(DeviceContext, Request, BRB, ReadBuffer, BluetoothContext->ReadBufferSize);
	if(!NT_SUCCESS(Status))
	{
		return;
	}
}

NTSTATUS
StartContiniousReader(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	CONST size_t ReadBufferSize = 50;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	PBRB BRB;
	PBLUETOOTH_DEVICE_CONTEXT BluetoothContext = &(DeviceContext->BluetoothContext);
	PVOID ReadBuffer = NULL;

	
	Trace("StartContiniousReader");

	//Create Report And Buffer
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, ReadBufferSize, &Request, &Memory, &ReadBuffer);
	if(!NT_SUCCESS(Status))
	{
		Trace("CreateRequestAndBuffer Failed: 0x%x", Status);
		return Status;
	}

	// Safe the Buffer Size
	BluetoothContext->ReadBufferSize = ReadBufferSize;

	// Create BRB
	BRB = BluetoothContext->ProfileDriverInterface.BthAllocateBrb(BRB_L2CA_ACL_TRANSFER, BLUETOOTH_POOL_TAG);
	if (BRB)
	{
		WdfObjectDelete(Request);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//Start the Reader
	Status = ReadFromDevice(DeviceContext, Request, (PBRB_L2CA_ACL_TRANSFER)BRB, ReadBuffer, ReadBufferSize);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	return Status;

}

/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Bluetooth.h

Abstract:
	Header file for Bluetooth.c
*/

#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include "HIDWiimote.h"

#include <bthdef.h>
#include <bthguid.h>
#include <bthioctl.h>
#include <sdpnode.h>
#include <bthddi.h>
#include <bthsdpddi.h>
#include <bthsdpdef.h>

#define BLUETOOTH_POOL_TAG '_htB'
#define BUFFER_POOL_TAG '_fuB'
#define SYNCHRONOUS_CALL_TIMEOUT (-1000000000) // 1 Second

typedef struct _BLUETOOTH_DEVICE_CONTEXT 
{
    BTH_PROFILE_DRIVER_INTERFACE ProfileDriverInterface;

    BTH_ADDR DeviceAddress;
	L2CAP_CHANNEL_HANDLE ControlChannelHandle;
	L2CAP_CHANNEL_HANDLE InterruptChannelHandle;

	size_t ReadBufferSize;

} BLUETOOTH_DEVICE_CONTEXT, * PBLUETOOTH_DEVICE_CONTEXT;

typedef struct _BRB_L2CA_OPEN_CHANNEL * PBRB_L2CA_OPEN_CHANNEL;
typedef struct _BRB_L2CA_CLOSE_CHANNEL  * PBRB_L2CA_CLOSE_CHANNEL;
typedef struct _BRB_L2CA_ACL_TRANSFER  * PBRB_L2CA_ACL_TRANSFER;

NTSTATUS GetVendorAndProductID(_In_ WDFIOTARGET IoTarget, _Out_ USHORT * VendorID, _Out_ USHORT * ProductID);

NTSTATUS PrepareBluetooth(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS OpenChannels(_In_ PDEVICE_CONTEXT DeviceContext);
EVT_WDF_REQUEST_COMPLETION_ROUTINE ControlChannelCompletion;
EVT_WDF_REQUEST_COMPLETION_ROUTINE InterruptChannelCompletion;
VOID L2CAPCallback(_In_  PVOID Context, _In_  INDICATION_CODE Indication, _In_  PINDICATION_PARAMETERS Parameters);

NTSTATUS CloseChannels(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS CreateRequestAndBuffer(_In_ WDFDEVICE Device, _In_ WDFIOTARGET IoTarget, _In_ size_t BufferSize, _Out_ WDFREQUEST * Request, _Out_ WDFMEMORY * Memory, _Out_opt_ PVOID * Buffer);
NTSTATUS TransferToDevice(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WDFREQUEST Request, _In_ WDFMEMORY Memory, _In_ BOOLEAN Synchronous);
EVT_WDF_REQUEST_COMPLETION_ROUTINE TransferToDeviceCompletion;

NTSTATUS StartContiniousReader(_In_ PDEVICE_CONTEXT DeviceContext);
EVT_WDF_REQUEST_COMPLETION_ROUTINE ReadFromDeviceCompletion;
#endif
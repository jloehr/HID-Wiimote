/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	Bluetooth.h

Abstract:
	Header file for Bluetooth.c
*/

#pragma once

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

#define BLUETOOTH_ADDRESS_STRING_SIZE 17

typedef struct _BLUETOOTH_DEVICE_CONTEXT 
{
    BTH_PROFILE_DRIVER_INTERFACE ProfileDriverInterface;

    BTH_ADDR DeviceAddress;
	UNICODE_STRING DeviceAddressString;
	WCHAR DeviceAddressStringBuffer[BLUETOOTH_ADDRESS_STRING_SIZE];

	L2CAP_CHANNEL_HANDLE ControlChannelHandle;
	L2CAP_CHANNEL_HANDLE InterruptChannelHandle;

	size_t ReadBufferSize;

} BLUETOOTH_DEVICE_CONTEXT, * PBLUETOOTH_DEVICE_CONTEXT;

typedef struct _BRB_L2CA_OPEN_CHANNEL * PBRB_L2CA_OPEN_CHANNEL;
typedef struct _BRB_L2CA_CLOSE_CHANNEL * PBRB_L2CA_CLOSE_CHANNEL;
typedef struct _BRB_L2CA_ACL_TRANSFER * PBRB_L2CA_ACL_TRANSFER;

NTSTATUS GetVendorAndProductID(_In_ WDFIOTARGET IoTarget, _Out_ USHORT * VendorID, _Out_ USHORT * ProductID);

NTSTATUS BluetoothPrepare(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS BluetoothOpenChannels(_In_ PDEVICE_CONTEXT DeviceContext);
NTSTATUS BluetoothCloseChannels(_In_ PDEVICE_CONTEXT DeviceContext);

NTSTATUS BluetoothCreateRequestAndBuffer(_In_ WDFDEVICE Device, _In_ WDFIOTARGET IoTarget, _In_ SIZE_T BufferSize, _Outptr_ WDFREQUEST * Request, _Outptr_ WDFMEMORY * Memory, _Outptr_opt_result_buffer_(BufferSize) PVOID * Buffer);
NTSTATUS BluetoothTransferToDevice(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WDFREQUEST Request, _In_ WDFMEMORY Memory, _In_ BOOLEAN Synchronous);

NTSTATUS BluetoothStartContiniousReader(_In_ PDEVICE_CONTEXT DeviceContext);

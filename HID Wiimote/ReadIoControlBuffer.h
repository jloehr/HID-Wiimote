/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	ReadIoControlBuffer.h

Abstract:
	Header file for ReadIoControlBuffer.c

*/
#pragma once

#include "HIDWiimote.h"

typedef VOID EVT_READ_IO_CONTROL_BUFFER_FILL_BUFFER(_In_ PDEVICE_CONTEXT DeviceContext, _Inout_updates_all_(BufferSize) PVOID Buffer, _In_ size_t BufferSize, _Out_ PSIZE_T BytesWritten);
typedef EVT_READ_IO_CONTROL_BUFFER_FILL_BUFFER *PFN_READ_IO_CONTROL_BUFFER_FILL_BUFFER;

typedef struct _READ_IO_CONTROL_BUFFER
{
	WDFQUEUE BufferQueue;
	PDEVICE_CONTEXT DeviceContext;
	PFN_READ_IO_CONTROL_BUFFER_FILL_BUFFER Callback;

	size_t MinOutputBufferSize;
	BOOLEAN InstantCompletion;

} READ_IO_CONTROL_BUFFER, * PREAD_IO_CONTROL_BUFFER;

NTSTATUS
ReadIoControlBufferCreate(
	_Inout_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer,
	_In_ WDFDEVICE ParentDevice,
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ PFN_READ_IO_CONTROL_BUFFER_FILL_BUFFER Callback,
	_In_ size_t MinOutputBufferSize
	);

VOID
ReadIoControlBufferFlush(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer
	);

VOID
ReadIoControlBufferForwardRequest(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer,
	_In_ WDFREQUEST Request);

VOID
ReadIoControlBufferDispatchRequest(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer
	);
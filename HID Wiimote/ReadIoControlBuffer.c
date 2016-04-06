/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	ReadIoControlBuffer.c

Abstract:
	A manual dispatch Queue to buffer read requests
*/
#include "ReadIoControlBuffer.h"


VOID
ReadIoControlBufferCompleteRequest(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer,
	_In_ WDFREQUEST Request
	);

NTSTATUS
ReadIoControlBufferCreate(
	_Inout_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer,
	_In_ WDFDEVICE ParentDevice,
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ PFN_READ_IO_CONTROL_BUFFER_FILL_BUFFER Callback,
	_In_ size_t MinOutputBufferSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDF_IO_QUEUE_CONFIG QueueConfig;

	// Set up the Object
	ReadIoControlBuffer->DeviceContext = DeviceContext;
	ReadIoControlBuffer->Callback = Callback;
	ReadIoControlBuffer->InstantCompletion = FALSE;
	ReadIoControlBuffer->MinOutputBufferSize = MinOutputBufferSize;

	// Create the WDF Queue
	WDF_IO_QUEUE_CONFIG_INIT(&QueueConfig, WdfIoQueueDispatchManual);
	QueueConfig.PowerManaged = WdfFalse;

	Status = WdfIoQueueCreate(ParentDevice, &QueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &(ReadIoControlBuffer->BufferQueue));
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Failed to create Read IO Control Buffer Queue", Status);
		return Status;
	}

	return Status;
}

VOID
ReadIoControlBufferFlush(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST TmpRequest;

	if (ReadIoControlBuffer->BufferQueue == NULL)
	{
		return;
	}

	while (NT_SUCCESS(Status = (WdfIoQueueRetrieveNextRequest(ReadIoControlBuffer->BufferQueue, &TmpRequest))))
	{
		WdfRequestComplete(TmpRequest, STATUS_DEVICE_REMOVED);
	}

	if (Status != STATUS_NO_MORE_ENTRIES)
	{
		TraceStatus("Something went wring while flushing a Read IO Control Buffer", Status);
	}
}

VOID
ReadIoControlBufferForwardRequest(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer,
	_In_ WDFREQUEST Request
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	if (ReadIoControlBuffer->InstantCompletion)
	{
		ReadIoControlBufferCompleteRequest(ReadIoControlBuffer, Request);
	}
	else
	{
		Status = WdfRequestForwardToIoQueue(Request, ReadIoControlBuffer->BufferQueue);
		if (!NT_SUCCESS(Status))
		{
			WdfRequestComplete(Request, Status);
			return;
		}
	}
}

VOID
ReadIoControlBufferCompleteRequest(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer,
	_In_ WDFREQUEST Request
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PVOID OutputBuffer;
	size_t OutputBufferSize;
	SIZE_T BytesWritten = 0;

	ReadIoControlBuffer->InstantCompletion = FALSE;

	Status = WdfRequestRetrieveOutputBuffer(Request, ReadIoControlBuffer->MinOutputBufferSize, &OutputBuffer, &OutputBufferSize);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error retrieving Output Buffer from Read IO Control", Status);
		WdfRequestComplete(Request, Status);
		return;
	}

	ReadIoControlBuffer->Callback(ReadIoControlBuffer->DeviceContext, OutputBuffer, OutputBufferSize, &BytesWritten);

	WdfRequestCompleteWithInformation(Request, Status, BytesWritten);
}

VOID
ReadIoControlBufferDispatchRequest(
	_In_ PREAD_IO_CONTROL_BUFFER ReadIoControlBuffer
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;

	Status = WdfIoQueueRetrieveNextRequest(ReadIoControlBuffer->BufferQueue, &Request);
	if (NT_SUCCESS(Status))
	{
		ReadIoControlBufferCompleteRequest(ReadIoControlBuffer, Request);
	}
	else if (Status == STATUS_NO_MORE_ENTRIES)
	{
		ReadIoControlBuffer->InstantCompletion = TRUE;
	}
}
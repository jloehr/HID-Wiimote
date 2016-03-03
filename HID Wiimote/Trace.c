/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Trace.c

Abstract:
	Contains the code for Tracing.

*/

#include "Trace.h"


VOID
Trace(
    _In_ PCCHAR  DebugMessage,
    ...
    )
{
#ifndef DBG
    UNREFERENCED_PARAMETER(DebugMessage);
#else
	NTSTATUS Status;
	va_list ParameterList;
	CHAR DebugMessageBuffer[512];

	va_start(ParameterList, DebugMessage);

	if(DebugMessage != NULL)
	{
		Status = RtlStringCbVPrintfA(DebugMessageBuffer, sizeof(DebugMessageBuffer), DebugMessage, ParameterList);

		if(NT_SUCCESS(Status))
		{
			DbgPrint("Trace Wiimote: %s\n", DebugMessageBuffer);
		}
	}

	va_end(ParameterList);

#endif
}


VOID 
PrintBytes(
	_In_reads_(Size) PCUCHAR Data,
	_In_ SIZE_T Size 
	)
{
#ifndef DBG
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(Size);
#else
	NTSTATUS Result = STATUS_SUCCESS;
	PCHAR Message;
	PCHAR WritePointer;
	PCUCHAR ReadPointer;
	const size_t BytesPerElement = 5;
	size_t StringBufferSize = (Size * BytesPerElement) + 1;

	Message = (PCHAR)ExAllocatePoolWithTag(NonPagedPool, StringBufferSize, PRINTBYTE_POOL_TAG);
	if (Message == NULL)
	{
		Trace("Error printing Bytes: Out of Memory");
		return;
	}

	WritePointer = Message;
	ReadPointer = Data;

	for (SIZE_T i = 0; (i < Size) && (NT_SUCCESS(Result)); ++i)
	{
		Result = RtlStringCbPrintfA(WritePointer, StringBufferSize, "%#04x ", *ReadPointer);

		WritePointer += BytesPerElement;
		StringBufferSize -= BytesPerElement;
		ReadPointer++;
	}

	(*WritePointer) = 0;
	Trace("Byte Print: %s", Message);
	ExFreePool(Message);

#endif
}

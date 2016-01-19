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
	_In_reads_(Size) PCCHAR Data,
	_In_ SIZE_T Size 
	)
{
#ifndef DBG
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(Size);
#else

	CHAR * Message;
	CHAR * WritePointer;
	CCHAR * ReadPointer;
	size_t i;

	Message = (CHAR *)ExAllocatePoolWithTag(NonPagedPool, (10 * Size), PRINTBYTE_POOL_TAG);
	WritePointer = Message;
	ReadPointer = Data;

	for (i = 0; i < Size; ++i)
	{
		WritePointer += sprintf(WritePointer, "%#02x ", *ReadPointer);
		ReadPointer++;
	}

	(*WritePointer) = 0;
	Trace("Byte Print: %s", Message);
	ExFreePool(Message);

#endif
}

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

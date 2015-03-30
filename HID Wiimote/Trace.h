/*

Copyright (C) 2013 Julian Löhr
All rights reserved.

Filename:
	Trace.h

Abstract:
	Header file for Trace.c
	Contains declarations and configuration for Tracing.

*/
#ifndef _TRACE_H_
#define _TRACE_H_

#include <ntddk.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

VOID
Trace(
    _In_ PCCHAR  DebugMessage,
    ...
    );

#endif
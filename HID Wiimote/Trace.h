/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	Trace.h

Abstract:
	Header file for Trace.c
	Contains declarations and configuration for Tracing.

*/
#pragma once

#include <ntddk.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

#define PRINTBYTE_POOL_TAG '_BrP'
#define NTSTATUS_FORMAT_IDENTIFIER "%#010x"

VOID
Trace(
    _In_ PCCHAR  DebugMessage,
    ...
    );

VOID
TraceStatus(
	_In_ PCCHAR  DebugMessage,
	_In_ NTSTATUS Status
	);

VOID
PrintBytes(
	_In_reads_(Size) PCUCHAR Data,
	_In_ SIZE_T Size
	);

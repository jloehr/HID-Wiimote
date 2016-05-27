/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteToHIDParser.h

Abstract:
	Header file for WiimoteToHIDParser.c
*/
#pragma once

#include "HIDWiimote.h"
#include "HIDDescriptors.h"
#include "WiimoteState.h"

VOID ParseWiimoteStateAsGamepad(_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, _Inout_updates_all_(BufferSize) PVOID Buffer, _In_ size_t BufferSize, _Out_ PSIZE_T BytesWritten);
VOID ParseWiimoteStateAsDPadMouse(_In_ PWIIMOTE_STATE WiimoteState, _Inout_updates_all_(BufferSize) PVOID Buffer, _In_ size_t BufferSize, _Out_ PSIZE_T BytesWritten);
VOID ParseWiimoteStateAsIRMouse(_In_ PWIIMOTE_STATE WiimoteState, _Inout_updates_all_(BufferSize) PVOID Buffer, _In_ size_t BufferSize, _Out_ PSIZE_T BytesWritten);

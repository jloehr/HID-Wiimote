/*

Copyright (C) 2014 Julian Löhr
All rights reserved.

Filename:
	WiimoteToHIDParser.c

Abstract:
	Parses the current Wiimote State to a HID Read Report

*/

#include "WiimoteToHIDParser.h"
#include "Wiimote.h"

VOID
ParseBooleanAxis(
	_In_ BOOLEAN MinimumValue,
	_In_ BOOLEAN MaximumValue,
	_Out_ BYTE * ReportByte,
	_In_ UCHAR LeastSignificantBitPosition,
	_In_ BYTE Bits
	)
{
	UNREFERENCED_PARAMETER(Bits);

	BYTE MinValue = 0; //(0x80 >> (8 - Bits)) + 0x01;
	BYTE MaxValue = 0xFF; //0x7F >> (8 - Bits);

	if(MinimumValue)
	{
		(*ReportByte) |= (MinValue << LeastSignificantBitPosition);
	}
	else if(MaximumValue)
	{
		(*ReportByte) |= (MaxValue << LeastSignificantBitPosition);
	}
}

VOID
ParseButton(
	_In_ BOOLEAN ButtonValue,
	_Out_ BYTE * ReportByte,
	_In_ UCHAR LeastSignificantBitPosition
	)
{
	if(ButtonValue)
	{
		(*ReportByte) |= (0x01 << LeastSignificantBitPosition);
	}
}

VOID
ParseAnalogAxis(
	_In_ BYTE RawValue,
	_Out_ BYTE ReportByte[1],
	_In_ BYTE UsedBits
)
{
	ReportByte[0] = RawValue << (8 - UsedBits);
}

VOID
ParseAccelerometer(
	_In_ BYTE RawValue,
	_Out_ BYTE ReportByte[2],
	_In_ UCHAR LeastSignificantBitPosition,
	_In_opt_ BOOLEAN Invert
)
{
	// 9,8 is gravity, shifted because two lsb are ignored, gives 24
	//Some varity, so Min = 0, Max = 63, middle 32

	if(RawValue < (0x80 - 0x20))
	{
		RawValue = 0x80 - 0x20;
	} 
	else if(RawValue > (0x80 + 0x1F))
	{
		RawValue = 0x80 + 0x1F;
	} 
	
	RawValue -= (0x80 - 0x20);
	if(Invert)
	{
		RawValue = 0x3F - RawValue;
	}
	RawValue &= 0x3F;

	ReportByte[0] |= (RawValue << LeastSignificantBitPosition);
	ReportByte[1] |= (RawValue >> (8 - LeastSignificantBitPosition));
}

VOID
ParseWiimoteStateAsStandaloneWiiremote(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ BYTE RequestBuffer[5]
	)
{
	//Axis
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Up, WiimoteContext->State.CoreButtons.DPad.Down, RequestBuffer, 0, 8);
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Right, WiimoteContext->State.CoreButtons.DPad.Left, RequestBuffer + 1, 0, 8);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.One, RequestBuffer + 2, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Two, RequestBuffer + 2, 1);
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer + 2, 2);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer + 2, 3);
	ParseButton(WiimoteContext->State.CoreButtons.Plus, RequestBuffer + 2, 4);
	ParseButton(WiimoteContext->State.CoreButtons.Minus, RequestBuffer + 2, 5);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer + 2, 6);
	
	//Accelerometer
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, RequestBuffer + 3, 0, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, RequestBuffer + 3, 6, TRUE);
	//ParseAccelerometer(WiimoteContext->State.Accelerometer.Z, RequestBuffer + 3, 4);
	
	//RtlCopyMemory(RequestBuffer + 2, WiimoteContext->State.AccelerometerRaw, 3);

}

VOID
ParseWiimoteStateAsNunchuckExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ BYTE RequestBuffer[5]
	)
{	
	//AnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.X, RequestBuffer, 8);
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.Y, RequestBuffer + 1, 8);

	//Accelerometer
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, RequestBuffer + 3, 0, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Z, RequestBuffer + 3, 6, TRUE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer + 2, 0);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer + 2, 1);
	ParseButton(WiimoteContext->NunchuckState.Buttons.C, RequestBuffer + 2, 2);
	ParseButton(WiimoteContext->NunchuckState.Buttons.Z, RequestBuffer + 2, 3);
	ParseButton(WiimoteContext->State.CoreButtons.Plus, RequestBuffer + 2, 4);
	ParseButton(WiimoteContext->State.CoreButtons.Minus, RequestBuffer + 2, 5);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer + 2, 6);
}

VOID
ParseWiimoteState(
_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
_Out_ BYTE RequestBuffer[5]
)
{
	RtlSecureZeroMemory(RequestBuffer, 4);

	ParseWiimoteStateAsStandaloneWiiremote(WiimoteContext, RequestBuffer);

	switch (WiimoteContext->Extension)
	{
	case None:
		ParseWiimoteStateAsStandaloneWiiremote(WiimoteContext, RequestBuffer);
		break;
	case Nunchuck:
		ParseWiimoteStateAsNunchuckExtension(WiimoteContext, RequestBuffer);
		break;
	case ClassicController:
		break;

	default:
		break;
	}

}

VOID ParseWiimoteStateAsDPadMouse( 
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, 
	_Out_ BYTE RequestBuffer[4])
{
	RtlSecureZeroMemory(RequestBuffer, 4);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.One, RequestBuffer, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Two, RequestBuffer, 1);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer, 2);

	//Axis
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Up, WiimoteContext->State.CoreButtons.DPad.Down, RequestBuffer + 1, 6, 2);
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Right, WiimoteContext->State.CoreButtons.DPad.Left, RequestBuffer + 2, 6, 2);
}
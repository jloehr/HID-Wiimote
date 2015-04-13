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

	BYTE MinValue = (0x80 >> (8 - Bits)) + 0x01;
	BYTE MaxValue = 0x7F >> (8 - Bits);

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
	_In_opt_ BOOLEAN Signed,
	_In_opt_ BOOLEAN Invert
)
{
	if (Invert)
	{
		RawValue ^= 0xFF;
	}

	if (Signed)
	{
		CHAR Value = RawValue - 0x80;

		ReportByte[0] |= Value;
	}
	else
	{
		ReportByte[0] = RawValue;
	}

}

VOID
ParseAccelerometer(
	_In_ BYTE RawValue,
	_Out_ BYTE ReportByte[2],
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
	RawValue = RawValue << 2;

	ReportByte[0] = RawValue;
	//ReportByte[1] |= (RawValue >> (8 - LeastSignificantBitPosition));
}

VOID
ParseTrigger(
	_In_ BYTE LeftValue,
	_In_ BYTE RightValue,
	_Out_ BYTE ReportByte[1]
)
{
	ReportByte[0] = (0x80 - LeftValue + RightValue);
}

VOID
ParseWiimoteStateAsStandaloneWiiremote(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ BYTE RequestBuffer[7]
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
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, RequestBuffer + 4, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, RequestBuffer + 5, TRUE);
	//ParseAccelerometer(WiimoteContext->State.Accelerometer.Z, RequestBuffer + 3, 4);
	
	//RtlCopyMemory(RequestBuffer + 2, WiimoteContext->State.AccelerometerRaw, 3);

}

VOID
ParseWiimoteStateAsNunchuckExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ BYTE RequestBuffer[8]
	)
{
	//AnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.X, RequestBuffer, TRUE, FALSE);
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.Y, RequestBuffer + 1, TRUE, TRUE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer + 2, 0);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer + 2, 1);
	ParseButton(WiimoteContext->NunchuckState.Buttons.C, RequestBuffer + 2, 2);
	ParseButton(WiimoteContext->NunchuckState.Buttons.Z, RequestBuffer + 2, 3);
	ParseButton(WiimoteContext->State.CoreButtons.One, RequestBuffer + 2, 4);
	ParseButton(WiimoteContext->State.CoreButtons.Two, RequestBuffer + 2, 5);
	ParseButton(WiimoteContext->State.CoreButtons.Plus, RequestBuffer + 2, 6);
	ParseButton(WiimoteContext->State.CoreButtons.Minus, RequestBuffer + 2, 7);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer + 3, 0);

	//Accelerometer
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, RequestBuffer + 4, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, RequestBuffer + 5, TRUE);

	//DPad
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Up, RequestBuffer + 7, 0);
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Right, RequestBuffer + 7, 1);
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Down, RequestBuffer + 7, 2);
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Left, RequestBuffer + 7, 3);
}

VOID
ParseWiimoteStateAsClassicControllerExtension(
_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
_Out_ BYTE RequestBuffer[8]
)
{
	//LeftAnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.LeftAnalogStick.X, RequestBuffer, TRUE, FALSE);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.LeftAnalogStick.Y, RequestBuffer + 1, TRUE, TRUE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A || WiimoteContext->ClassicControllerState.Buttons.A, RequestBuffer + 2, 0);
	ParseButton(WiimoteContext->State.CoreButtons.B || WiimoteContext->ClassicControllerState.Buttons.B, RequestBuffer + 2, 1);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.Y, RequestBuffer + 2, 2);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.X, RequestBuffer + 2, 3);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.L, RequestBuffer + 2, 4);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.R, RequestBuffer + 2, 5);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.ZL, RequestBuffer + 2, 6);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.ZR, RequestBuffer + 2, 7);
	ParseButton(WiimoteContext->State.CoreButtons.Plus || WiimoteContext->ClassicControllerState.Buttons.Plus, RequestBuffer + 3, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Minus || WiimoteContext->ClassicControllerState.Buttons.Minus, RequestBuffer + 3, 1);
	ParseButton(WiimoteContext->State.CoreButtons.Home || WiimoteContext->ClassicControllerState.Buttons.Home, RequestBuffer + 3, 2);
	ParseButton(WiimoteContext->State.CoreButtons.One || WiimoteContext->ClassicControllerState.Buttons.LH, RequestBuffer + 3, 3);
	ParseButton(WiimoteContext->State.CoreButtons.Two || WiimoteContext->ClassicControllerState.Buttons.RH, RequestBuffer + 3, 4);

	//Right Analog Stick as Second Axis
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.RightAnalogStick.X, RequestBuffer + 4, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.RightAnalogStick.Y, RequestBuffer + 5, FALSE, TRUE);
	ParseTrigger(WiimoteContext->ClassicControllerState.LeftTrigger, WiimoteContext->ClassicControllerState.RightTrigger, RequestBuffer + 6);

	//DPad
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Up || WiimoteContext->ClassicControllerState.Buttons.DPad.Up, RequestBuffer + 7, 0);
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Right || WiimoteContext->ClassicControllerState.Buttons.DPad.Right, RequestBuffer + 7, 1);
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Down || WiimoteContext->ClassicControllerState.Buttons.DPad.Down, RequestBuffer + 7, 2);
	ParseButton(WiimoteContext->State.CoreButtons.DPad.Left || WiimoteContext->ClassicControllerState.Buttons.DPad.Left, RequestBuffer + 7, 3);
}

VOID
ParseWiimoteState(
_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
_Out_ BYTE RequestBuffer[8]
)
{
	UNREFERENCED_PARAMETER(WiimoteContext);
	UNREFERENCED_PARAMETER(RequestBuffer);

	RtlSecureZeroMemory(RequestBuffer, 8);

	switch (WiimoteContext->Extension)
	{
	case None:
		ParseWiimoteStateAsStandaloneWiiremote(WiimoteContext, RequestBuffer);
		break;
	case Nunchuck:
		ParseWiimoteStateAsNunchuckExtension(WiimoteContext, RequestBuffer);
		break;
	case ClassicController:
	case WiiUProController:
		ParseWiimoteStateAsClassicControllerExtension(WiimoteContext, RequestBuffer);
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
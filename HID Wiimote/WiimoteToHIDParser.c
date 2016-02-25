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
	_Inout_updates_(1) PUCHAR ReportByte,
	_In_ UCHAR LeastSignificantBitPosition,
	_In_ BYTE Bits,
	_In_ BOOLEAN Signed
	)
{
	BYTE MinValue = (Signed ? 0x81 : 0x00) >> (8 - Bits);
	BYTE MaxValue = (Signed ? 0x7F : 0xFF) >> (8 - Bits);
	BYTE NullValue = (Signed ? 0x00 : 0x7F) >> (8 - Bits);

	if(MinimumValue)
	{
		(*ReportByte) |= (MinValue << LeastSignificantBitPosition);
	}
	else if(MaximumValue)
	{
		(*ReportByte) |= (MaxValue << LeastSignificantBitPosition);
	}
	else
	{
		(*ReportByte) |= (NullValue << LeastSignificantBitPosition);
	}
}

VOID
ParseButton(
	_In_ BOOLEAN ButtonValue,
	_Inout_updates_(1) PUCHAR ReportByte,
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
	_Out_writes_all_(1) PUCHAR ReportByte,
	_In_ BOOLEAN Signed,
	_In_ BOOLEAN Invert
)
{
	if (Invert)
	{
		RawValue ^= 0xFF;
	}

	if (Signed)
	{
		CHAR Value = ((CHAR)RawValue) - 0x80;

		ReportByte[0] = (0x00 | Value);
	}
	else
	{
		ReportByte[0] = RawValue;
	}

}

VOID
ParseAccelerometer(
	_In_ BYTE RawValue,
	_Out_writes_all_(2) PUCHAR ReportByte,
	_In_ BOOLEAN Invert
)
{
	// Gravity is always present. 10 Bit Accelerometer data would have value of 96 (9,8 m/s²).
	// As only the 8 MSBs are used the value is 24. To caputure that constant pull a value range from 0 to 63 is used (32 = nullstate).
	// For a range of 63 6 bits are used.

	// Normally 0x80 (128) is the middle of the 8 bit range, so cap the accelerometer data to a range of 32 around 128.
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
}

VOID
ParseTrigger(
	_In_ BYTE TriggerValue,
	_Out_ BYTE ReportByte[1]
)
{
	ReportByte[0] = TriggerValue;
}

VOID
ParseDPad(
_In_ BOOLEAN Up,
_In_ BOOLEAN Right,
_In_ BOOLEAN Down,
_In_ BOOLEAN Left,
_Out_writes_all_(1) PUCHAR ReportByte
)
{
	/*
		Up			1		0000 0001			0000 0000			
		Up-Right	2		0000 0010			0000 0001
		Right		3		0000 0011			0000 0010
		Down-Right	4		0000 0100			0000 0011
		Down		5		0000 0101			0000 0100
		Down-Left	6		0000 0110			0000 0101
		Left		7		0000 0111			0000 0110
		Up-Left		8		0000 1000			0000 0111
	*/

	//Thanks to Waterlimon for this solution
	//See http://www.gamedev.net/topic/667868-direction-booleans-into-single-integer/ for more solutions.
	//
	// Branchless polynomial version (just for fun):
	//int64_t x = 3 * (up - down) + left - right;
	// return x * (x * (x * (x * (x * (x * (x * (x * (-31) - 28) + 938) + 840) - 8519) - 6972) + 24412) + 12880) / 3360;
	//
	const BYTE ValueLookUpTable[3][3] = {
			{ 8, 1, 2 },
			{ 7, 0, 3 },
			{ 6, 5, 4 },
		};

	//BOOLEAN is a UCHAR in C
	//so make sure its numerical value is 1, some may have other values due to bit operations.
	Up = Up ? 1 : 0;
	Down = Down ? 1 : 0;
	Left = Left ? 1 : 0;
	Right = Right ? 1 : 0;

	ReportByte[0] = ValueLookUpTable[Down + 1 - Up][Right + 1 - Left];
}

UCHAR
JoinSensorValue(
	_In_ USHORT ValueOne,
	_In_ USHORT ValueTwo
	)
{
	ValueOne = min(ValueOne, 0x7F);
	ValueTwo = min(ValueTwo, 0x7F);

	UCHAR Value = (UCHAR)(ValueOne + ValueTwo);
	return (Value / 2);
}

BYTE
ParseBalanceBoardSensors(
	_In_ USHORT PositiveValueOne,
	_In_ USHORT PositiveValueTwo,
	_In_ USHORT NegativeValueOne,
	_In_ USHORT NegativeValueTwo
	)
{
	UCHAR PositiveValue = JoinSensorValue(PositiveValueOne, PositiveValueTwo);
	UCHAR NegativeValue = JoinSensorValue(NegativeValueOne, NegativeValueTwo);

	BYTE Value = PositiveValue + 0x7F - NegativeValue;

	return Value;
}

BOOLEAN AccumulateIRPoint(
	_In_ PWIIMOTE_IR_POINT Point,
	_Inout_ PUINT16 X,
	_Inout_ PUINT16 Y
	)
{
	// X ranges from 0-1023; Y from 0-767; so if Y == 0x3FF (1023) the point data is empty
	if (Point->Y != 0x3FF)
	{
		(*X) += Point->X;
		(*Y) += Point->Y;
		return TRUE;
	}

	return FALSE;
}

BOOLEAN ParseIRPoints(
	_In_ WIIMOTE_IR_POINT Points[WIIMOTE_IR_POINTS],
	_Out_ PUINT32 X,
	_Out_ PUINT32 Y
	)
{
	BYTE ValidPointCount = 0;
	UINT16 GroupX = 0;
	UINT16 GroupY = 0;

	for (BYTE i = 0; i < WIIMOTE_IR_POINTS; i++)
	{
		if (AccumulateIRPoint(&Points[i], &GroupX, &GroupY))
		{
			ValidPointCount++;
		}
	}

	if (ValidPointCount > 0)
	{
		(*X) += GroupX / ValidPointCount;
		(*Y) += GroupY / ValidPointCount;

		return TRUE;
	}

	return FALSE;
}

VOID ParseIRCamera(
	_In_ WIIMOTE_IR_POINT Points[WIIMOTE_IR_POINTS_BUFFER_SIZE][WIIMOTE_IR_POINTS],
	_Out_writes_all_(4) PUCHAR ReportByte,
	_In_ BYTE XPadding,
	_In_ BYTE YPadding
	)
{
	UNREFERENCED_PARAMETER(ReportByte);

	BYTE ValidBufferCount = 0;
	UINT32 X = 0;
	UINT32 Y = 0;

	for (BYTE i = 0; i < WIIMOTE_IR_POINTS_BUFFER_SIZE; i++)
	{
		if (ParseIRPoints(Points[i], &X, &Y))
		{
			ValidBufferCount++;
		}
	}

	if (ValidBufferCount == 0)
	{
		return;
	}

	X /= ValidBufferCount;
	Y /= ValidBufferCount;

	//Invert X
	X = WIIMOTE_IR_POINT_X_MAX - X;

	X = (X >= (UINT32)(WIIMOTE_IR_POINT_X_MAX - XPadding)) ? WIIMOTE_IR_POINT_X_MAX - XPadding : X;
	X = (X <= XPadding) ? 0 : X - XPadding;

	Y = (Y >= (UINT32)(WIIMOTE_IR_POINT_Y_MAX - YPadding)) ? WIIMOTE_IR_POINT_Y_MAX - YPadding : Y;
	Y = (Y <= YPadding) ? 0 : Y - YPadding;

	// Value = OldValue * NewMax / OldMax
	// Bug or Feature in at least Windows 8.1 HID Class, LOGCIAL_MAXIMUM musst be power of 2 - 1
	X *= 1023;
	X /= WIIMOTE_IR_POINT_X_MAX - 2 * XPadding;

	Y *= 1023;
	Y /= WIIMOTE_IR_POINT_Y_MAX - 2 * YPadding;
	

	ReportByte[0] = (0xFF & X);
	ReportByte[1] = (0xFF & (X >> 8));

	ReportByte[2] = (0xFF & Y);
	ReportByte[3] = (0xFF & (Y >> 8));
}

VOID
ParseWiimoteStateAsStandaloneWiiremote(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_(7) PUCHAR RequestBuffer
	)
{
	//Axis
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Up, WiimoteContext->State.CoreButtons.DPad.Down, RequestBuffer, 0, 8, FALSE);
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Right, WiimoteContext->State.CoreButtons.DPad.Left, RequestBuffer + 1, 0, 8, FALSE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.One, RequestBuffer + 2, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Two, RequestBuffer + 2, 1);
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer + 2, 2);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer + 2, 3);
	ParseButton(WiimoteContext->State.CoreButtons.Plus, RequestBuffer + 2, 4);
	ParseButton(WiimoteContext->State.CoreButtons.Minus, RequestBuffer + 2, 5);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer + 2, 6);
	
	//Accelerometer
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, RequestBuffer + 5, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, RequestBuffer + 6, TRUE);
	//ParseAccelerometer(WiimoteContext->State.Accelerometer.Z, RequestBuffer + 3, 4);
	
	//RtlCopyMemory(RequestBuffer + 2, WiimoteContext->State.AccelerometerRaw, 3);

}

VOID
ParseWiimoteStateAsNunchuckExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_(9) PUCHAR RequestBuffer
	)
{
	//AnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.X, RequestBuffer, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.Y, RequestBuffer + 1, FALSE, TRUE);

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
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, RequestBuffer + 5, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, RequestBuffer + 6, TRUE);

	//DPad
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up, 
		WiimoteContext->State.CoreButtons.DPad.Right, 
		WiimoteContext->State.CoreButtons.DPad.Down, 
		WiimoteContext->State.CoreButtons.DPad.Left, 
		RequestBuffer + 8);
}

VOID
ParseWiimoteStateAsBalanceBoard(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_(3) PUCHAR RequestBuffer
	)
{
	RequestBuffer[0] = ParseBalanceBoardSensors(WiimoteContext->BalanceBoardState.Sensor.Data.TopLeft, WiimoteContext->BalanceBoardState.Sensor.Data.TopRight, WiimoteContext->BalanceBoardState.Sensor.Data.BottomLeft, WiimoteContext->BalanceBoardState.Sensor.Data.BottomRight);
	RequestBuffer[1] = ParseBalanceBoardSensors(WiimoteContext->BalanceBoardState.Sensor.Data.BottomRight, WiimoteContext->BalanceBoardState.Sensor.Data.TopRight, WiimoteContext->BalanceBoardState.Sensor.Data.TopLeft, WiimoteContext->BalanceBoardState.Sensor.Data.BottomLeft);

	// Balance Board has only a single button, that is reported as "A"
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer + 2, 0);
}

VOID
ParseWiimoteStateAsClassicControllerExtension(
_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
_Inout_updates_(9) PUCHAR RequestBuffer
)
{
	//LeftAnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.LeftAnalogStick.X, RequestBuffer, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.LeftAnalogStick.Y, RequestBuffer + 1, FALSE, TRUE);

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
	ParseTrigger(WiimoteContext->ClassicControllerState.LeftTrigger, RequestBuffer + 4);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.RightAnalogStick.X, RequestBuffer + 5, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.RightAnalogStick.Y, RequestBuffer + 6, FALSE, TRUE);
	ParseTrigger(WiimoteContext->ClassicControllerState.RightTrigger, RequestBuffer + 7);

	//DPad
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up || WiimoteContext->ClassicControllerState.Buttons.DPad.Up,
		WiimoteContext->State.CoreButtons.DPad.Right || WiimoteContext->ClassicControllerState.Buttons.DPad.Right,
		WiimoteContext->State.CoreButtons.DPad.Down || WiimoteContext->ClassicControllerState.Buttons.DPad.Down,
		WiimoteContext->State.CoreButtons.DPad.Left || WiimoteContext->ClassicControllerState.Buttons.DPad.Left,
		RequestBuffer + 8);
}

VOID
ParseWiimoteStateAsGuitarExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_(9) PUCHAR RequestBuffer
	)
{
	// LeftAnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->GuitarState.AnalogStick.X, RequestBuffer, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->GuitarState.AnalogStick.Y, RequestBuffer + 1, FALSE, TRUE);

	// Buttons
	ParseButton(WiimoteContext->GuitarState.Buttons.Green, RequestBuffer + 2, 0);
	ParseButton(WiimoteContext->GuitarState.Buttons.Red, RequestBuffer + 2, 1);
	ParseButton(WiimoteContext->GuitarState.Buttons.Yellow, RequestBuffer + 2, 2);
	ParseButton(WiimoteContext->GuitarState.Buttons.Blue, RequestBuffer + 2, 3);
	ParseButton(WiimoteContext->GuitarState.Buttons.Orange, RequestBuffer + 2, 4);
	ParseButton(WiimoteContext->GuitarState.Buttons.Up, RequestBuffer + 2, 5);
	ParseButton(WiimoteContext->GuitarState.Buttons.Down, RequestBuffer + 2, 6);
	ParseButton(WiimoteContext->State.CoreButtons.Plus || WiimoteContext->GuitarState.Buttons.Plus, RequestBuffer + 3, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Minus || WiimoteContext->GuitarState.Buttons.Minus, RequestBuffer + 3, 1);
	ParseButton(WiimoteContext->State.CoreButtons.One, RequestBuffer + 3, 2);
	ParseButton(WiimoteContext->State.CoreButtons.Two, RequestBuffer + 3, 3);
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer + 3, 4);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer + 3, 5);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer + 3, 6);

	// Analog Bars
	ParseAnalogAxis(WiimoteContext->GuitarState.WhammyBar, RequestBuffer + 5, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->GuitarState.TouchBar, RequestBuffer + 6, FALSE, TRUE);

	// DPad
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up,
		WiimoteContext->State.CoreButtons.DPad.Right,
		WiimoteContext->State.CoreButtons.DPad.Down,
		WiimoteContext->State.CoreButtons.DPad.Left,
		RequestBuffer + 8);
}


VOID
ParseWiimoteState(
_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
_Out_writes_all_(9) PUCHAR RequestBuffer
)
{
	RtlSecureZeroMemory(RequestBuffer, 9);

	switch (WiimoteContext->Extension)
	{
	case None:
		ParseWiimoteStateAsStandaloneWiiremote(WiimoteContext, RequestBuffer);
		break;
	case Nunchuck:
		ParseWiimoteStateAsNunchuckExtension(WiimoteContext, RequestBuffer);
		break;
	case BalanceBoard:
		ParseWiimoteStateAsBalanceBoard(WiimoteContext, RequestBuffer);
		break;
	case ClassicController:
	case WiiUProController:
		ParseWiimoteStateAsClassicControllerExtension(WiimoteContext, RequestBuffer);
		break;
	case Guitar:
		ParseWiimoteStateAsGuitarExtension(WiimoteContext, RequestBuffer);
	default:
		break;
	}
}

VOID ParseWiimoteStateAsDPadMouse( 
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, 
	_Out_writes_all_(4) PUCHAR RequestBuffer
	)
{
	RtlSecureZeroMemory(RequestBuffer, 4);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.One, RequestBuffer, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Two, RequestBuffer, 1);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer, 2);

	//Axis
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Up, WiimoteContext->State.CoreButtons.DPad.Down, RequestBuffer + 1, 6, 2, TRUE);
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Right, WiimoteContext->State.CoreButtons.DPad.Left, RequestBuffer + 2, 6, 2, TRUE);
}

VOID ParseWiimoteStateAsIRMouse(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_writes_all_(3) PUCHAR RequestBuffer
	)
{
	RtlSecureZeroMemory(RequestBuffer, 3);
	
	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer, 0);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer, 1);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer, 2);

	//Axis
	ParseIRCamera(WiimoteContext->IRState.Points, RequestBuffer + 1, 128, 121);
}

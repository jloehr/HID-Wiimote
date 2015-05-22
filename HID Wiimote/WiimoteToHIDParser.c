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
ParseDPad(
_In_ BOOLEAN Up,
_In_ BOOLEAN Right,
_In_ BOOLEAN Down,
_In_ BOOLEAN Left,
_Out_ BYTE ReportByte[1]
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

	if (Up && Down)
	{
		Up = FALSE;
		Down = FALSE;
	}

	if (Left && Right)
	{
		Left = FALSE;
		Right = FALSE;
	}

	if (Left)
	{
		ReportByte[0] = 0x07;
	}
	else if (Right)
	{
		ReportByte[0] = 0x03;
	}

	if (Up)
	{
		if (Right)
		{
			ReportByte[0] = 0x02;
		}
		else if (Left)
		{
			ReportByte[0] = 0x08;
		}
		else
		{
			ReportByte[0] = 0x01;
		}
	}
	else if (Down)
	{
		if (Right)
		{
			ReportByte[0] = 0x04;
		}
		else if (Left)
		{
			ReportByte[0] = 0x06;
		}
		else
		{
			ReportByte[0] = 0x05;
		}
	}
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
	_Out_ PUINT16 X,
	_Out_ PUINT16 Y
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
	_Out_ BYTE ReportByte[4],
	_In_ BYTE XPadding,
	_In_ BYTE YPadding
	)
{
	UNREFERENCED_PARAMETER(ReportByte);

	BYTE ValidBufferCount = 0;
	UINT16 X = 0;
	UINT16 Y = 0;

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

	X = (X > (WIIMOTE_IR_POINT_X_MAX - XPadding)) ? WIIMOTE_IR_POINT_X_MAX - XPadding : X;
	X = (X <= XPadding) ? 0 : X - XPadding;

	Y = (Y > (WIIMOTE_IR_POINT_Y_MAX - YPadding)) ? WIIMOTE_IR_POINT_Y_MAX - YPadding : X;
	Y = (Y <= YPadding) ? 0 : X - YPadding;

	ReportByte[0] = (0xFF & X);
	ReportByte[1] = (0xFF & (X >> 8));

	ReportByte[2] = (0xFF & Y);
	ReportByte[3] = (0xFF & (Y >> 8));
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
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up, 
		WiimoteContext->State.CoreButtons.DPad.Right, 
		WiimoteContext->State.CoreButtons.DPad.Down, 
		WiimoteContext->State.CoreButtons.DPad.Left, 
		RequestBuffer + 7);
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

	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up || WiimoteContext->ClassicControllerState.Buttons.DPad.Up,
		WiimoteContext->State.CoreButtons.DPad.Right || WiimoteContext->ClassicControllerState.Buttons.DPad.Right,
		WiimoteContext->State.CoreButtons.DPad.Down || WiimoteContext->ClassicControllerState.Buttons.DPad.Down,
		WiimoteContext->State.CoreButtons.DPad.Left || WiimoteContext->ClassicControllerState.Buttons.DPad.Left,
		RequestBuffer + 7);
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

VOID ParseWiimoteStateAsIRMouse(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ BYTE RequestBuffer[5])
{
	UNREFERENCED_PARAMETER(WiimoteContext);

	RtlSecureZeroMemory(RequestBuffer, 3);
	
	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A, RequestBuffer, 0);
	ParseButton(WiimoteContext->State.CoreButtons.B, RequestBuffer, 1);
	ParseButton(WiimoteContext->State.CoreButtons.Home, RequestBuffer, 2);

	//Axis
	ParseIRCamera(WiimoteContext->IRState.Points, RequestBuffer + 1, 128, 121);


}
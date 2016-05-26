/*

Copyright (C) 2016 Julian Löhr
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
	_Inout_ PUCHAR AxisValue,
	_In_ UCHAR LeastSignificantBitPosition,
	_In_ BYTE Bits,
	_In_ BOOLEAN Signed
)
{
	BYTE MinValue = (Signed ? 0x81 : 0x00) >> (8 - Bits);
	BYTE MaxValue = (Signed ? 0x7F : 0xFF) >> (8 - Bits);
	BYTE NullValue = (Signed ? 0x00 : 0x7F) >> (8 - Bits);

	if (MinimumValue)
	{
		(*AxisValue) |= (MinValue << LeastSignificantBitPosition);
	}
	else if (MaximumValue)
	{
		(*AxisValue) |= (MaxValue << LeastSignificantBitPosition);
	}
	else
	{
		(*AxisValue) |= (NullValue << LeastSignificantBitPosition);
	}
}

VOID
ParseButton(
	_In_ BOOLEAN ButtonValue,
	_Inout_ PUCHAR ButtonBitField,
	_In_ UCHAR LeastSignificantBitPosition
)
{
	if (ButtonValue)
	{
		(*ButtonBitField) |= (0x01 << LeastSignificantBitPosition);
	}
}

VOID
ParseAnalogAxis(
	_In_ BYTE RawValue,
	_Out_ PUCHAR AxisValue,
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

		(*AxisValue) = (0x00 | Value);
	}
	else
	{
		(*AxisValue) = RawValue;
	}

}

VOID
ParseAccelerometer(
	_In_ BYTE RawValue,
	_Out_ PUCHAR AxisValue,
	_In_ BOOLEAN Invert
)
{
	// Gravity is always present. 10 Bit Accelerometer data would have value of 96 (9,8 m/s²).
	// As only the 8 MSBs are used the value is 24. To caputure that constant pull a value range from 0 to 63 is used (32 = nullstate).
	// For a range of 63 6 bits are used.

	// Normally 0x80 (128) is the middle of the 8 bit range, so cap the accelerometer data to a range of 32 around 128.
	if (RawValue < (0x80 - 0x20))
	{
		RawValue = 0x80 - 0x20;
	}
	else if (RawValue > (0x80 + 0x1F))
	{
		RawValue = 0x80 + 0x1F;
	}

	RawValue -= (0x80 - 0x20);
	if (Invert)
	{
		RawValue = 0x3F - RawValue;
	}
	RawValue &= 0x3F;
	RawValue = RawValue << 2;

	(*AxisValue) = RawValue;
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
	_Out_ PUCHAR HatswitchBitField
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

	(*HatswitchBitField) = ValueLookUpTable[Down + 1 - Up][Right + 1 - Left];
}

UCHAR
JoinSensorValue(
	_In_ USHORT ValueOne,
	_In_ USHORT ValueTwo
)
{
	USHORT Value = (ValueOne + ValueTwo);
	return (UCHAR)(Value / 4);
}

BYTE
ParseBalanceBoardSensors(
	_In_ UCHAR PositiveValueOne,
	_In_ UCHAR PositiveValueTwo,
	_In_ UCHAR NegativeValueOne,
	_In_ UCHAR NegativeValueTwo
)
{
	UCHAR PositiveValue = JoinSensorValue(PositiveValueOne, PositiveValueTwo);
	UCHAR NegativeValue = JoinSensorValue(NegativeValueOne, NegativeValueTwo);

	BYTE Value = PositiveValue + 0x7F - NegativeValue;

	return Value;
}

UCHAR
GetCalibratedBoardValue(
	_In_ USHORT Value,
	_In_reads_(3) PUSHORT CalibrationData)
{
	/*
	Calibration - return range 0 - 255
	-----------
	0		0
		...
	1		128
		...
	2		255

	*/

	if (Value <= CalibrationData[0])
	{
		return 0;
	}

	if (Value >= CalibrationData[2])
	{
		return 255;
	}

	size_t CalibrationHigherBound = (Value > CalibrationData[1]) ? 2 : 1;

	const UCHAR SegmentRange = 128;
	USHORT RawRange = CalibrationData[CalibrationHigherBound] - CalibrationData[CalibrationHigherBound - 1];
	DWORD32 RawValue = (Value - CalibrationData[CalibrationHigherBound - 1]);

	RawValue *= SegmentRange;
	UCHAR ActualValue = (UCHAR)(RawValue / RawRange);
	ActualValue += (UCHAR)(CalibrationHigherBound - 1) * SegmentRange;

	return ActualValue;
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
	_Out_ PUSHORT XAxis,
	_Out_ PUSHORT YAxis,
	_In_ BYTE XPadding,
	_In_ BYTE YPadding
)
{
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

	(*XAxis) = RtlUshortByteSwap(X);
	(*YAxis) = RtlUshortByteSwap(Y);
}

VOID
ParseWiimoteStateAsStandaloneWiiremote(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ PHID_GAMEPAD_REPORT GamepadReport
)
{
	//Axis
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Up, WiimoteContext->State.CoreButtons.DPad.Down, &GamepadReport->XAxis, 0, 8, FALSE);
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Right, WiimoteContext->State.CoreButtons.DPad.Left, &GamepadReport->YAxis, 0, 8, FALSE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.One, &GamepadReport->Buttons[0], 0);
	ParseButton(WiimoteContext->State.CoreButtons.Two, &GamepadReport->Buttons[0], 1);
	ParseButton(WiimoteContext->State.CoreButtons.A, &GamepadReport->Buttons[0], 2);
	ParseButton(WiimoteContext->State.CoreButtons.B, &GamepadReport->Buttons[0], 3);
	ParseButton(WiimoteContext->State.CoreButtons.Plus, &GamepadReport->Buttons[0], 4);
	ParseButton(WiimoteContext->State.CoreButtons.Minus, &GamepadReport->Buttons[0], 5);
	ParseButton(WiimoteContext->State.CoreButtons.Home, &GamepadReport->Buttons[0], 6);

	//Accelerometer
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, &GamepadReport->RXAxis, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, &GamepadReport->RYAxis, TRUE);
	//ParseAccelerometer(WiimoteContext->State.Accelerometer.Z, RequestBuffer + 3, 4);
	
}

VOID
ParseWiimoteStateAsNunchuckExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ PHID_GAMEPAD_REPORT GamepadReport
)
{
	//AnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.X, &GamepadReport->XAxis, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->NunchuckState.AnalogStick.Y, &GamepadReport->YAxis, FALSE, TRUE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A, &GamepadReport->Buttons[0], 0);
	ParseButton(WiimoteContext->State.CoreButtons.B, &GamepadReport->Buttons[0], 1);
	ParseButton(WiimoteContext->NunchuckState.Buttons.C, &GamepadReport->Buttons[0], 2);
	ParseButton(WiimoteContext->NunchuckState.Buttons.Z, &GamepadReport->Buttons[0], 3);
	ParseButton(WiimoteContext->State.CoreButtons.One, &GamepadReport->Buttons[0], 4);
	ParseButton(WiimoteContext->State.CoreButtons.Two, &GamepadReport->Buttons[0], 5);
	ParseButton(WiimoteContext->State.CoreButtons.Plus, &GamepadReport->Buttons[0], 6);
	ParseButton(WiimoteContext->State.CoreButtons.Minus, &GamepadReport->Buttons[0], 7);
	ParseButton(WiimoteContext->State.CoreButtons.Home, &GamepadReport->Buttons[0], 0);

	//Accelerometer
	ParseAccelerometer(WiimoteContext->State.Accelerometer.X, &GamepadReport->RXAxis, TRUE);
	ParseAccelerometer(WiimoteContext->State.Accelerometer.Y, &GamepadReport->RYAxis, TRUE);

	//DPad
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up,
		WiimoteContext->State.CoreButtons.DPad.Right,
		WiimoteContext->State.CoreButtons.DPad.Down,
		WiimoteContext->State.CoreButtons.DPad.Left,
		&GamepadReport->Hatswitch);
}

VOID
ParseWiimoteStateAsBalanceBoard(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ PHID_GAMEPAD_REPORT GamepadReport
)
{
	UCHAR TopRight = GetCalibratedBoardValue(WiimoteContext->BalanceBoardState.Sensor.TopRight, WiimoteContext->BalanceBoardState.Calibration.TopRight);
	UCHAR BottomRight = GetCalibratedBoardValue(WiimoteContext->BalanceBoardState.Sensor.BottomRight, WiimoteContext->BalanceBoardState.Calibration.BottomRight);
	UCHAR TopLeft = GetCalibratedBoardValue(WiimoteContext->BalanceBoardState.Sensor.TopLeft, WiimoteContext->BalanceBoardState.Calibration.TopLeft);
	UCHAR BottomLeft = GetCalibratedBoardValue(WiimoteContext->BalanceBoardState.Sensor.BottomLeft, WiimoteContext->BalanceBoardState.Calibration.BottomLeft);

	GamepadReport->XAxis = ParseBalanceBoardSensors(BottomRight, TopRight, TopLeft, BottomLeft);
	GamepadReport->YAxis = ParseBalanceBoardSensors(BottomLeft, BottomRight, TopLeft, TopRight);

	// Balance Board has only a single button, that is reported as "A"
	ParseButton(WiimoteContext->State.CoreButtons.A, &GamepadReport->Buttons[0], 0);
}

VOID
ParseWiimoteStateAsClassicControllerExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ PHID_GAMEPAD_REPORT GamepadReport
)
{
	//LeftAnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.LeftAnalogStick.X, &GamepadReport->XAxis, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.LeftAnalogStick.Y, &GamepadReport->YAxis, FALSE, TRUE);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A || WiimoteContext->ClassicControllerState.Buttons.A, &GamepadReport->Buttons[0], 0);
	ParseButton(WiimoteContext->State.CoreButtons.B || WiimoteContext->ClassicControllerState.Buttons.B, &GamepadReport->Buttons[0], 1);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.Y, &GamepadReport->Buttons[0], 2);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.X, &GamepadReport->Buttons[0], 3);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.L, &GamepadReport->Buttons[0], 4);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.R, &GamepadReport->Buttons[0], 5);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.ZL, &GamepadReport->Buttons[0], 6);
	ParseButton(WiimoteContext->ClassicControllerState.Buttons.ZR, &GamepadReport->Buttons[0], 7);
	ParseButton(WiimoteContext->State.CoreButtons.Plus || WiimoteContext->ClassicControllerState.Buttons.Plus, &GamepadReport->Buttons[1], 0);
	ParseButton(WiimoteContext->State.CoreButtons.Minus || WiimoteContext->ClassicControllerState.Buttons.Minus, &GamepadReport->Buttons[1], 1);
	ParseButton(WiimoteContext->State.CoreButtons.Home || WiimoteContext->ClassicControllerState.Buttons.Home, &GamepadReport->Buttons[1], 2);
	ParseButton(WiimoteContext->State.CoreButtons.One || WiimoteContext->ClassicControllerState.Buttons.LH, &GamepadReport->Buttons[1], 3);
	ParseButton(WiimoteContext->State.CoreButtons.Two || WiimoteContext->ClassicControllerState.Buttons.RH, &GamepadReport->Buttons[1], 4);

	//Right Analog Stick as Second Axis
	ParseTrigger(WiimoteContext->ClassicControllerState.LeftTrigger, &GamepadReport->ZAxis);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.RightAnalogStick.X, &GamepadReport->RXAxis, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->ClassicControllerState.RightAnalogStick.Y, &GamepadReport->RYAxis, FALSE, TRUE);
	ParseTrigger(WiimoteContext->ClassicControllerState.RightTrigger, &GamepadReport->RZAxis);

	//DPad
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up || WiimoteContext->ClassicControllerState.Buttons.DPad.Up,
		WiimoteContext->State.CoreButtons.DPad.Right || WiimoteContext->ClassicControllerState.Buttons.DPad.Right,
		WiimoteContext->State.CoreButtons.DPad.Down || WiimoteContext->ClassicControllerState.Buttons.DPad.Down,
		WiimoteContext->State.CoreButtons.DPad.Left || WiimoteContext->ClassicControllerState.Buttons.DPad.Left,
		&GamepadReport->Hatswitch);
}

VOID
ParseWiimoteStateAsGuitarExtension(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Out_ PHID_GAMEPAD_REPORT GamepadReport
)
{
	// LeftAnalogStick as Axis
	ParseAnalogAxis(WiimoteContext->GuitarState.AnalogStick.X, &GamepadReport->XAxis, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->GuitarState.AnalogStick.Y, &GamepadReport->YAxis, FALSE, TRUE);

	// Buttons
	ParseButton(WiimoteContext->GuitarState.Buttons.Green, &GamepadReport->Buttons[0], 0);
	ParseButton(WiimoteContext->GuitarState.Buttons.Red, &GamepadReport->Buttons[0], 1);
	ParseButton(WiimoteContext->GuitarState.Buttons.Yellow, &GamepadReport->Buttons[0], 2);
	ParseButton(WiimoteContext->GuitarState.Buttons.Blue, &GamepadReport->Buttons[0], 3);
	ParseButton(WiimoteContext->GuitarState.Buttons.Orange, &GamepadReport->Buttons[0], 4);
	ParseButton(WiimoteContext->GuitarState.Buttons.Up, &GamepadReport->Buttons[0], 5);
	ParseButton(WiimoteContext->GuitarState.Buttons.Down, &GamepadReport->Buttons[0], 6);
	ParseButton(WiimoteContext->State.CoreButtons.Plus || WiimoteContext->GuitarState.Buttons.Plus, &GamepadReport->Buttons[1], 0);
	ParseButton(WiimoteContext->State.CoreButtons.Minus || WiimoteContext->GuitarState.Buttons.Minus, &GamepadReport->Buttons[1], 1);
	ParseButton(WiimoteContext->State.CoreButtons.One, &GamepadReport->Buttons[1], 2);
	ParseButton(WiimoteContext->State.CoreButtons.Two, &GamepadReport->Buttons[1], 3);
	ParseButton(WiimoteContext->State.CoreButtons.A, &GamepadReport->Buttons[1], 4);
	ParseButton(WiimoteContext->State.CoreButtons.B, &GamepadReport->Buttons[1], 5);
	ParseButton(WiimoteContext->State.CoreButtons.Home, &GamepadReport->Buttons[1], 6);

	// Analog Bars
	ParseAnalogAxis(WiimoteContext->GuitarState.WhammyBar, &GamepadReport->RXAxis, FALSE, FALSE);
	ParseAnalogAxis(WiimoteContext->GuitarState.TouchBar, &GamepadReport->RYAxis, FALSE, TRUE);

	// DPad
	ParseDPad(
		WiimoteContext->State.CoreButtons.DPad.Up,
		WiimoteContext->State.CoreButtons.DPad.Right,
		WiimoteContext->State.CoreButtons.DPad.Down,
		WiimoteContext->State.CoreButtons.DPad.Left,
		&GamepadReport->Hatswitch);
}


VOID
ParseWiimoteStateAsGamepad(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_all_(BufferSize) PVOID Buffer,
	_In_ size_t BufferSize,
	_Out_ PSIZE_T BytesWritten
)
{
	PHID_GAMEPAD_REPORT GamepadReport = (PHID_GAMEPAD_REPORT)Buffer;
	if (BufferSize < sizeof(HID_GAMEPAD_REPORT))
	{
		Trace("Gamepad Report Buffer seems to be too small: %d - %d", BufferSize, sizeof(HID_GAMEPAD_REPORT));
	}
	//ASSERT(BufferSize < sizeof(HID_GAMEPAD_REPORT));
	HID_GAMEPAD_REPORT_INIT(GamepadReport);

	switch (WiimoteContext->Extension)
	{
	case None:
		ParseWiimoteStateAsStandaloneWiiremote(WiimoteContext, GamepadReport);
		break;
	case Nunchuck:
		ParseWiimoteStateAsNunchuckExtension(WiimoteContext, GamepadReport);
		break;
	case BalanceBoard:
		ParseWiimoteStateAsBalanceBoard(WiimoteContext, GamepadReport);
		break;
	case ClassicController:
	case WiiUProController:
		ParseWiimoteStateAsClassicControllerExtension(WiimoteContext, GamepadReport);
		break;
	case Guitar:
		ParseWiimoteStateAsGuitarExtension(WiimoteContext, GamepadReport);
	default:
		break;
	}

	(*BytesWritten) = sizeof(HID_GAMEPAD_REPORT);
}

VOID ParseWiimoteStateAsDPadMouse(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_all_(BufferSize) PVOID Buffer,
	_In_ size_t BufferSize,
	_Out_ PSIZE_T BytesWritten
)
{
	PHID_DPADMOUSE_REPORT DPadMouseReport = (PHID_DPADMOUSE_REPORT)Buffer;
	if (BufferSize < sizeof(HID_DPADMOUSE_REPORT))
	{
		Trace("DPadMouse Report Buffer seems to be too small: %d - %d", BufferSize, sizeof(HID_DPADMOUSE_REPORT));
	}
	//ASSERT(BufferSize < sizeof(HID_DPADMOUSE_REPORT));
	HID_DPADMOUSE_REPORT_INIT(DPadMouseReport);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.One, &DPadMouseReport->Buttons, 0);
	ParseButton(WiimoteContext->State.CoreButtons.Two, &DPadMouseReport->Buttons, 1);
	ParseButton(WiimoteContext->State.CoreButtons.B, &DPadMouseReport->Buttons, 2);

	//Axis
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Up, WiimoteContext->State.CoreButtons.DPad.Down, &DPadMouseReport->X, 6, 2, TRUE);
	ParseBooleanAxis(WiimoteContext->State.CoreButtons.DPad.Right, WiimoteContext->State.CoreButtons.DPad.Left, &DPadMouseReport->Y, 6, 2, TRUE);

	(*BytesWritten) = sizeof(HID_DPADMOUSE_REPORT);
}

VOID ParseWiimoteStateAsIRMouse(
	_In_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_updates_all_(BufferSize) PVOID Buffer,
	_In_ size_t BufferSize,
	_Out_ PSIZE_T BytesWritten
)
{
	PHID_IRMOUSE_REPORT IRMouseReport = (PHID_IRMOUSE_REPORT)Buffer;
	if (BufferSize < sizeof(HID_IRMOUSE_REPORT))
	{
		Trace("IRMouse Report Buffer seems to be too small: %d - %d", BufferSize, sizeof(HID_IRMOUSE_REPORT));
	}
	//ASSERT(BufferSize < sizeof(HID_IRMOUSE_REPORT));
	HID_IRMOUSE_REPORT_INIT(IRMouseReport);

	//Buttons
	ParseButton(WiimoteContext->State.CoreButtons.A, &IRMouseReport->Buttons, 0);
	ParseButton(WiimoteContext->State.CoreButtons.B, &IRMouseReport->Buttons, 1);
	ParseButton(WiimoteContext->State.CoreButtons.Home, &IRMouseReport->Buttons, 2);

	//Axis
	ParseIRCamera(WiimoteContext->IRState.Points, &IRMouseReport->X, &IRMouseReport->Y, 128, 121);

	(*BytesWritten) = sizeof(HID_IRMOUSE_REPORT);
}

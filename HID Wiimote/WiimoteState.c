/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteState.c

Abstract:
	Parses Wiimote Reports into State Buffers
*/
#include "WiimoteState.h"

#include "Device.h"

NTSTATUS UpdateCoreButtons(_Inout_ PWIIMOTE_WII_REMOTE_STATE WiiRemoteState, _In_reads_bytes_(2) PUCHAR ButtonData, _In_ size_t ButtonDataSize);
NTSTATUS UpdateAccelerometer(_Inout_ PWIIMOTE_WII_REMOTE_STATE WiiRemoteState, _In_reads_bytes_(3) PUCHAR AccelerometerData, _In_ size_t AccelerometerDataSize);
NTSTATUS UpdateExtension(_Inout_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, _In_reads_bytes_(DataSize) PUCHAR Data, _In_ size_t DataSize);
NTSTATUS UpdateNunchuck(_Inout_ PWIIMOTE_NUNCHUCK_STATE NunchuckState, _In_reads_bytes_(6) PUCHAR NunchuckData, _In_ size_t NunchuckDataSize);
NTSTATUS UpdateBalanceBoard(_Inout_ PWIIMOTE_BALANCE_BOARD_STATE BalanceBoardState, _In_reads_bytes_(8) PUCHAR BalanceBoardData, _In_ size_t BalanceBoardDataSize);
NTSTATUS UpdateClassicController(_Inout_ PWIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState, _In_reads_bytes_(6) PUCHAR ClassicControllerData, _In_ size_t ClassicControllerDataSize);
NTSTATUS UpdateWiiUProController(_Inout_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext, _Inout_ PWIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState, _In_reads_bytes_(11) PUCHAR WiiUProControllerData, _In_ size_t WiiUProControllerDataSize);
NTSTATUS UpdateGuitar(_Inout_ PWIIMOTE_GUITAR_STATE GuitarState, _In_reads_bytes_(6) PUCHAR GuitarData, _In_ size_t GuitarDataSize);
NTSTATUS UpdateIRCamera(_In_ PWIIMOTE_IR_STATE IRState, _In_reads_bytes_(10) PUCHAR IRData, _In_ size_t IRDataSize, _Out_ PBOOLEAN IRDataIsValid);

VOID UpdateClassicControllerButtons(_Inout_ PWIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState, _In_reads_bytes_(2) PUCHAR ButtonData);
BOOLEAN UpdateIRCameraPoint(_In_ PWIIMOTE_IR_POINT IRPointData1, _In_ PWIIMOTE_IR_POINT IRPointData2, _In_reads_bytes_(5) PUCHAR IRData);

VOID WiimoteStateResetToNullState(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	PWIIMOTE_STATE WiimoteState = &DeviceContext->WiimoteContext.State;

	// Reset all to zero => No buttons pressed
	RtlSecureZeroMemory(WiimoteState, sizeof(WIIMOTE_STATE));

	// Reset Accelerometers to null state
	WiimoteState->WiiRemoteState.Accelerometer.X = 0x80;
	WiimoteState->WiiRemoteState.Accelerometer.Y = 0x80;
	WiimoteState->WiiRemoteState.Accelerometer.Z = 0x80;

	// Reset IR data to "invalid data"
	for (size_t i = 0; i < WIIMOTE_IR_POINTS_BUFFER_SIZE; ++i)
		for (size_t j = 0; j < WIIMOTE_IR_POINTS; j++)
		{
			DeviceContext->WiimoteContext.State.IRState.Points[i][j].X = 0x3FF;
			DeviceContext->WiimoteContext.State.IRState.Points[i][j].Y = 0x3FF;
		}

	HIDWiimoteStateUpdated(DeviceContext);
}

NTSTATUS WiimoteStateUpdate(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(DataSize) PUCHAR Data,
	_In_ _In_range_(1, SIZE_MAX) size_t DataSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BOOLEAN ForwardUpdateToHID = TRUE;
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext = &(DeviceContext->WiimoteContext);
	PWIIMOTE_WII_REMOTE_STATE WiiRemoteState = &(WiimoteContext->State.WiiRemoteState);
	BYTE ReportID = Data[0];

	//Every Report but 0x3D has Core Buttons
	if (ReportID != 0x3D)
	{
		Status = UpdateCoreButtons(WiiRemoteState, Data + 1, DataSize - 1);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
	}

	// Process additional payload
	switch (ReportID)
	{
	case 0x31: // Accelerometer
		Status = UpdateAccelerometer(WiiRemoteState, Data + 3, DataSize - 3);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		break;
	case 0x32: // 8 Byte Extension
	case 0x34: // 19 Byte Extension
		Status = UpdateExtension(WiimoteContext, Data + 3, DataSize - 3);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		break;
	case 0x35: // Accelerometer & 16 Byte Extension
		Status = UpdateAccelerometer(WiiRemoteState, Data + 3, DataSize - 3);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		Status = UpdateExtension(WiimoteContext, Data + 6, DataSize - 6);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		break;
	case 0x36: // 10 Byte IR & 9 Byte Extension
		Status = UpdateIRCamera(&(WiimoteContext->State.IRState), Data + 3, DataSize - 3, &ForwardUpdateToHID);
		if (NT_SUCCESS(Status))
		{
			return Status;
		}
		break;
	}

	if (ForwardUpdateToHID)
	{
		HIDWiimoteStateUpdated(DeviceContext);
	}

	return Status;
}

NTSTATUS
UpdateCoreButtons(
	_Inout_ PWIIMOTE_WII_REMOTE_STATE WiiRemoteState,
	_In_reads_bytes_(2) PUCHAR ButtonData,
	_In_ size_t ButtonDataSize
	)
{
	if (ButtonDataSize < 2)
	{
		Trace("Data Buffer too small to read Core Buttons");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	WiiRemoteState->CoreButtons.A = (ButtonData[1] & 0x08);
	WiiRemoteState->CoreButtons.B = (ButtonData[1] & 0x04);
	WiiRemoteState->CoreButtons.One = (ButtonData[1] & 0x02);
	WiiRemoteState->CoreButtons.Two = (ButtonData[1] & 0x01);
	WiiRemoteState->CoreButtons.Minus = (ButtonData[1] & 0x10);
	WiiRemoteState->CoreButtons.Plus = (ButtonData[0] & 0x10);
	WiiRemoteState->CoreButtons.Home = (ButtonData[1] & 0x80);

	WiiRemoteState->CoreButtons.DPad.Left = (ButtonData[0] & 0x01);
	WiiRemoteState->CoreButtons.DPad.Right = (ButtonData[0] & 0x02);
	WiiRemoteState->CoreButtons.DPad.Down = (ButtonData[0] & 0x04);
	WiiRemoteState->CoreButtons.DPad.Up = (ButtonData[0] & 0x08);

	return STATUS_SUCCESS;
}

NTSTATUS
UpdateAccelerometer(
	_Inout_ PWIIMOTE_WII_REMOTE_STATE WiiRemoteState,
	_In_reads_bytes_(3) PUCHAR AccelerometerData,
	_In_ size_t AccelerometerDataSize
	)
{
	if (AccelerometerDataSize < 3)
	{
		Trace("Data Buffer too small to Update Accelerometer Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	/*
	  Accelerometer data is originally 10 Bits wide. 8 MSBs are carried in the Accelermoter bytes.
	  The 2 LSBs are packed into the Core Button bytes. (Only for X are actually 2 bits packed; for Y and Z only the second LSB is provided)
	  For further processing only the 8 MSB are used.
	*/
	WiiRemoteState->Accelerometer.X = AccelerometerData[0];
	WiiRemoteState->Accelerometer.Y = AccelerometerData[1];
	WiiRemoteState->Accelerometer.Z = AccelerometerData[2];

	return STATUS_SUCCESS;
}

NTSTATUS UpdateExtension(
	_Inout_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_In_reads_bytes_(DataSize) PUCHAR Data,
	_In_ size_t DataSize
	)
{
	switch (WiimoteContext->Extension)
	{
	case Nunchuck:
		return UpdateNunchuck(&(WiimoteContext->State.NunchuckState), Data, DataSize);
	case BalanceBoard:
		return UpdateBalanceBoard(&(WiimoteContext->State.BalanceBoardState), Data, DataSize);
	case ClassicController:
		return UpdateClassicController(&(WiimoteContext->State.ClassicControllerState), Data, DataSize);
	case WiiUProController:
		return UpdateWiiUProController(WiimoteContext, &(WiimoteContext->State.ClassicControllerState), Data, DataSize);
	case Guitar:
		return UpdateGuitar(&(WiimoteContext->State.GuitarState), Data, DataSize);
	default:
		return STATUS_SUCCESS;
	}
}

NTSTATUS
UpdateNunchuck(
	_Inout_ PWIIMOTE_NUNCHUCK_STATE NunchuckState,
	_In_reads_bytes_(6) PUCHAR NunchuckData,
	_In_ size_t NunchuckDataSize
	)
{
	if (NunchuckDataSize < 6)
	{
		Trace("Data Buffer too small to read Nunchuck Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	BYTE ButtonData = ~NunchuckData[5];

	//Buttons
	NunchuckState->Buttons.Z = ButtonData & 0x01;
	NunchuckState->Buttons.C = ButtonData & 0x02;

	//Analog Stick
	NunchuckState->AnalogStick.X = NunchuckData[0];
	NunchuckState->AnalogStick.Y = NunchuckData[1];

	//Accelerometer
	NunchuckState->Accelerometer.X = NunchuckData[2];
	NunchuckState->Accelerometer.Y = NunchuckData[3];
	NunchuckState->Accelerometer.Z = NunchuckData[4];

	return STATUS_SUCCESS;
}

NTSTATUS
UpdateBalanceBoard(
	_Inout_ PWIIMOTE_BALANCE_BOARD_STATE BalanceBoardState,
	_In_reads_bytes_(8) PUCHAR BalanceBoardData,
	_In_ size_t BalanceBoardDataSize
	)
{
	if (BalanceBoardDataSize < 8)
	{
		Trace("Data Buffer too small to read Balance Board Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	// Sensor Data is Big-Endian, Windows is always Little-Endian
	BalanceBoardState->Sensor[WIIMOTE_BALANCE_BOARD_TOP_RIGHT] = RtlUshortByteSwap(*((PUSHORT)(BalanceBoardData + 0)));
	BalanceBoardState->Sensor[WIIMOTE_BALANCE_BOARD_BOTTOM_RIGHT] = RtlUshortByteSwap(*((PUSHORT)(BalanceBoardData + 2)));
	BalanceBoardState->Sensor[WIIMOTE_BALANCE_BOARD_TOP_LEFT] = RtlUshortByteSwap(*((PUSHORT)(BalanceBoardData + 4)));
	BalanceBoardState->Sensor[WIIMOTE_BALANCE_BOARD_BOTTOM_LEFT] = RtlUshortByteSwap(*((PUSHORT)(BalanceBoardData + 6)));

	return STATUS_SUCCESS;
}

NTSTATUS
UpdateClassicController(
	_Inout_ PWIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState,
	_In_reads_bytes_(6) PUCHAR ClassicControllerData,
	_In_ size_t ClassicControllerDataSize
	)
{
	if (ClassicControllerDataSize < 6)
	{
		Trace("Data Buffer too small to read Classic Controller Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	//Buttons
	UpdateClassicControllerButtons(ClassicControllerState, ClassicControllerData + 4);

	//Analog Sticks
	ClassicControllerState->LeftAnalogStick.X = 0xFF & ((0x3F & ClassicControllerData[0]) << 2);
	ClassicControllerState->LeftAnalogStick.Y = 0xFF & ((0x3F & ClassicControllerData[1]) << 2);

	ClassicControllerState->RightAnalogStick.X = 0xFF & ((0xC0 & ClassicControllerData[0]));
	ClassicControllerState->RightAnalogStick.X |= 0xFF & ((0xC0 & ClassicControllerData[1]) >> 2);
	ClassicControllerState->RightAnalogStick.X |= 0xFF & ((0x80 & ClassicControllerData[2]) >> 4);
	ClassicControllerState->RightAnalogStick.Y = 0xFF & ((0x1F & ClassicControllerData[2]) << 3);

	//Trigger
	ClassicControllerState->LeftTrigger = 0xFF & ((0x60 & ClassicControllerData[2]));
	ClassicControllerState->LeftTrigger |= 0xFF & ((0xE0 & ClassicControllerData[3]) >> 3);
	ClassicControllerState->RightTrigger = 0xFF & ((0x1F & ClassicControllerData[3]) << 2);

	//Not Supported Input
	ClassicControllerState->Buttons.LH = FALSE;
	ClassicControllerState->Buttons.RH = FALSE;

	return STATUS_SUCCESS;
}

NTSTATUS
UpdateWiiUProController(
	_Inout_ PWIIMOTE_DEVICE_CONTEXT WiimoteContext,
	_Inout_ PWIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState,
	_In_reads_bytes_(11) PUCHAR WiiUProControllerData,
	_In_ size_t WiiUProControllerDataSize
	)
{
	if (WiiUProControllerDataSize < 11)
	{
		Trace("Data Buffer too small to read Wii U Pro Controller Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	BYTE BatteryLevel = 0;
	BYTE ButtonData = ~WiiUProControllerData[10];

	//Buttons
	UpdateClassicControllerButtons(ClassicControllerState, WiiUProControllerData + 8);

	ClassicControllerState->Buttons.LH = (ButtonData & 0x02);
	ClassicControllerState->Buttons.RH = (ButtonData & 0x01);

	//AnalogSticks
	ClassicControllerState->LeftAnalogStick.X = 0xFF & ((WiiUProControllerData[0] >> 4) | (WiiUProControllerData[1] << 4));
	ClassicControllerState->LeftAnalogStick.Y = 0xFF & ((WiiUProControllerData[4] >> 4) | (WiiUProControllerData[5] << 4));

	ClassicControllerState->RightAnalogStick.X = 0xFF & ((WiiUProControllerData[2] >> 4) | (WiiUProControllerData[3] << 4));
	ClassicControllerState->RightAnalogStick.Y = 0xFF & ((WiiUProControllerData[6] >> 4) | (WiiUProControllerData[7] << 4));

	//Not Supported Input
	ClassicControllerState->LeftTrigger = ClassicControllerState->Buttons.ZL ? 0xFF : 0x00;
	ClassicControllerState->RightTrigger = ClassicControllerState->Buttons.ZR ? 0xFF : 0x00;

	//Battery Level
	switch (WiiUProControllerData[10] & 0xF0)
	{
	case 0xC0:
		BatteryLevel = 0xFF;
		break;
	case 0xB0:
		BatteryLevel = 0xBF;
		break;
	case 0xA0:
		BatteryLevel = 0x7F;
		break;
	case 0x90:
		BatteryLevel = 0x3F;
		break;
	case 0x80:
	default:
		BatteryLevel = 0x10;
	}

	WiimoteContext->BatteryLevel = BatteryLevel;

	return STATUS_SUCCESS;
}

VOID
UpdateClassicControllerButtons(
	_Inout_ PWIIMOTE_CLASSIC_CONTROLLER_STATE ClassicControllerState,
	_In_reads_bytes_(2) PUCHAR ButtonData
)
{
	BYTE InvertedButtonData[2] = { 0 };
	InvertedButtonData[0] = ~ButtonData[0];
	InvertedButtonData[1] = ~ButtonData[1];

	ClassicControllerState->Buttons.A = InvertedButtonData[1] & 0x10;
	ClassicControllerState->Buttons.B = InvertedButtonData[1] & 0x40;
	ClassicControllerState->Buttons.Y = InvertedButtonData[1] & 0x20;
	ClassicControllerState->Buttons.X = InvertedButtonData[1] & 0x08;
	ClassicControllerState->Buttons.Home = InvertedButtonData[0] & 0x08;
	ClassicControllerState->Buttons.Plus = InvertedButtonData[0] & 0x04;
	ClassicControllerState->Buttons.Minus = InvertedButtonData[0] & 0x10;

	ClassicControllerState->Buttons.DPad.Up = InvertedButtonData[1] & 0x01;
	ClassicControllerState->Buttons.DPad.Right = InvertedButtonData[0] & 0x80;
	ClassicControllerState->Buttons.DPad.Down = InvertedButtonData[0] & 0x40;
	ClassicControllerState->Buttons.DPad.Left = InvertedButtonData[1] & 0x02;

	ClassicControllerState->Buttons.L = InvertedButtonData[0] & 0x20;
	ClassicControllerState->Buttons.ZL = InvertedButtonData[1] & 0x80;
	ClassicControllerState->Buttons.R = InvertedButtonData[0] & 0x02;
	ClassicControllerState->Buttons.ZR = InvertedButtonData[1] & 0x04;
}

NTSTATUS
UpdateGuitar(
	_Inout_ PWIIMOTE_GUITAR_STATE GuitarState,
	_In_reads_bytes_(6) PUCHAR GuitarData,
	_In_ size_t GuitarDataSize
	)
{
	if (GuitarDataSize < 6)
	{
		Trace("Data Buffer too small to read Guitar Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	BYTE ButtonData[2] = { 0 };
	ButtonData[0] = ~GuitarData[0];
	ButtonData[1] = ~GuitarData[1];

	// Buttons
	GuitarState->Buttons.Green = ButtonData[1] & 0x10;
	GuitarState->Buttons.Red = ButtonData[1] & 0x40;
	GuitarState->Buttons.Yellow = ButtonData[1] & 0x08;
	GuitarState->Buttons.Blue = ButtonData[1] & 0x20;
	GuitarState->Buttons.Orange = ButtonData[1] & 0x80;
	GuitarState->Buttons.Plus = ButtonData[0] & 0x04;
	GuitarState->Buttons.Minus = ButtonData[0] & 0x10;
	GuitarState->Buttons.Up = ButtonData[1] & 0x01;
	GuitarState->Buttons.Down = ButtonData[0] & 0x40;

	// Analog Sticks
	GuitarState->AnalogStick.X = 0xFF & ((0x3F & GuitarData[0]) << 2);
	GuitarState->AnalogStick.Y = 0xFF & ((0x3F & GuitarData[1]) << 2);

	// Analog Bars
	GuitarState->WhammyBar = 0xFF & ((0x1F & GuitarData[0]) << 3);
	GuitarState->TouchBar = 0xFF & ((0x1F & GuitarData[1]) << 3);

	return STATUS_SUCCESS;
}

NTSTATUS
UpdateIRCamera(
	_In_ PWIIMOTE_IR_STATE IRState,
	_In_reads_bytes_(10) PUCHAR IRData,
	_In_ size_t IRDataSize,
	_Out_ PBOOLEAN IRDataIsValid
	)
{
	if (IRDataSize < 10)
	{
		Trace("Data Buffer too small to read IR Data");
		return STATUS_INVALID_BUFFER_SIZE;
	}

	BYTE BufferIndex = IRState->PointsBufferPointer++;

	IRState->PointsBufferPointer %= WIIMOTE_IR_POINTS_BUFFER_SIZE;
	(*IRDataIsValid) = FALSE;

	(*IRDataIsValid) |= UpdateIRCameraPoint(&(IRState->Points[BufferIndex][0]), &(IRState->Points[BufferIndex][1]), IRData);
	(*IRDataIsValid) |= UpdateIRCameraPoint(&(IRState->Points[BufferIndex][2]), &(IRState->Points[BufferIndex][3]), IRData + 5);

	return STATUS_SUCCESS;
}

BOOLEAN
UpdateIRCameraPoint(
	_In_ PWIIMOTE_IR_POINT IRPointData1,
	_In_ PWIIMOTE_IR_POINT IRPointData2,
	_In_reads_bytes_(5) PUCHAR IRData
	)
{
	USHORT X1 = IRData[0];
	USHORT Y1 = IRData[1];
	USHORT X2 = IRData[3];
	USHORT Y2 = IRData[4];

	X1 |= (0x30 & IRData[2]) << 4;
	Y1 |= (0xC0 & IRData[2]) << 2;
	X2 |= (0x03 & IRData[2]) << 8;
	Y2 |= (0x0C & IRData[2]) << 6;

	IRPointData1->X = X1;
	IRPointData1->Y = Y1;
	IRPointData2->X = X2;
	IRPointData2->Y = Y2;

	// Max for Y is 767, so 0x3FF Value means no Data
	return ((Y1 != 0x3FF) || (Y2 != 0x3FF));
}

/*

Copyright (C) 2013 Julian L�hr
All rights reserved.

Filename:
	Wiimote.c

Abstract:
	Manages the Wiimote states and is the layer between HID and Bluetooth.
*/

#include "Wiimote.h"
#include "Device.h"
#include "Bluetooth.h"

NTSTATUS 
PrepareWiimote(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext;
	WDF_TIMER_CONFIG TimerConfig;
	WDF_OBJECT_ATTRIBUTES TimerAttributes;

	WiimoteContext = &(DeviceContext->WiimoteContext);

	RtlSecureZeroMemory(&(WiimoteContext->State), sizeof(WiimoteContext->State));

	//Create Timer to request StatusInformation

	WDF_TIMER_CONFIG_INIT_PERIODIC(&TimerConfig, StatusInformationTimerExpired, WIIMOTE_STATUSINFORMATION_INTERVAL);
	WDF_OBJECT_ATTRIBUTES_INIT(&TimerAttributes);
	TimerAttributes.ParentObject = DeviceContext->Device;
	
	Status = WdfTimerCreate(&TimerConfig, &TimerAttributes, &(WiimoteContext->StatusInformationTimer));
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
SetLEDs(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE LedFlag
	)
{
	CONST size_t BufferSize = 3;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x11;	//Player LED
	Data[2] = LedFlag;

	Status = TransferToDevice(DeviceContext, Request, Memory, FALSE);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;

}

NTSTATUS
SetReportMode(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE ReportMode
	)
{
	CONST size_t BufferSize = 4;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x12;	//Set ReportMode
	Data[2] = 0x00;	//Only On Change
	Data[3] = ReportMode; //Mode

	Status = TransferToDevice(DeviceContext, Request, Memory, FALSE);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	DeviceContext->WiimoteContext.CurrentReportMode = ReportMode;

	return Status;
}

NTSTATUS
RequestStatusInformation(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	CONST size_t BufferSize = 3;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x15;	//Status Information Request
	Data[2] = 0x00;	//Rumble Off

	Status = TransferToDevice(DeviceContext, Request, Memory, FALSE);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
ReadFromRegister(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ INT32 Address,
_In_ BYTE Size
)
{
	CONST size_t BufferSize = 8;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x17;	//Status Information Request
	Data[2] = 0x04;	//Rumble Off but write to Registers

	//Offset
	Data[3] = 0xFF & (Address >> 16);
	Data[4] = 0xFF & (Address >> 8);
	Data[5] = 0xFF & (Address);

	Data[6] = 0x00; //Size
	Data[7] = Size; //Size

	Status = TransferToDevice(DeviceContext, Request, Memory, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
WriteToRegister(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ const INT32 Address,
_In_ const BYTE * Data,
_In_ const BYTE DataSize
)
{
	CONST size_t BufferSize = 23;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Report;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Report);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Report[0] = 0xA2;	//HID Output Report
	Report[1] = 0x16;	//Status Information Request
	Report[2] = 0x04;	//Rumble Off but write to Registers
	
	//Offset
	Report[3] = 0xFF & (Address >> 16);
	Report[4] = 0xFF & (Address >> 8);
	Report[5] = 0xFF & (Address);

	Report[6] = DataSize; //Size

	RtlSecureZeroMemory(Report + 7, 16);
	RtlCopyBytes(Report + 7, Data, DataSize);

	Status = TransferToDevice(DeviceContext, Request, Memory, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
WriteSingeByteToRegister(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ const INT32 Address,
_In_ const BYTE Data
)
{
	return WriteToRegister(DeviceContext, Address, &Data, 1);
}

NTSTATUS
SendSingleByteReport(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE ReportID,
_In_ BYTE Byte
)
{
	CONST size_t BufferSize = 3;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = ReportID;	// Enable IR 
	Data[2] = Byte; // Get acknowledgement 


	Status = TransferToDevice(DeviceContext, Request, Memory, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS 
StartWiimote(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	DeviceContext->WiimoteContext.Extension = None;

	//Set LEDs
	Status = SetLEDs(DeviceContext, WIIMOTE_LEDS_FOUR);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

#ifdef MOUSE_IR
	//Enable IR
	Status = SendSingleByteReport(DeviceContext, 0x13, 0x06);
	if(!NT_SUCCESS(Status))
	{
		Trace("EnableIR Failed: 0x%x", Status);
		return Status;
	}
#else
	//Set Report Mode
	Status = SetReportMode(DeviceContext, 0x31);
	if(!NT_SUCCESS(Status))
	{
		Trace("SetReportMode Failed: 0x%x", Status);
		return Status;
	}
#endif

	//Start Continious Reader
	Status = StartContiniousReader(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		Trace("StartContiniousReader Failed: 0x%x", Status);
		return Status;
	}

	//Start Timer
	WdfTimerStart(DeviceContext->WiimoteContext.StatusInformationTimer, WDF_REL_TIMEOUT_IN_SEC(1));

	return Status;
}

NTSTATUS
EnableIR(
_In_ PDEVICE_CONTEXT DeviceContext
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	const BYTE SensivityBlock1[] = { 0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0xaa, 0x00, 0x64 };
	const BYTE SensivityBlock2[] = { 0x63, 0x03 };

	//Write 0x08 to register 0xb00030
	Status = WriteSingeByteToRegister(DeviceContext, 0xb00030, 0x08);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Write Sensitivity Block 1 to registers at 0xb00000
	Status = WriteToRegister(DeviceContext, 0xb00000, SensivityBlock1, sizeof(SensivityBlock1));
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Write Sensitivity Block 2 to registers at 0xb0001a
	Status = WriteToRegister(DeviceContext, 0xb0001a, SensivityBlock2, sizeof(SensivityBlock2));
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Write Mode Number to register 0xb00033
	//0x01 Basic Mode 10 Byte
	Status = WriteSingeByteToRegister(DeviceContext, 0xb00030, 0x01);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}
	//Write 0x08 to register 0xb00030 (again)
	Status = WriteSingeByteToRegister(DeviceContext, 0xb00030, 0x08);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Set Report Mode
	Status = SetReportMode(DeviceContext, 0x36);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
SuspendWiimote(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	CONST size_t BufferSize = 1;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Get Resources
	Status = CreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0x13;	//HID Control with Suspend: 0x10 = HID_Control - 0x03 = SUSPEND

	Status = TransferToDevice(DeviceContext, Request, Memory, TRUE);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;

}

NTSTATUS
StopWiimote(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	//Stop Timer
	WdfTimerStop(DeviceContext->WiimoteContext.StatusInformationTimer, TRUE);

	//Shut down Wiimote
	Status = SuspendWiimote(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}


	return Status;
}


NTSTATUS
ProcessBatteryLevel(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE BatteryLevel
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BYTE BatteryFlag = WIIMOTE_LEDS_ONE;
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext = &(DeviceContext->WiimoteContext);
	
	Trace("BatteryLevel: 0x%x", BatteryLevel);

	BatteryFlag = ((0xF0 << (3 - (BatteryLevel / 64))) & 0xF0);
	
	Trace("BatteryFlag: 0x%x", BatteryFlag);

Status = SetLEDs(DeviceContext, BatteryFlag);
if (!NT_SUCCESS(Status))
{
	return Status;
}

if (BatteryFlag == WIIMOTE_LEDS_ONE)
{
	WdfTimerStop(WiimoteContext->StatusInformationTimer, FALSE);
}

return Status;
}

VOID
XorData(
_In_ BYTE * Source,
_Inout_ BYTE * Dst,
_In_ SIZE_T Length
)
{
	for (SIZE_T i = 0; i < Length; i++)
	{
		Dst[i] = Source[i] ^ 0xFF;
	}
}

VOID
ExtractCoreButtons(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE RawCoreButtons[2]
)
{
	PWIIMOTE_STATE WiimoteState = &(DeviceContext->WiimoteContext.State);

	WiimoteState->CoreButtons.A = (RawCoreButtons[1] & 0x08);
	WiimoteState->CoreButtons.B = (RawCoreButtons[1] & 0x04);
	WiimoteState->CoreButtons.One = (RawCoreButtons[1] & 0x02);
	WiimoteState->CoreButtons.Two = (RawCoreButtons[1] & 0x01);
	WiimoteState->CoreButtons.Minus = (RawCoreButtons[1] & 0x10);
	WiimoteState->CoreButtons.Plus = (RawCoreButtons[0] & 0x10);
	WiimoteState->CoreButtons.Home = (RawCoreButtons[1] & 0x80);

	WiimoteState->CoreButtons.DPad.Left = (RawCoreButtons[0] & 0x01);
	WiimoteState->CoreButtons.DPad.Right = (RawCoreButtons[0] & 0x02);
	WiimoteState->CoreButtons.DPad.Down = (RawCoreButtons[0] & 0x04);
	WiimoteState->CoreButtons.DPad.Up = (RawCoreButtons[0] & 0x08);
}

VOID
ExtractAccelerometer(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE RawCoreButtons[2],
_In_ BYTE RawAccelerometer[3]
)
{
	PWIIMOTE_STATE WiimoteState = &(DeviceContext->WiimoteContext.State);

	UNREFERENCED_PARAMETER(RawCoreButtons);

	/*/RawAccelerometer is always 10 to 2 Bit; RawCoreButtons have Bit 2 and for X also Bit 1
	WiimoteState->Accelerometer.X = (((RawAccelerometer[0] << 2) | (RawCoreButtons[0] & 0x60 >> 5)) & 0x03FF) - (0x80 << 2);
	WiimoteState->Accelerometer.Y = (((RawAccelerometer[1] << 2) | (RawCoreButtons[1] & 0x20 >> 4)) & 0x03FE) - (0x80 << 2);
	WiimoteState->Accelerometer.Z = (((RawAccelerometer[2] << 2) | (RawCoreButtons[1] & 0x40 >> 5)) & 0x03FE) - (0x80 << 2);
	*/
	// Just use the Byte
	WiimoteState->Accelerometer.X = RawAccelerometer[0];
	WiimoteState->Accelerometer.Y = RawAccelerometer[1];
	WiimoteState->Accelerometer.Z = RawAccelerometer[2];
}

VOID
ExtractNunchuck(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE RawInputData[6]
)
{
	BYTE DecodedInputData[1];
	XorData(RawInputData + 5, DecodedInputData, 1);

	//Buttons
	DeviceContext->WiimoteContext.NunchuckState.Buttons.Z = DecodedInputData[0] & 0x01;
	DeviceContext->WiimoteContext.NunchuckState.Buttons.C = DecodedInputData[0] & 0x02;

	//Analog Stick
	DeviceContext->WiimoteContext.NunchuckState.AnalogStick.X = RawInputData[0];
	DeviceContext->WiimoteContext.NunchuckState.AnalogStick.Y = RawInputData[1];

	//Accelerometer
	DeviceContext->WiimoteContext.NunchuckState.Accelerometer.X = RawInputData[2];
	DeviceContext->WiimoteContext.NunchuckState.Accelerometer.Y = RawInputData[3];
	DeviceContext->WiimoteContext.NunchuckState.Accelerometer.Z = RawInputData[4];
}

VOID
ExtractClassicControllerButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE DecodedInputData[2]
	)
{
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.A = DecodedInputData[1] & 0x10;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.B = DecodedInputData[1] & 0x40;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.Y = DecodedInputData[1] & 0x20;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.X = DecodedInputData[1] & 0x08;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.Home = DecodedInputData[0] & 0x08;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.Plus = DecodedInputData[0] & 0x04;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.Minus = DecodedInputData[0] & 0x10;

	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.DPad.Up = DecodedInputData[1] & 0x01;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.DPad.Right = DecodedInputData[0] & 0x80;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.DPad.Down = DecodedInputData[0] & 0x40;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.DPad.Left = DecodedInputData[1] & 0x02;

	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.L = DecodedInputData[0] & 0x20;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.ZL = DecodedInputData[1] & 0x80;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.R = DecodedInputData[0] & 0x02;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.ZR = DecodedInputData[1] & 0x04;
}

VOID
ExtractClassicController(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE RawInputData[6]
	)
{
	BYTE DecodedInputData[2];
	XorData(RawInputData + 4, DecodedInputData, 2);

	//Buttons
	ExtractClassicControllerButtons(DeviceContext, DecodedInputData);

	//Analog Sticks
	DeviceContext->WiimoteContext.ClassicControllerState.LeftAnalogStick.X = 0xFF & ((0x3F & RawInputData[0]) << 2);
	DeviceContext->WiimoteContext.ClassicControllerState.LeftAnalogStick.Y = 0xFF & ((0x3F & RawInputData[1]) << 2);

	DeviceContext->WiimoteContext.ClassicControllerState.RightAnalogStick.X = 0xFF & ((0xC0 & RawInputData[0]));
	DeviceContext->WiimoteContext.ClassicControllerState.RightAnalogStick.X |= 0xFF & ((0xC0 & RawInputData[1]) >> 2);
	DeviceContext->WiimoteContext.ClassicControllerState.RightAnalogStick.X |= 0xFF & ((0x80 & RawInputData[2]) >> 4);
	DeviceContext->WiimoteContext.ClassicControllerState.RightAnalogStick.Y = 0xFF & ((0x1F & RawInputData[2]) << 3);

	//Trigger
	DeviceContext->WiimoteContext.ClassicControllerState.LeftTrigger = 0xFF & ((0x60 & RawInputData[2]));
	DeviceContext->WiimoteContext.ClassicControllerState.LeftTrigger |= 0xFF & ((0xE0 & RawInputData[3]) >> 3);
	DeviceContext->WiimoteContext.ClassicControllerState.RightTrigger = 0xFF & ((0x1F & RawInputData[3]) << 2);

	//Not Supported Input
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.LH = FALSE;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.RH = FALSE;
}

VOID
ExtractWiiUProController(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE RawInputData[11]
)
{
	BYTE DecodedInputData[3];
	XorData(RawInputData + 8, DecodedInputData, 3);

	//Buttons
	ExtractClassicControllerButtons(DeviceContext, DecodedInputData);

	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.LH = DecodedInputData[2] & 0x02;
	DeviceContext->WiimoteContext.ClassicControllerState.Buttons.RH = DecodedInputData[2] & 0x01;

	//AnalogSticks
	DeviceContext->WiimoteContext.ClassicControllerState.LeftAnalogStick.X = 0xFF & ((RawInputData[0] >> 4) | (RawInputData[1] << 4));
	DeviceContext->WiimoteContext.ClassicControllerState.LeftAnalogStick.Y = 0xFF & ((RawInputData[4] >> 4) | (RawInputData[5] << 4));

	DeviceContext->WiimoteContext.ClassicControllerState.RightAnalogStick.X = 0xFF & ((RawInputData[2] >> 4) | (RawInputData[3] << 4));
	DeviceContext->WiimoteContext.ClassicControllerState.RightAnalogStick.Y = 0xFF & ((RawInputData[6] >> 4) | (RawInputData[7] << 4));

	//Not Supported Input
	DeviceContext->WiimoteContext.ClassicControllerState.LeftTrigger = DeviceContext->WiimoteContext.ClassicControllerState.Buttons.ZL ? 0x7F : 0x00;
	DeviceContext->WiimoteContext.ClassicControllerState.RightTrigger = DeviceContext->WiimoteContext.ClassicControllerState.Buttons.ZR ? 0x7F : 0x00;
}


VOID
ExtractIRCameraPoint(
_In_ PWIIMOTE_IR_POINT IRPointData1,
_In_ PWIIMOTE_IR_POINT IRPointData2,
_In_ BYTE InputData[5]
)
{
	USHORT X1 = InputData[0];
	USHORT Y1 = InputData[1];
	USHORT X2 = InputData[3];
	USHORT Y2 = InputData[4];

	X1 |= (0x30 & InputData[2]) << 4;
	Y1 |= (0xC0 & InputData[2]) << 2;
	X2 |= (0x03 & InputData[2]) << 8;
	Y2 |= (0x0C & InputData[2]) << 6;

	IRPointData1->X = X1;
	IRPointData1->Y = Y1;
	IRPointData2->X = X2;
	IRPointData2->Y = Y2;
}

VOID
ExtractIRCamera(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE RawInputData[10]
)
{
	BYTE BufferIndex = DeviceContext->WiimoteContext.IRState.PointsBufferPointer++;
	DeviceContext->WiimoteContext.IRState.PointsBufferPointer %= WIIMOTE_IR_POINTS_BUFFER_SIZE;

	ExtractIRCameraPoint(&(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][0]), &(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][1]), RawInputData);
	ExtractIRCameraPoint(&(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][2]), &(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][3]), RawInputData + 5);
}

NTSTATUS
ProcessExtensionData(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE * ReadBuffer,
	_In_ BYTE ReportID
	)
{
	UNREFERENCED_PARAMETER(ReportID);

	switch (DeviceContext->WiimoteContext.Extension)
	{
	case Nunchuck:
		ExtractNunchuck(DeviceContext, ReadBuffer);
		break;
	case ClassicController:
		ExtractClassicController(DeviceContext, ReadBuffer);
		break;
	case WiiUProController:
		ExtractWiiUProController(DeviceContext, ReadBuffer);
		break;
	default:
		break;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
ProcessStatusInformation(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE * ReadBuffer, 
	_In_ size_t ReadBufferSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ReadBufferSize);
	
	Trace("ProcessStatusInformation");

#ifndef MOUSE_IR
	BOOLEAN Extension = (ReadBuffer[3] & 0x02);
	Trace("Extension Flag: %d", (ReadBuffer[3] & 0x02)); 

	if (Extension)
	{
		Status = WriteSingeByteToRegister(DeviceContext, 0xA400F0, 0x55);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}

		Status = WriteSingeByteToRegister(DeviceContext, 0xA400FB, 0x00);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}

		Status = ReadFromRegister(DeviceContext, 0xA400FA, 0x06);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
	}
	else
	{
		DeviceContext->WiimoteContext.Extension = None;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x31;
	}
#endif

	//Process the Battery Level to set the LEDS
	Status = ProcessBatteryLevel(DeviceContext, ReadBuffer[6]);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Set Report Mode, if extension changed
	Status = SetReportMode(DeviceContext, DeviceContext->WiimoteContext.CurrentReportMode);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	

	return Status;
}

NTSTATUS
ProcessInputReport(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE * ReadBuffer, 
	_In_ size_t ReadBufferSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BYTE ReportID = ReadBuffer[0];

	UNREFERENCED_PARAMETER(ReadBufferSize);

	//Trace("ProcessInputReport");
	/*Trace("ProcessReport - ReportID: 0x%x - ReportSize: %d", ReportID, ReadBufferSize); */

	//PrintBytes(ReadBuffer, ReadBufferSize);

	//Every Report but 0x3d has Core Buttons
	if(ReportID != 0x3d)
	{
		ExtractCoreButtons(DeviceContext, ReadBuffer + 1);
	}

	switch (ReportID)
	{
	case 0x31: //Accelerometer
		ExtractAccelerometer(DeviceContext, ReadBuffer + 1, ReadBuffer + 3);
		break;
	case 0x32: //8 Byte Extension
	case 0x34: //19 Byte Extension
		ProcessExtensionData(DeviceContext, ReadBuffer + 3, ReportID);
		break;
	case 0x35:
		//Accelerometer & 16 Byte Extension
		ExtractAccelerometer(DeviceContext, ReadBuffer + 1, ReadBuffer + 3);
		ProcessExtensionData(DeviceContext, ReadBuffer + 6, ReportID);
	case 0x36:
		//10 Byte IR & 9 Byte Extension
		ExtractIRCamera(DeviceContext, ReadBuffer + 3);
		break;
	default:
		break;
	}

	Status = WiimoteStateUpdated(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
ProcessRegisterReadReport(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE * ReadBuffer,
_In_ size_t ReadBufferSize
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	//BYTE ReportID = ReadBuffer[0];

	UNREFERENCED_PARAMETER(ReadBufferSize);

	Trace("ProcessRegisterReadReport");
	/*Trace("ProcessReport - ReportID: 0x%x - ReportSize: %d", ReportID, ReadBufferSize); */
	//PrintBytes(ReadBuffer, ReadBufferSize);

	BYTE Error = 0x0F & (ReadBuffer[4]);
	Trace("Error Flag: %x", Error);

	ExtractCoreButtons(DeviceContext, ReadBuffer + 1);

	if (Error != 0x00)
	{
		return Status;
	}

	SHORT ExtensionType = 0;
	ExtensionType |= ReadBuffer[10] << 8;
	ExtensionType |= ReadBuffer[11];

	Trace("Extension Type: %#06x", ExtensionType);

	switch (ExtensionType)
	{
	case 0x0000: //Nunchuck
		Trace("Nunchuck Extension");
		DeviceContext->WiimoteContext.Extension = Nunchuck;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x35;
		break;
	case 0x0101: //CLassic Controler (Pro)
		Trace("Classic Controller (Pro) Extension");
		DeviceContext->WiimoteContext.Extension = ClassicController;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
		break;
	case 0x0120: //Wii U Pro Controller
		Trace("Wii U Pro Controller");
		DeviceContext->WiimoteContext.Extension = WiiUProController;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x34;
		break;
	default:
		Trace("No supported Extension!");
		return Status;
	}


	Status = SetReportMode(DeviceContext, DeviceContext->WiimoteContext.CurrentReportMode);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
ProcessAcknowledgementReport(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ BYTE * ReadBuffer,
_In_ size_t ReadBufferSize
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ReadBufferSize);

	ExtractCoreButtons(DeviceContext, ReadBuffer + 1);

	BYTE Report = ReadBuffer[3];
	BYTE Result = ReadBuffer[4];

	Trace("Error Flag: %x", Result);

	if (Result == 0x03)
	{
		return STATUS_SUCCESS;
	}

	switch (Report)
	{
	case 0x13:
		Status = SendSingleByteReport(DeviceContext, 0x1A, 0x06);
		break;
	case 0x1A:
		Status = EnableIR(DeviceContext);
		break;
	}

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
ProcessReport(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ PVOID ReadBuffer, 
	_In_ size_t ReadBufferSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BYTE ReportID = 0x00;
	
	if(ReadBufferSize >= 2)
	{
		ReportID = ((BYTE *)ReadBuffer)[1];
	}

	//PrintBytes(ReadBuffer, ReadBufferSize);

	switch(ReportID)
	{
	// Status Information
	case 0x20:	
		Status = ProcessStatusInformation(DeviceContext,(((BYTE *)ReadBuffer) + 1), (ReadBufferSize - 1));
		break;

	case 0x21:
		Status = ProcessRegisterReadReport(DeviceContext, (((BYTE *)ReadBuffer) + 1), (ReadBufferSize - 1));
		break;

	case 0x22:
		Status = ProcessAcknowledgementReport(DeviceContext, (((BYTE *)ReadBuffer) + 1), (ReadBufferSize - 1));
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x3D:
		Status = ProcessInputReport(DeviceContext,(((BYTE *)ReadBuffer) + 1), (ReadBufferSize - 1));
		break;
	default:
		Trace("Unkown Report Recieved");
		break;
	}

	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

VOID
StatusInformationTimerExpired (
    WDFTIMER  Timer
    )
{
	NTSTATUS Status;
	PDEVICE_CONTEXT DeviceContext = GetDeviceContext(WdfTimerGetParentObject(Timer));

	Trace("StatusInfro Timer!");

	Status = RequestStatusInformation(DeviceContext);
	Trace("Request Status Information Result: 0x%x", Status);
	if(!NT_SUCCESS(Status))
	{
		return;
	}
}
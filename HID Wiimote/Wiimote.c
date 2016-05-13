/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Wiimote.c

Abstract:
	Manages the Wiimote states and is the layer between HID and Bluetooth.
*/
#include "Wiimote.h"

#include "Device.h"
#include "Bluetooth.h"
#include "WiimoteSettings.h"

EVT_WDF_TIMER BatteryLevelLEDUpdateTimerExpired;

NTSTATUS 
WiimotePrepare(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext;
	WDF_TIMER_CONFIG TimerConfig;
	WDF_OBJECT_ATTRIBUTES TimerAttributes;

	WiimoteContext = &(DeviceContext->WiimoteContext);

	RtlSecureZeroMemory(&(WiimoteContext->State), sizeof(WiimoteContext->State));

	// Create Timer to request StatusInformation
	WDF_TIMER_CONFIG_INIT_PERIODIC(&TimerConfig, BatteryLevelLEDUpdateTimerExpired, WIIMOTE_STATUSINFORMATION_INTERVAL);
	WDF_OBJECT_ATTRIBUTES_INIT(&TimerAttributes);
	TimerAttributes.ParentObject = DeviceContext->Device;
	
	Status = WdfTimerCreate(&TimerConfig, &TimerAttributes, &(WiimoteContext->BatteryLevelLEDUpdateTimer));
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Load Settings from Registry
	WiimoteSettingsLoad(DeviceContext);

	return Status;
}

NTSTATUS
SetLEDs(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE LEDFlag
	)
{
	CONST size_t BufferSize = 3;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Data;

	// Save new LED State
	DeviceContext->WiimoteContext.LEDState = LEDFlag;

	// Get Resources
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x11;	//Player LED
	Data[2] = LEDFlag;

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, FALSE);
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
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x12;	//Set ReportMode
	Data[2] = 0x00;	//Only On Change
	Data[3] = ReportMode; //Mode

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, FALSE);
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
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = 0x15;	//Status Information Request
	Data[2] = 0x00;	//Rumble Off

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, FALSE);
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
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data);
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

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
WriteToRegister(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ INT32 Address,
_In_reads_(DataSize) PCUCHAR Data,
_In_ BYTE DataSize
)
{
	CONST size_t BufferSize = 23;
	NTSTATUS Status = STATUS_SUCCESS;
	WDFREQUEST Request;
	WDFMEMORY Memory;
	BYTE * Report;

	// Get Resources
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Report);
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

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
WriteSingeByteToRegister(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_ INT32 Address,
_In_ BYTE Data
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
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0xA2;	//HID Output Report
	Data[1] = ReportID;	// Enable IR 
	Data[2] = Byte; // Get acknowledgement 


	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, FALSE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS 
WiimoteStart(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	DeviceContext->WiimoteContext.Mode = Gamepad;
	DeviceContext->WiimoteContext.Extension = None;

	//Set LEDs
	Status = SetLEDs(DeviceContext, WIIMOTE_LEDS_ALL);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

#ifdef MOUSE_IR
	//Enable IR
	Status = SendSingleByteReport(DeviceContext, 0x13, 0x06);
	if(!NT_SUCCESS(Status))
	{
		TraceStatus("EnableIR Failed", Status);
		return Status;
	}
#else
	//Set Report Mode
	Status = SetReportMode(DeviceContext, 0x31);
	if(!NT_SUCCESS(Status))
	{
		TraceStatus("SetReportMode Failed", Status);
		return Status;
	}
#endif

	//Start Continious Reader
	Status = BluetoothStartContiniousReader(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		TraceStatus("StartContiniousReader Failed", Status);
		return Status;
	}

	//Start Timer
	WdfTimerStart(DeviceContext->WiimoteContext.BatteryLevelLEDUpdateTimer, WDF_REL_TIMEOUT_IN_SEC(1));

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
	Status = BluetoothCreateRequestAndBuffer(DeviceContext->Device, DeviceContext->IoTarget, BufferSize, &Request, &Memory, (PVOID *)&Data); 
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	// Fill Buffer	
	Data[0] = 0x13;	//HID Control with Suspend: 0x10 = HID_Control - 0x03 = SUSPEND

	Status = BluetoothTransferToDevice(DeviceContext, Request, Memory, TRUE);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	WdfObjectDelete(Request);
	return Status;

}

NTSTATUS
WiimoteStop(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	//Stop Timer
	WdfTimerStop(DeviceContext->WiimoteContext.BatteryLevelLEDUpdateTimer, TRUE);

	//Shut down Wiimote
	Status = SuspendWiimote(DeviceContext);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}


	return Status;
}

NTSTATUS 
WiimoteResetToNullState(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	DeviceContext->WiimoteContext.Extension = None;
	RtlSecureZeroMemory(&DeviceContext->WiimoteContext.State, sizeof(WIIMTOE_STATE));

	HIDWiimoteStateUpdated(DeviceContext);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return Status;
}

NTSTATUS
UpdateBatteryLEDs(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PWIIMOTE_DEVICE_CONTEXT WiimoteContext = &(DeviceContext->WiimoteContext);

	if (WiimoteContext->Extension == BalanceBoard)
	{
		return Status;
	}

	Status = SetLEDs(DeviceContext, ((0xF0 << (3 - (DeviceContext->WiimoteContext.BatteryLevel / 64))) & 0xF0));
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	if (DeviceContext->WiimoteContext.LEDState == WIIMOTE_LEDS_FOUR)
	{
		WdfTimerStop(WiimoteContext->BatteryLevelLEDUpdateTimer, FALSE);
	}

	return Status;
}

NTSTATUS
InitializeExtension(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

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

	return Status;
}

NTSTATUS
ProcessWiimoteBatteryLevel(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BYTE BatteryLevel
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	Trace("BatteryLevel: 0x%x", BatteryLevel);

	DeviceContext->WiimoteContext.BatteryLevel = BatteryLevel;

	Status = UpdateBatteryLEDs(DeviceContext); 
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	return STATUS_SUCCESS;
}

VOID
XorData(
_In_reads_(Length) BYTE * Source,
_Out_writes_all_(Length) BYTE * Dst,
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
_In_reads_bytes_(2) BYTE RawCoreButtons[]
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
_In_reads_bytes_(2) BYTE RawCoreButtons[],
_In_reads_bytes_(3) BYTE RawAccelerometer[]
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
_In_reads_bytes_(6) BYTE RawInputData[]
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
ExtractBalanceBoard(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(8) PUCHAR RawInputData
	)
{
	// Sensor Data is Big-Endian, Windows is always Little-Endian
	DeviceContext->WiimoteContext.BalanceBoardState.Sensor.TopRight = RtlUshortByteSwap(*((PUSHORT)(RawInputData + 0)));
	DeviceContext->WiimoteContext.BalanceBoardState.Sensor.BottomRight = RtlUshortByteSwap(*((PUSHORT)(RawInputData + 2)));
	DeviceContext->WiimoteContext.BalanceBoardState.Sensor.TopLeft = RtlUshortByteSwap(*((PUSHORT)(RawInputData + 4)));
	DeviceContext->WiimoteContext.BalanceBoardState.Sensor.BottomLeft = RtlUshortByteSwap(*((PUSHORT)(RawInputData + 6)));
}

VOID
ExtractClassicControllerButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(2) BYTE DecodedInputData[]
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
	_In_reads_bytes_(6) BYTE RawInputData[]
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
_In_reads_bytes_(11) BYTE RawInputData[]
)
{
	BYTE DecodedInputData[3];
	XorData(RawInputData + 8, DecodedInputData, 3);

	BYTE BatteryLevel = 0;

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
	DeviceContext->WiimoteContext.ClassicControllerState.LeftTrigger = DeviceContext->WiimoteContext.ClassicControllerState.Buttons.ZL ? 0xFF : 0x00;
	DeviceContext->WiimoteContext.ClassicControllerState.RightTrigger = DeviceContext->WiimoteContext.ClassicControllerState.Buttons.ZR ? 0xFF : 0x00;

	//Battery Level
	switch ((DecodedInputData[2] ^ 0xFF) & 0xF0)
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

	DeviceContext->WiimoteContext.BatteryLevel = BatteryLevel;
}

VOID
ExtractGuitar(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(6) BYTE RawInputData[]
	)
{
	BYTE DecodedInputData[2];
	XorData(RawInputData + 4, DecodedInputData, 2);

	// Buttons
	DeviceContext->WiimoteContext.GuitarState.Buttons.Green = DecodedInputData[1] & 0x10;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Red = DecodedInputData[1] & 0x40;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Yellow = DecodedInputData[1] & 0x08;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Blue = DecodedInputData[1] & 0x20;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Orange = DecodedInputData[1] & 0x80;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Plus = DecodedInputData[0] & 0x04;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Minus = DecodedInputData[0] & 0x10;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Up = DecodedInputData[1] & 0x01;
	DeviceContext->WiimoteContext.GuitarState.Buttons.Down = DecodedInputData[0] & 0x40;

	// Analog Sticks
	DeviceContext->WiimoteContext.GuitarState.AnalogStick.X = 0xFF & ((0x3F & RawInputData[0]) << 2);
	DeviceContext->WiimoteContext.GuitarState.AnalogStick.Y = 0xFF & ((0x3F & RawInputData[1]) << 2);

	// Analog Bars
	DeviceContext->WiimoteContext.GuitarState.WhammyBar = 0xFF & ((0x1F & RawInputData[0]) << 3);
	DeviceContext->WiimoteContext.GuitarState.TouchBar = 0xFF & ((0x1F & RawInputData[1]) << 3);
}

BOOLEAN
ExtractIRCameraPoint(
_In_ PWIIMOTE_IR_POINT IRPointData1,
_In_ PWIIMOTE_IR_POINT IRPointData2,
_In_reads_bytes_(5) BYTE InputData[]
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

	// Max for Y is 767, so 0x3FF Value means no Data
	return ((Y1 != 0x3FF) || (Y2 != 0x3FF));
}

BOOLEAN
ExtractIRCamera(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_reads_bytes_(10) BYTE RawInputData[]
)
{
	BOOLEAN ValidPointData = FALSE;
	BYTE BufferIndex = DeviceContext->WiimoteContext.IRState.PointsBufferPointer++;
	DeviceContext->WiimoteContext.IRState.PointsBufferPointer %= WIIMOTE_IR_POINTS_BUFFER_SIZE;

	ValidPointData |= ExtractIRCameraPoint(&(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][0]), &(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][1]), RawInputData);
	ValidPointData |= ExtractIRCameraPoint(&(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][2]), &(DeviceContext->WiimoteContext.IRState.Points[BufferIndex][3]), RawInputData + 5);

	return ValidPointData;
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
	case BalanceBoard:
		ExtractBalanceBoard(DeviceContext, ReadBuffer);
		break;
	case ClassicController:
		ExtractClassicController(DeviceContext, ReadBuffer);
		break;
	case WiiUProController:
		ExtractWiiUProController(DeviceContext, ReadBuffer);
		break;
	case Guitar:
		ExtractGuitar(DeviceContext, ReadBuffer);
		break;
	default:
		break;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
ProcessStatusInformation(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(ReadBufferSize) BYTE * ReadBuffer,
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
		if (DeviceContext->WiimoteContext.Extension == None)
		{
			Status = InitializeExtension(DeviceContext);
			if (!NT_SUCCESS(Status))
			{
				return Status;
			}
		}
	}
	else
	{
		DeviceContext->WiimoteContext.Extension = None;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x31;
	}
#endif

	//Process the Battery Level to set the LEDS; Wii U Pro Controller reports its Battery Level in its Input Report
	switch (DeviceContext->WiimoteContext.Extension)
	{
	case WiiUProController:
		break;
	default:
		Status = ProcessWiimoteBatteryLevel(DeviceContext, ReadBuffer[6]);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
	}

	//Set Report Mode, if extension changed
	Status = SetReportMode(DeviceContext, DeviceContext->WiimoteContext.CurrentReportMode);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}
	
	// Notify Device Interface about Status Update
	DeviceInterfaceWiimoteStateUpdated(DeviceContext);

	return Status;
}

NTSTATUS
ProcessInputReport(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(ReadBufferSize) BYTE * ReadBuffer,
	_In_ size_t ReadBufferSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BYTE ReportID = ReadBuffer[0];
	BOOLEAN UpdateHIDState = TRUE;

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
		UpdateHIDState = ExtractIRCamera(DeviceContext, ReadBuffer + 3);
		break;
	default:
		break;
	}

	if (UpdateHIDState)
	{
		HIDWiimoteStateUpdated(DeviceContext);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
	}

	return Status;
}

NTSTATUS
ProcessExtensionRegister(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(ReadBufferSize) BYTE * ReadBuffer,
	_In_ size_t ReadBufferSize,
	_In_ BYTE ErrorFlag,
	_In_ USHORT ReadAddress)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ReadBufferSize);
	UNREFERENCED_PARAMETER(ErrorFlag);
	UNREFERENCED_PARAMETER(ReadAddress);

	USHORT ExtensionType = 0;
	ExtensionType |= ReadBuffer[4] << 8;
	ExtensionType |= ReadBuffer[5];

	Trace("Extension Type: %#06x", ExtensionType);

	switch (ExtensionType)
	{
	case 0x0000: // Nunchuck
		Trace("Nunchuck Extension");
		DeviceContext->WiimoteContext.Extension = Nunchuck;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x35;
		break;
	case 0x0402: // Balance Board
		Trace("Balance Board");
		DeviceContext->WiimoteContext.Extension = BalanceBoard;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
		SetLEDs(DeviceContext, WIIMOTE_LEDS_ALL);
		// Get Calibration Data
		ReadFromRegister(DeviceContext, 0xA40024, 24);
		break;
	case 0x0101: // Classic Controler (Pro)
		Trace("Classic Controller (Pro) Extension");
		DeviceContext->WiimoteContext.Extension = ClassicController;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
		break;
	case 0x0120: // Wii U Pro Controller
		Trace("Wii U Pro Controller");
		DeviceContext->WiimoteContext.Extension = WiiUProController;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x34;
		break;
	case 0x0103: // Guitar Hero Guitar
		Trace("Guitar Hero Guitar");
		DeviceContext->WiimoteContext.Extension = Guitar;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
		break;
	case 0xFFFF: // Error
		Trace("Error");
		DeviceContext->WiimoteContext.Extension = None;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x31;
		Status = InitializeExtension(DeviceContext);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
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

	// Signal Device Interface to report the new Extension Type
	DeviceInterfaceWiimoteStateUpdated(DeviceContext);

	return Status;
}

NTSTATUS
ProcessBalanceBoardCalibrationRegister(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(ReadBufferSize) PUCHAR ReadBuffer,
	_In_ size_t ReadBufferSize,
	_In_ BYTE ErrorFlag,
	_In_ USHORT ReadAddress)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ErrorFlag);

	PUSHORT Data = (PUSHORT)ReadBuffer;
	size_t CalibrationIndexStart = (ReadAddress == 0x0024) ? 0 : 2;
	size_t CalibrationIndexEnd = CalibrationIndexStart + (ReadBufferSize / 8);

	for (size_t CalibrationDataIndex = CalibrationIndexStart; CalibrationDataIndex < CalibrationIndexEnd; ++CalibrationDataIndex)
	{
		for (size_t SensorIndex = 0; SensorIndex < 4; ++SensorIndex)
		{
			DeviceContext->WiimoteContext.BalanceBoardState.CalibrationRaw[SensorIndex][CalibrationDataIndex] = RtlUshortByteSwap(Data[4 * (CalibrationDataIndex - CalibrationIndexStart) + SensorIndex]);
		}
	}

	return Status;
}

NTSTATUS
ProcessRegisterReadReport(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(ReadBufferSize) BYTE * ReadBuffer,
	_In_ size_t ReadBufferSize
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ReadBufferSize);

	Trace("ProcessRegisterReadReport");

	ExtractCoreButtons(DeviceContext, ReadBuffer + 1);

	BYTE Error = 0x0F & (ReadBuffer[3]);
	Trace("Error Flag: %#04x", Error);

	BYTE Size = ((0xF0 & (ReadBuffer[3])) >> 4) + 1;
	Trace("Size: %#04x", Size);
	USHORT Address = RtlUshortByteSwap(*((USHORT *)(ReadBuffer + 4)));
	Trace("Address: %#06x", Address);

	switch(Address)
	{
	case 0x00FA: // Extension Type
		Status = ProcessExtensionRegister(DeviceContext, ReadBuffer + 6, Size, Error, Address);
		break;
	case 0x0024: // BalanceBoard Calibration
	case 0x0034:
		Status = ProcessBalanceBoardCalibrationRegister(DeviceContext, ReadBuffer + 6, Size, Error, Address);
		break;
	}

	return Status;
}

NTSTATUS
ProcessAcknowledgementReport(
_In_ PDEVICE_CONTEXT DeviceContext,
_In_reads_bytes_(ReadBufferSize) BYTE * ReadBuffer,
_In_ size_t ReadBufferSize
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(ReadBufferSize);

	Trace("ProcessAcknowledgementReport");

	ExtractCoreButtons(DeviceContext, ReadBuffer + 1);

	BYTE Report = ReadBuffer[3];
	BYTE Result = ReadBuffer[4];

	Trace("Report: %#04x", Report);
	Trace("Error Flag: %#04x", Result);

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
WiimoteProcessReport(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_reads_bytes_(ReadBufferSize) PVOID ReadBuffer,
	_In_ SIZE_T ReadBufferSize
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
BatteryLevelLEDUpdateTimerExpired(
    WDFTIMER  Timer
    )
{
	NTSTATUS Status;
	PDEVICE_CONTEXT DeviceContext = GetDeviceContext(WdfTimerGetParentObject(Timer));

	Trace("BatteryLevelLEDUpdate Timer!");

	if (DeviceContext->WiimoteContext.Extension == WiiUProController)
	{
		Status = UpdateBatteryLEDs(DeviceContext);
		if (!NT_SUCCESS(Status))
		{
			return;
		}
	}
	else
	{
		Status = RequestStatusInformation(DeviceContext);
		TraceStatus("Request Status Information Result", Status);
		if (!NT_SUCCESS(Status))
		{
			return;
		}
	}
}

/*

Copyright (C) 2017 Julian Löhr
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
	WDF_TIMER_CONFIG TimerConfig;
	WDF_OBJECT_ATTRIBUTES TimerAttributes;

	// Reset Device
	WiimoteReset(DeviceContext);

	// Load Settings from Registry
	WiimoteSettingsLoad(DeviceContext);

	// Create Timer to request StatusInformation
	WDF_TIMER_CONFIG_INIT_PERIODIC(&TimerConfig, BatteryLevelLEDUpdateTimerExpired, WIIMOTE_STATUSINFORMATION_INTERVAL);
	WDF_OBJECT_ATTRIBUTES_INIT(&TimerAttributes);
	TimerAttributes.ParentObject = DeviceContext->Device;
	
	Status = WdfTimerCreate(&TimerConfig, &TimerAttributes, &(DeviceContext->WiimoteContext.BatteryLevelLEDUpdateTimer));
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

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

VOID 
WiimoteReset(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	DeviceContext->WiimoteContext.Extension = None;
	WiimoteStateResetToNullState(DeviceContext);
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

	USHORT ExtensionType = ReadBuffer[4] << 8 | ReadBuffer[5];
	USHORT SubType = ReadBuffer[0] << 8 | ReadBuffer[1];

	Trace("Extension Type: %#010x", ExtensionType);
	Trace("Sub Type: %#010x", SubType);

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
	case 0x0101:
		switch (SubType)
		{
		default:
		case 0x0000: // Classic Controler
			Trace("Classic Controller Extension");
			DeviceContext->WiimoteContext.Extension = ClassicController;
			DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
			break;
		case 0x0100: // Classic Controler Pro
			Trace("Classic Controller Pro Extension");
			DeviceContext->WiimoteContext.Extension = ClassicControllerPro;
			DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
			break;
		}
		break;
	case 0x0120: // Wii U Pro Controller
		Trace("Wii U Pro Controller");
		DeviceContext->WiimoteContext.Extension = WiiUProController;
		DeviceContext->WiimoteContext.CurrentReportMode = 0x34;
		break;
	case 0x0103:
		switch (SubType)
		{
		default:
		case 0x0000:  // 0x0000: Guitar Hero Guitar
			Trace("Guitar Hero Guitar");
			DeviceContext->WiimoteContext.Extension = Guitar;
			DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
			break;
		case 0x0100:  // 0x0100: Guitar Hero Drum Kit
			Trace("Guitar Hero Drums");
			DeviceContext->WiimoteContext.Extension = Drums;
			DeviceContext->WiimoteContext.CurrentReportMode = 0x32;
			break;
			// 0x0300: DJ Hero Turntables
		}
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
		Trace("Unsupported Extension!");
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
			DeviceContext->WiimoteContext.State.BalanceBoardState.Calibration[SensorIndex][CalibrationDataIndex] = RtlUshortByteSwap(Data[4 * (CalibrationDataIndex - CalibrationIndexStart) + SensorIndex]);
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

	WiimoteStateUpdate(DeviceContext, ReadBuffer, ReadBufferSize);

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

	WiimoteStateUpdate(DeviceContext, ReadBuffer, ReadBufferSize);

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
		Status = WiimoteStateUpdate(DeviceContext,(((PUCHAR)ReadBuffer) + 1), (ReadBufferSize - 1));
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

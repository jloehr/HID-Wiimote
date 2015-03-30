/*

Copyright (C) 2013 Julian Löhr
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
StartWiimote(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	//Set LEDs
	Status = SetLEDs(DeviceContext, WIIMOTE_LEDS_FOUR);
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	//Set Report Mode
	Status = SetReportMode(DeviceContext, 0x31);
	if(!NT_SUCCESS(Status))
	{
		Trace("SetReportMode Failed: 0x%x", Status);
		return Status;
	}

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
	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	if(BatteryFlag == WIIMOTE_LEDS_ONE)
	{
		WdfTimerStop(WiimoteContext->StatusInformationTimer, FALSE);
	}

	return Status;
}

VOID
ExtractCoreButtons (
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
	
	Trace("Extension Flag: %d", (ReadBuffer[3] & 0x02)); 

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

	//Every Report but 0x3d has Core Buttons
	if(ReportID != 0x3d)
	{
		ExtractCoreButtons(DeviceContext, ReadBuffer + 1);
	}

	if(ReportID == 0x31)
	{
		ExtractAccelerometer(DeviceContext, ReadBuffer + 1, ReadBuffer + 3);
	}

	//Trace("Acceleometer: X => %d; Y => %d; Z => %d", (DeviceContext->WiimoteContext.State.Accelerometer.X - 0x80), (DeviceContext->WiimoteContext.State.Accelerometer.Y - 0x80), (DeviceContext->WiimoteContext.State.Accelerometer.Z - 0x80));
	
	Status = WiimoteStateUpdated(DeviceContext);
	if(!NT_SUCCESS(Status))
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
	//CHAR * Message;
	//CHAR * WritePointer;
	//size_t i;

	if(ReadBufferSize >= 2)
	{
		ReportID = ((BYTE *)ReadBuffer)[1];
	}

	/*Trace("ProcessReport - ReportID: 0x%x - ReportSize: %d", ReportID, ReadBufferSize); 

	Message = (CHAR * )ExAllocatePool(NonPagedPool, (10*ReadBufferSize));
	WritePointer = Message;

	for(i = 0; i < ReadBufferSize; ++i)
	{
		WritePointer += sprintf(WritePointer, "%#02x ", ((BYTE *)ReadBuffer)[i]);
	}
	(*WritePointer) = 0;
	Trace("ReadBuffer: %s", Message);
	*/

	switch(ReportID)
	{
	// Status Information
	case 0x20:	
		Status = ProcessStatusInformation(DeviceContext,(((BYTE *)ReadBuffer) + 1), (ReadBufferSize - 1));
		break;

	case 0x30:
	case 0x31:
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
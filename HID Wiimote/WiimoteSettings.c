/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteSettings.c

Abstract:
	Loads and saves the Wiimote Settings from and to the Registry. 

*/
#include "WiimoteToHIDParser.h"

#include "Device.h"

DECLARE_CONST_UNICODE_STRING(DriverModeValueName, L"DriverMode");
DECLARE_CONST_UNICODE_STRING(XAxisEnabledValueName, L"XAxisEnbaled");
DECLARE_CONST_UNICODE_STRING(YAxisEnabledValueName, L"YAxisEnbaled");
DECLARE_CONST_UNICODE_STRING(MouseButtonsSwitchedValueName, L"MouseButtonsSwitched");
DECLARE_CONST_UNICODE_STRING(TriggerAndShoulderSwitchedValueName, L"TriggerAndShoulderSwitched");
DECLARE_CONST_UNICODE_STRING(TriggerSplitValueName, L"TriggerSplit");

NTSTATUS
OpenRegistryKey(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ ACCESS_MASK Access,
	_Out_ WDFKEY * Key,
	_Out_opt_ BOOLEAN * NewlyCreated
	);

VOID
LoadBooleanValue(
	_In_ WDFKEY Key,
	_In_ PCUNICODE_STRING ValueName,
	_Inout_ PBOOLEAN Value
);

VOID
SaveULONGValue(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ PCUNICODE_STRING ValueName,
	_In_ ULONG Value
	);

VOID
WiimoteSettingsLoad(
	_In_ PDEVICE_CONTEXT DeviceContext
	)
{
	NTSTATUS Status;
	WDFKEY Key;
	BOOLEAN NewlyCreated;
	ULONG Value;

	Status = OpenRegistryKey(DeviceContext, KEY_READ, &Key, &NewlyCreated);
	if (!NT_SUCCESS(Status))
	{
		Trace("Failed to open Reg Key to load settings.");
		return;
	}

	if (NewlyCreated)
	{
		Trace("Subkey was created, so nothing to read from.");
		WdfRegistryClose(Key);
		return;
	}

	// Load each Setting
	Status = WdfRegistryQueryULong(Key, &DriverModeValueName, &Value);
	if (NT_SUCCESS(Status))
	{
		DeviceContext->WiimoteContext.Mode = (WIIMOTE_DRIVER_MODE)Value;
	}

	LoadBooleanValue(Key, &XAxisEnabledValueName, &DeviceContext->WiimoteContext.Settings.XAxisEnabled);
	LoadBooleanValue(Key, &YAxisEnabledValueName, &DeviceContext->WiimoteContext.Settings.YAxisEnabled);
	LoadBooleanValue(Key, &MouseButtonsSwitchedValueName, &DeviceContext->WiimoteContext.Settings.MouseButtonsSwitched);
	LoadBooleanValue(Key, &TriggerAndShoulderSwitchedValueName, &DeviceContext->WiimoteContext.Settings.TriggerAndShoulderSwitched);
	LoadBooleanValue(Key, &TriggerSplitValueName, &DeviceContext->WiimoteContext.Settings.TriggerSplit);

	// Close Key
	WdfRegistryClose(Key);
}

VOID
WiimoteSettingsSetDriverMode(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ WIIMOTE_DRIVER_MODE DriverMode
	)
{
	DeviceContext->WiimoteContext.Mode = DriverMode;

	SaveULONGValue(DeviceContext, &DriverModeValueName, (ULONG)DriverMode);
}

VOID
WiimoteSettingsSetXAxisEnabled(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.XAxisEnabled = Enabled;

	SaveULONGValue(DeviceContext, &XAxisEnabledValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetYAxisEnabled(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.YAxisEnabled = Enabled;

	SaveULONGValue(DeviceContext, &YAxisEnabledValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetMouseButtonsSwitched(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.MouseButtonsSwitched = Enabled;

	SaveULONGValue(DeviceContext, &MouseButtonsSwitchedValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetTriggerAndShoulderSwitched(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.TriggerAndShoulderSwitched = Enabled;

	SaveULONGValue(DeviceContext, &TriggerAndShoulderSwitchedValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetTriggerSplit(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.TriggerSplit = Enabled;

	SaveULONGValue(DeviceContext, &TriggerSplitValueName, (ULONG)Enabled);
}

NTSTATUS
OpenRegistryKey(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ ACCESS_MASK Access,
	_Out_ WDFKEY * Key,
	_Out_opt_ BOOLEAN * NewlyCreated
	)
{
	NTSTATUS Status;
	WDFKEY DeviceKey;
	ULONG CreateDisposition;

	DECLARE_CONST_UNICODE_STRING(HIDWiimoteSubkey, L"HIDWiimote");

	Status = WdfDeviceOpenRegistryKey(DeviceContext->Device, PLUGPLAY_REGKEY_DEVICE, Access, WDF_NO_OBJECT_ATTRIBUTES, &DeviceKey);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error opening Device Key", Status);
		return Status;
	}

	Status = WdfRegistryCreateKey(DeviceKey, &HIDWiimoteSubkey, Access, REG_OPTION_NON_VOLATILE, &CreateDisposition, WDF_NO_OBJECT_ATTRIBUTES, Key);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error opening HID Wiimote Subkey", Status);
		WdfRegistryClose(DeviceKey);
		return Status;
	}

	if (NewlyCreated != NULL)
	{
		(*NewlyCreated) = (CreateDisposition == REG_CREATED_NEW_KEY) ? TRUE : FALSE;
	}

	WdfRegistryClose(DeviceKey);

	return STATUS_SUCCESS;
}

VOID
LoadBooleanValue(
	_In_ WDFKEY Key,
	_In_ PCUNICODE_STRING ValueName,
	_Out_ PBOOLEAN Value
)
{
	NTSTATUS Status;
	ULONG ValueBuffer;

	Status = WdfRegistryQueryULong(Key, ValueName, &ValueBuffer);
	if (NT_SUCCESS(Status))
	{
		(*Value) = (BOOLEAN)ValueBuffer;
	}
}

VOID
SaveULONGValue(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ PCUNICODE_STRING ValueName,
	_In_ ULONG Value
)
{
	NTSTATUS Status;
	WDFKEY Key;

	Status = OpenRegistryKey(DeviceContext, KEY_WRITE, &Key, NULL);
	if (!NT_SUCCESS(Status)) 
	{
		TraceStatus("Error opening Subkey for writing", Status);
		return;
	}

	Status = WdfRegistryAssignULong(Key, ValueName, Value);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error writing Value", Status);
		WdfRegistryClose(Key);
		return;
	}

	WdfRegistryClose(Key);
}
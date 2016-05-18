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
	_Out_ WDFKEY * DeviceKey,
	_Out_opt_ BOOLEAN * NewlyCreated
	);

VOID
LoadWiimoteDriverModeValue(
	_In_ WDFKEY Key,
	_In_ PCUNICODE_STRING ValueName,
	_In_ WIIMOTE_DRIVER_MODE DefaultValue,
	_Out_ PWIIMOTE_DRIVER_MODE Value
);

VOID
LoadBooleanValue(
	_In_ WDFKEY Key,
	_In_ PCUNICODE_STRING ValueName,
	_In_ BOOLEAN DefaultValue,
	_Out_ PBOOLEAN Value
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

	Status = OpenRegistryKey(DeviceContext, KEY_READ, &Key, NULL);
	if (!NT_SUCCESS(Status))
	{
		Trace("Failed to open Reg Key to load settings.");
		return;
	}

	// Load each Setting
	LoadWiimoteDriverModeValue(Key, &DriverModeValueName, Gamepad, &DeviceContext->WiimoteContext.Mode);
	LoadBooleanValue(Key, &XAxisEnabledValueName, FALSE, &DeviceContext->WiimoteContext.Settings.XAxisEnabled);
	LoadBooleanValue(Key, &YAxisEnabledValueName, FALSE, &DeviceContext->WiimoteContext.Settings.YAxisEnabled);
	LoadBooleanValue(Key, &MouseButtonsSwitchedValueName, FALSE, &DeviceContext->WiimoteContext.Settings.MouseButtonsSwitched);
	LoadBooleanValue(Key, &TriggerAndShoulderSwitchedValueName, FALSE, &DeviceContext->WiimoteContext.Settings.TriggerAndShoulderSwitched);
	LoadBooleanValue(Key, &TriggerSplitValueName, TRUE, &DeviceContext->WiimoteContext.Settings.TriggerSplit);

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

/*
 Currently the hardware/device key as well as the driver software key is not persistent accross disconnecting Wiimotes.
 Therefore we need to use the driver/service parameters key, that seems to be persistent, even when the driver is uninstalled.
 An "DeviceSettings" subkey is used so the parameters key remains clear.
 For each device the Bluetooth address is used as unique identifier and device key.
*/
NTSTATUS
OpenRegistryKey(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ ACCESS_MASK Access,
	_Out_ WDFKEY * DeviceKey,
	_Out_opt_ BOOLEAN * NewlyCreated
	)
{
	NTSTATUS Status;
	WDFKEY ServiceKey;
	WDFKEY DeviceSettingsSubkey;
	ULONG CreateDisposition;

	DECLARE_CONST_UNICODE_STRING(DeviceSettingsSubkeyName, L"DeviceSettings");

	// Open the driver/service parameters key "../Services/HIDWiimote/Parameters"
	Status = WdfDriverOpenParametersRegistryKey(WdfGetDriver(), Access, WDF_NO_OBJECT_ATTRIBUTES, &ServiceKey);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error opening Service Key", Status);
		return Status;
	}

	// Open/create a subkey for all devices "../Services/HIDWiimote/Parameters/DeviceSettings"
	Status = WdfRegistryCreateKey(ServiceKey, &DeviceSettingsSubkeyName, Access, REG_OPTION_NON_VOLATILE, &CreateDisposition, WDF_NO_OBJECT_ATTRIBUTES, &DeviceSettingsSubkey);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error opening Device Settings Subkey", Status);
		WdfRegistryClose(ServiceKey);
		return Status;
	}

	// Open/create the device "../Services/HIDWiimote/Parameters/DeviceSettings/XXXXXXXXXXX"
	Status = WdfRegistryCreateKey(DeviceSettingsSubkey, &DeviceContext->BluetoothContext.DeviceAddressString, Access, REG_OPTION_NON_VOLATILE, &CreateDisposition, WDF_NO_OBJECT_ATTRIBUTES, DeviceKey);
	if (!NT_SUCCESS(Status))
	{
		TraceStatus("Error opening Device Subkey", Status);
		WdfRegistryClose(ServiceKey);
		WdfRegistryClose(DeviceSettingsSubkey);
		return Status;
	}

	if (NewlyCreated != NULL)
	{
		(*NewlyCreated) = (CreateDisposition == REG_CREATED_NEW_KEY) ? TRUE : FALSE;
	}

	WdfRegistryClose(ServiceKey);
	WdfRegistryClose(DeviceSettingsSubkey);

	return STATUS_SUCCESS;
}

VOID
LoadWiimoteDriverModeValue(
	_In_ WDFKEY Key,
	_In_ PCUNICODE_STRING ValueName,
	_In_ WIIMOTE_DRIVER_MODE DefaultValue,
	_Out_ PWIIMOTE_DRIVER_MODE Value
)
{
	NTSTATUS Status;
	ULONG ValueBuffer;

	Status = WdfRegistryQueryULong(Key, ValueName, &ValueBuffer);
	(*Value) = NT_SUCCESS(Status) ? (WIIMOTE_DRIVER_MODE)ValueBuffer : DefaultValue;
}

VOID
LoadBooleanValue(
	_In_ WDFKEY Key,
	_In_ PCUNICODE_STRING ValueName,
	_In_ BOOLEAN DefaultValue,
	_Out_ PBOOLEAN Value
)
{
	NTSTATUS Status;
	ULONG ValueBuffer;

	Status = WdfRegistryQueryULong(Key, ValueName, &ValueBuffer);
	(*Value) = NT_SUCCESS(Status) ? (BOOLEAN)ValueBuffer : DefaultValue;
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
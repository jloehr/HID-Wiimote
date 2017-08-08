/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	WiimoteSettings.c

Abstract:
	Loads and saves the Wiimote Settings from and to the Registry. 

*/
#include "WiimoteToHIDParser.h"

#include "Device.h"

DECLARE_CONST_UNICODE_STRING(DriverModeValueName, L"DriverMode");
DECLARE_CONST_UNICODE_STRING(EnableWiimoteXAxisAccelerometerValueName, L"EnableWiimoteXAxisAccelerometer");
DECLARE_CONST_UNICODE_STRING(EnableWiimoteYAxisAccelerometerValueName, L"EnableWiimoteYAxisAccelerometer");
DECLARE_CONST_UNICODE_STRING(SwitchMouseButtonsValueName, L"SwitchMouseButtons");
DECLARE_CONST_UNICODE_STRING(SwitchTriggerAndShoulderValueName, L"SwitchTriggerAndShoulder");
DECLARE_CONST_UNICODE_STRING(SplitTriggerAxisValueName, L"SplitTriggerAxis");
DECLARE_CONST_UNICODE_STRING(MapTriggerAsAxisValueName, L"MapTriggerAsAxis");
DECLARE_CONST_UNICODE_STRING(MapTriggerAsButtonsValueName, L"MapTriggerAsButtons");

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
	DeviceContext->WiimoteContext.Settings.Mode = Gamepad;
	//LoadWiimoteDriverModeValue(Key, &DriverModeValueName, Gamepad, &DeviceContext->WiimoteContext.Settings.Mode);
	LoadBooleanValue(Key, &EnableWiimoteXAxisAccelerometerValueName, FALSE, &DeviceContext->WiimoteContext.Settings.EnableWiimoteXAxisAccelerometer);
	LoadBooleanValue(Key, &EnableWiimoteYAxisAccelerometerValueName, FALSE, &DeviceContext->WiimoteContext.Settings.EnableWiimoteYAxisAcceleromenter);
	LoadBooleanValue(Key, &SwitchMouseButtonsValueName, FALSE, &DeviceContext->WiimoteContext.Settings.SwitchMouseButtons);
	LoadBooleanValue(Key, &SwitchTriggerAndShoulderValueName, FALSE, &DeviceContext->WiimoteContext.Settings.SwitchTriggerAndShoulder);
	LoadBooleanValue(Key, &SplitTriggerAxisValueName, TRUE, &DeviceContext->WiimoteContext.Settings.SplitTriggerAxis);
	LoadBooleanValue(Key, &MapTriggerAsAxisValueName, FALSE, &DeviceContext->WiimoteContext.Settings.MapTriggerAsAxis);
	LoadBooleanValue(Key, &MapTriggerAsButtonsValueName, TRUE, &DeviceContext->WiimoteContext.Settings.MapTriggerAsButtons);

	// Close Key
	WdfRegistryClose(Key);
}

VOID
WiimoteSettingsSetDriverMode(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ WIIMOTE_DRIVER_MODE DriverMode
	)
{
	//DeviceContext->WiimoteContext.Settings.Mode = DriverMode;

	SaveULONGValue(DeviceContext, &DriverModeValueName, (ULONG)DriverMode);

	// Check for IR Activation/Deactivation

}

VOID
WiimoteSettingsSetEnableWiimoteXAxisAccelerometer(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.EnableWiimoteXAxisAccelerometer = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &EnableWiimoteXAxisAccelerometerValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetEnableWiimoteYAxisAccelerometer(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.EnableWiimoteYAxisAcceleromenter = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &EnableWiimoteYAxisAccelerometerValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetSwitchMouseButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.SwitchMouseButtons = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &SwitchMouseButtonsValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetSwitchTriggerAndShoulder(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.SwitchTriggerAndShoulder = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &SwitchTriggerAndShoulderValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetSplitTriggerAxis(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
	)
{
	DeviceContext->WiimoteContext.Settings.SplitTriggerAxis = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &SplitTriggerAxisValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetMapTriggerAsAxis(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
)
{
	DeviceContext->WiimoteContext.Settings.MapTriggerAsAxis = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &MapTriggerAsAxisValueName, (ULONG)Enabled);
}

VOID
WiimoteSettingsSetMapTriggerAsButtons(
	_In_ PDEVICE_CONTEXT DeviceContext,
	_In_ BOOLEAN Enabled
)
{
	DeviceContext->WiimoteContext.Settings.MapTriggerAsButtons = Enabled;
	HIDWiimoteStateUpdated(DeviceContext);

	SaveULONGValue(DeviceContext, &MapTriggerAsButtonsValueName, (ULONG)Enabled);
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
/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterfaceEnumerator.cpp

Abstract:
	Implementation of the WiimoteDeviceInterfaceEnumerator Class

*/
#include "stdafx.h"
#include "WiimoteDeviceInterfaceEnumerator.h"

#pragma comment(lib, "setupapi.lib")

#include "../HID Wiimote/DeviceInterfacePublic.h"

#include "Log.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		bool WiimoteDeviceInterfaceEnumerator::Enumerate()
		{
			HDEVINFO DeviceInfoSet; 

			EnumeratedDevices.clear();

			DeviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_HIDWIIMOTE, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
			if (!DeviceInfoSet)
			{
				return false;
			}

			DWORD DeviceIndex = 0;
			while (EnumerateDeviceInterface(DeviceInfoSet, DeviceIndex))
			{
				DeviceIndex++;
			}

			if (!SetupDiDestroyDeviceInfoList(DeviceInfoSet))
			{
				Log::Write("Error destroying Device Info Set");
			}

			return true;
		}
		
		bool WiimoteDeviceInterfaceEnumerator::EnumerateDeviceInterface(HDEVINFO DeviceInfoSet, DWORD DeviceIndex)
		{
			SP_DEVICE_INTERFACE_DATA DeviceInterfaceData = {};
			DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

			if (!SetupDiEnumDeviceInterfaces(DeviceInfoSet, NULL, &GUID_DEVINTERFACE_HIDWIIMOTE, DeviceIndex, &DeviceInterfaceData))
			{
				return false;
			}

			BOOL Result;
			DWORD RequiredSize;

			SP_DEVINFO_DATA DeviceInfoData = {};
			DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

			SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, NULL, 0, &RequiredSize, NULL);
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				Log::Write("Error getting DeviceInterfaceDetailData size");
				return true;
			}
			
			PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
			DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			Result = SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, NULL, &DeviceInfoData);
			if (!Result)
			{
				Log::Write("Error getting Device Interface Details");
				return true;
			}

			EnumeratedDevices.push_back(DeviceInterfaceDetailData->DevicePath);

			return true;
		}
	}
}
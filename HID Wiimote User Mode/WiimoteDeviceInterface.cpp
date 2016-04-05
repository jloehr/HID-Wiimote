/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterface.cpp

Abstract:
	Implementation of the WiimoteDeviceInterface Class

*/
#include "stdafx.h"
#include "WiimoteDeviceInterface.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		WiimoteDeviceInterface::WiimoteDeviceInterface(System::String ^ DeviceInterfacePath)
			: DeviceInterfaceHandle(INVALID_HANDLE_VALUE)
		{
			this->DeviceInterfacePath = DeviceInterfacePath;
		}

		void WiimoteDeviceInterface::Initialize()
		{
		}

		bool WiimoteDeviceInterface::OpenDevice()
		{
			System::IntPtr PointerToNativeString = System::Runtime::InteropServices::Marshal::StringToHGlobalUni(DeviceInterfacePath);

			HANDLE OpenDevice = CreateFile(static_cast<LPCWSTR>(PointerToNativeString.ToPointer()), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
			System::Runtime::InteropServices::Marshal::FreeHGlobal(PointerToNativeString);

			if (OpenDevice == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			DeviceInterfaceHandle = OpenDevice;
			return true;
		}

		bool WiimoteDeviceInterface::DeviceIsGood()
		{
			return (DeviceInterfaceHandle != INVALID_HANDLE_VALUE);
		}

		void WiimoteDeviceInterface::CloseDevice()
		{
			CloseHandle(DeviceInterfaceHandle);
			DeviceInterfaceHandle = INVALID_HANDLE_VALUE;
		}
	}
}
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

#include "Log.h"

#include "../HID Wiimote/SettingsInterface.h"

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

		State^ WiimoteDeviceInterface::GetState()
		{
			if (!DeviceIsGood())
			{
				return nullptr;
			}

			OVERLAPPED Overlapped = { 0 };
			WIIMOTE_STATE_IOCTL_DATA StateData;

			Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

			if (!SendBufferdIOCTL(IOCTL_WIIMOTE_GET_STATE, &StateData, sizeof(StateData), &Overlapped))
			{
				CloseHandle(Overlapped.hEvent);
				Log::Write("Error getting State");
				return nullptr;
			}

			CloseHandle(Overlapped.hEvent);

			return gcnew State(StateData);
		}

		bool WiimoteDeviceInterface::SendBufferdIOCTL(DWORD IoControlCode, LPVOID Buffer, DWORD BufferSize, LPOVERLAPPED Overlapped)
		{
			BOOL Result = DeviceIoControl(DeviceInterfaceHandle, IoControlCode, nullptr, 0, Buffer, BufferSize, nullptr, Overlapped);
			if (!Result)
			{
				DWORD Error = GetLastError();
				if (Error != ERROR_IO_PENDING)
				{
					Log::Write("Error calling DeviceIoControl: " + Error);
					return false;
				}
				else
				{
					DWORD NumberOfBytesTransferred;
					if (!GetOverlappedResult(DeviceInterfaceHandle, Overlapped, &NumberOfBytesTransferred, TRUE))
					{
						Log::Write("Error calling GetOverlappedResult: " + GetLastError());
						return false;
					}
				}
			}

			return true;
		}
	}
}
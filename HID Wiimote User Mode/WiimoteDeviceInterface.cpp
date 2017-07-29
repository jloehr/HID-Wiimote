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
			: DeviceInterfaceHandle(INVALID_HANDLE_VALUE),
			ReaderThread(nullptr),
			StopThread(false)
		{
			this->DeviceInterfacePath = DeviceInterfacePath;
		}

		State^ WiimoteDeviceInterface::Initialize()
		{
			if (DeviceIsGood())
			{
				Log::Write("Device Interface already initialized: " + DeviceInterfacePath);
			}

			// Open Device Interface Path
			if (!OpenDevice())
			{
				Log::Write("Error Opening Device: " + DeviceInterfacePath);
				return nullptr;
			}

			// Query Initinal State
			State^ InitinalState = GetState();

			if (InitinalState == nullptr)
			{
				Log::Write("Error getting initinal state " + DeviceInterfacePath);
				return nullptr;
			}

			StartContinousReader();

			return InitinalState;
		}

		void WiimoteDeviceInterface::Disconnect()
		{
			StopContinousReader();

			CloseDevice();

			DeviceRemoved(this, nullptr);
		}

		System::Boolean WiimoteDeviceInterface::SetDriverMode(DriverMode NewMode)
		{
			WIIMOTE_DRIVER_MODE Mode = (WIIMOTE_DRIVER_MODE)NewMode;

			return SendBufferdIOCTL(IOCTL_WIIMOTE_SET_MODE, &Mode, sizeof(Mode), nullptr, 0);
		}

		System::Boolean WiimoteDeviceInterface::SetEnableWiimoteXAxisAccelerometer(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_ENABLE_WIIMOTE_XAXIS_ACCELEROMETER, Enabled);
		}

		System::Boolean WiimoteDeviceInterface::SetEnableWiimoteYAxisAccelerometer(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_ENABLE_WIIMOTE_YAXIS_ACCELEROMETER, Enabled);
		}

		System::Boolean WiimoteDeviceInterface::SetSwitchMouseButtons(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_SWITCH_MOUSEBUTTONS, Enabled);
		}

		System::Boolean WiimoteDeviceInterface::SetSwitchTriggerAndShoulder(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_SWITCH_TRIGGERANDSHOULDER, Enabled);
		}

		System::Boolean WiimoteDeviceInterface::SetSplitTrigger(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_SPLIT_TRIGGERAXIS, Enabled);
		}

		System::Boolean WiimoteDeviceInterface::SetMapTriggerAsAxis(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_MAP_TRIGGER_AS_AXIS, Enabled);
		}

		System::Boolean WiimoteDeviceInterface::SetMapTriggerAsButtons(System::Boolean Enabled)
		{
			return SendBooleanSetting(IOCTL_WIIMOTE_SET_MAP_TRIGGER_AS_BUTTONS, Enabled);
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

		void WiimoteDeviceInterface::StartContinousReader()
		{
			if (ReaderThread != nullptr)
			{
				return;
			}

			System::Threading::ThreadStart^ ThreadEntry = gcnew System::Threading::ThreadStart(this, &WiimoteDeviceInterface::ContinousReaderThreadBody);
			ReaderThread = gcnew System::Threading::Thread(ThreadEntry);
			ReaderThread->Name = "Continous Reader Thread";
			StopThread = false;

			ReadIo = new OVERLAPPED();
			ReadIo->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			ReaderThread->Start();
		}

		void WiimoteDeviceInterface::StopContinousReader()
		{
			StopThread = true;

			if ((ReaderThread != nullptr) && (System::Threading::Thread::CurrentThread != ReaderThread) && (ReaderThread->IsAlive))
			{
				do {
					SetEvent(ReadIo->hEvent);
				} while (!ReaderThread->Join(100));
			}
		}

		void WiimoteDeviceInterface::ContinousReaderThreadBody()
		{
			WIIMOTE_STATUS_IOCTL_DATA StatusBuffer = {};
			DWORD ByteWritten = 0;

			while (!StopThread)
			{
				ResetEvent(ReadIo->hEvent);

				if (!SendBufferdIOCTL(IOCTL_WIIMOTE_READ_STATUS, nullptr, 0, &StatusBuffer, sizeof(StatusBuffer), ReadIo))
				{
					Log::Write("Error sending Read Status Request");
					Disconnect();
					break;
				}

				if (ReadIo->Internal == STATUS_PENDING)
				{
					// Stop Continous Reader was called;
					CancelIo(DeviceInterfaceHandle);
					break;
				}

				StatusUpdate(this, gcnew Status(StatusBuffer));
			}

			FreeThreadResources();
			ReaderThread = nullptr;
		}

		void WiimoteDeviceInterface::FreeThreadResources()
		{
			if (ReadIo != nullptr)
			{
				if (ReadIo->hEvent != INVALID_HANDLE_VALUE)
				{
					CloseHandle(ReadIo->hEvent);
					ReadIo->hEvent = INVALID_HANDLE_VALUE;
				}

				delete ReadIo;
				ReadIo = nullptr;
			}

		}

		State^ WiimoteDeviceInterface::GetState()
		{
			if (!DeviceIsGood())
			{
				return nullptr;
			}

			WIIMOTE_STATE_IOCTL_DATA StateData;

			if (!SendBufferdIOCTL(IOCTL_WIIMOTE_GET_STATE, nullptr, 0, &StateData, sizeof(StateData)))
			{
				Log::Write("Error getting State");
				return nullptr;
			}

			return gcnew State(StateData);
		}

		bool WiimoteDeviceInterface::SendBooleanSetting(DWORD IoControlCode, bool Value)
		{
			BOOL Converted = Value;

			return SendBufferdIOCTL(IoControlCode, &Converted, sizeof(Converted), nullptr, 0);
		}

		bool WiimoteDeviceInterface::SendBufferdIOCTL(DWORD IoControlCode, LPVOID InputBuffer, DWORD InputBufferSize, LPVOID OutputBuffer, DWORD OutputBufferSize)
		{
			if (!DeviceIsGood())
			{
				return false;
			}

			OVERLAPPED Overlapped = { 0 };
			Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

			bool Result = SendBufferdIOCTL(IoControlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, &Overlapped);

			CloseHandle(Overlapped.hEvent);

			return Result;
		}

		bool WiimoteDeviceInterface::SendBufferdIOCTL(DWORD IoControlCode, LPVOID InputBuffer, DWORD InputBufferSize, LPVOID OutputBuffer, DWORD OutputBufferSize, LPOVERLAPPED Overlapped)
		{
			if (!DeviceIsGood())
			{
				return false;
			}

			BOOL Result = DeviceIoControl(DeviceInterfaceHandle, IoControlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, nullptr, Overlapped);
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
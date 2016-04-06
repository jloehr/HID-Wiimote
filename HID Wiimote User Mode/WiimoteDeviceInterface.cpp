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

				if (!SendBufferdIOCTL(IOCTL_WIIMOTE_READ_STATUS, &StatusBuffer, sizeof(StatusBuffer), ReadIo))
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
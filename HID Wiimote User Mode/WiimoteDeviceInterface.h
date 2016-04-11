/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterface.h

Abstract:
	Public managed class that represents a HID Wiimote Device Interface

*/
#pragma once

#include "Status.h"
#include "State.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		public ref class WiimoteDeviceInterface
		{
		public:
			WiimoteDeviceInterface(System::String^ DeviceInterfacePath);

			property System::String^ DeviceInterfacePath;

			event System::EventHandler^ DeviceRemoved;
			event System::EventHandler<Status^>^ StatusUpdate;

			State^ Initialize();
			void Disconnect();

			System::Boolean SetDriverMode(DriverMode NewMode);
			System::Boolean SetXAxis(System::Boolean Enabled);
			System::Boolean SetYAxis(System::Boolean Enabled);
			System::Boolean SetMouseButtonsSwitched(System::Boolean Enabled);
			System::Boolean SetTriggerAndShoulderSwitched(System::Boolean Enabled);
			System::Boolean SetTriggerSplit(System::Boolean Enabled);

		private:
			HANDLE DeviceInterfaceHandle;
			LPOVERLAPPED ReadIo;

			System::Threading::Thread^ ReaderThread;
			volatile bool StopThread;

			bool OpenDevice();
			bool DeviceIsGood();
			void CloseDevice();

			void StartContinousReader();
			void StopContinousReader();
			void ContinousReaderThreadBody();

			void FreeThreadResources();

			State^ GetState();

			bool SendBooleanSetting(DWORD IoControlCode, bool Value);
			bool SendBufferdIOCTL(DWORD IoControlCode, LPVOID InputBuffer, DWORD InputBufferSize, LPVOID OutputBuffer, DWORD OutputBufferSize);
			bool SendBufferdIOCTL(DWORD IoControlCode, LPVOID InputBuffer, DWORD InputBufferSize, LPVOID OutputBuffer, DWORD OutputBufferSize, LPOVERLAPPED Overlapped);

		};
	}
}


/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterface.h

Abstract:
	Internal managed class that represents a HID Wiimote Device Interface

*/
#pragma once

#include "IWiimoteDeviceInterface.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		private ref class WiimoteDeviceInterface : IWiimoteDeviceInterface
		{
		public:
			WiimoteDeviceInterface(System::String^ DeviceInterfacePath);

			property System::String^ DeviceInterfacePath;

			virtual event System::EventHandler^ DeviceRemoved;
			virtual event System::EventHandler<Status^>^ StatusUpdate;

			virtual State^ Initialize();
			virtual void Disconnect();

			virtual System::Boolean SetDriverMode(DriverMode NewMode);
			virtual System::Boolean SetEnableWiimoteXAxisAccelerometer(System::Boolean Enabled);
			virtual System::Boolean SetEnableWiimoteYAxisAccelerometer(System::Boolean Enabled);
			virtual System::Boolean SetSwapMouseButtons(System::Boolean Enabled);
			virtual System::Boolean SetSwapTriggerAndShoulder(System::Boolean Enabled);
			virtual System::Boolean SetSplitTrigger(System::Boolean Enabled);
			virtual System::Boolean SetMapTriggerAsAxis(System::Boolean Enabled);
			virtual System::Boolean SetMapTriggerAsButtons(System::Boolean Enabled);

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


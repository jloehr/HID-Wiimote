/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterfaceDiscoverer.h

Abstract:
	Public managed class to find HID Wiimote Device Interfaces.

*/
#pragma once

#include "IWiimoteDeviceInterface.h"
#include "WiimoteDeviceInterface.h"
#include "WiimoteDeviceInterfaceEnumerator.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		public ref class WiimoteDeviceInterfaceDiscoverer
		{
		public:
			WiimoteDeviceInterfaceDiscoverer();
			~WiimoteDeviceInterfaceDiscoverer();

			void Start();
			void Stop();

			event System::EventHandler<IWiimoteDeviceInterface^>^ NewWiimoteDeviceInterfaceFound;

		private:
			PWiimoteDeviceInterfaceEnumerator NativeEnumerator;

			System::Collections::Generic::List<System::String^>^ ActiveInterfacesPaths;
			System::Threading::Thread^ SearchingThread;
			volatile bool StopThread;

			void SearchingThreadBody();
			void OnWiimoteDeviceInterfaceRemoved(System::Object ^sender, System::EventArgs ^e);
		};
	}
}


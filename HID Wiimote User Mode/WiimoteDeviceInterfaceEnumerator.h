/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterfaceEnumerator.h

Abstract:
	Native class that enumerates and returns all HID Wiimote Device Interfaces.

*/
#pragma once

namespace HIDWiimote
{
	namespace UserModeLib
	{
		class WiimoteDeviceInterfaceEnumerator;
		typedef WiimoteDeviceInterfaceEnumerator * PWiimoteDeviceInterfaceEnumerator;

		class WiimoteDeviceInterfaceEnumerator
		{
		public:
			typedef std::vector<std::wstring> DevicePathVector;

			bool Enumerate();

			const DevicePathVector & GetEnumeratedDevices()
			{
				return EnumeratedDevices;
			}

		private:
			bool EnumerateDeviceInterface(HDEVINFO DeviceInfoSet, DWORD DeviceIndex);

			DevicePathVector EnumeratedDevices;
		};
	}
}


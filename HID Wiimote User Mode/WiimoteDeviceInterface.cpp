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
		{
			this->DeviceInterfacePath = DeviceInterfacePath;
		}

		void WiimoteDeviceInterface::Initialize()
		{
		}
	}
}
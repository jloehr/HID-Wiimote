/*

Copyright (C) 2017 Julian Löhr
	All rights reserved.

Filename:
	IWiimoteDeviceInterface.h

Abstract:
	Public managed interface that represents a HID Wiimote Device Interface

*/
#pragma once

#include "Status.h"
#include "State.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		public interface class IWiimoteDeviceInterface
		{
			event System::EventHandler^ DeviceRemoved;
			event System::EventHandler<Status^>^ StatusUpdate;

			State^ Initialize();
			void Disconnect();

			System::Boolean SetDriverMode(DriverMode NewMode);
			System::Boolean SetEnableWiimoteXAxisAccelerometer(System::Boolean Enabled);
			System::Boolean SetEnableWiimoteYAxisAccelerometer(System::Boolean Enabled);
			System::Boolean SetSwitchMouseButtons(System::Boolean Enabled);
			System::Boolean SetSwitchTriggerAndShoulder(System::Boolean Enabled);
			System::Boolean SetSplitTrigger(System::Boolean Enabled);
			System::Boolean SetMapTriggerAsAxis(System::Boolean Enabled);
			System::Boolean SetMapTriggerAsButtons(System::Boolean Enabled);
		};
	}
}
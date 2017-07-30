/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Status.h

Abstract:
	Public managed class that contains information about an status update of a Wiimote Device

*/
#pragma once

#include "../HID Wiimote/DeviceInterfacePublic.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		public enum class Extension { None, Nunchuck, BalanceBoard, ClassicController, WiiUProController, Guitar };

		public ref class Status
		{
		public:
			Status();
			Status(const WIIMOTE_STATUS_IOCTL_DATA & StatusData);

			property array<System::Boolean>^ LEDState;
			property System::Byte BatteryLevel;
			property Extension Extension;

		private:
			void SetLEDState(UCHAR LEDs);
		};
	}
}


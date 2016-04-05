/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Status.h

Abstract:
	Public managed class that contains information about an status update of a Wiimote Device

*/
#pragma once

typedef struct _WIIMOTE_STATUS_IOCTL_DATA WIIMOTE_STATUS_IOCTL_DATA;

namespace HIDWiimote
{
	namespace UserModeLib
	{
		public enum class Extension { None, Nunchuck, BalanceBoard, ClassicController, WiiUProController, Guitar };

		public ref class Status
		{
		public:
			Status(const WIIMOTE_STATUS_IOCTL_DATA & StatusData);

			property array<System::Boolean>^ LEDState;
			property System::Byte BatteryLevel;
			property Extension Extension;

		private:
			void SetLEDState(UCHAR LEDs);
			void SetExtension(WIIMOTE_EXTENSION Extension);
		};
	}
}


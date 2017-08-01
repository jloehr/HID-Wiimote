/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	Status.cpp

Abstract:
	Implementation of the Status Class

*/
#include "stdafx.h"
#include "Status.h"

#include "Log.h"

#include "../HID Wiimote/DeviceInterfacePublic.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		Status::Status()
		{
			SetLEDState(0x00);
		}

		Status::Status(const WIIMOTE_STATUS_IOCTL_DATA & StatusData)
		{
			BatteryLevel = StatusData.BatteryLevel;
			Extension = (UserModeLib::Extension)StatusData.Extension;

			SetLEDState(StatusData.LEDs);
		}

		void Status::SetLEDState(UCHAR LEDs)
		{
			LEDState = gcnew array<System::Boolean>(4);

			LEDState[0] = (LEDs & WIIMOTE_LEDS_ONE);
			LEDState[1] = (LEDs & WIIMOTE_LEDS_TWO);
			LEDState[2] = (LEDs & WIIMOTE_LEDS_THREE);
			LEDState[3] = (LEDs & WIIMOTE_LEDS_FOUR);
		}
	}
}
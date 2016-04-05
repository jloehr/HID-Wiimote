/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Status.cpp

Abstract:
	Implementation of the Status Class

*/
#include "stdafx.h"
#include "Status.h"

#include "Log.h"

#include "../HID Wiimote/SettingsInterface.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		Status::Status(const WIIMOTE_STATUS_IOCTL_DATA & StatusData)
		{
			BatteryLevel = StatusData.BatteryLevel;

			SetLEDState(StatusData.LEDs);
			SetExtension(StatusData.Extension);
		}

		void Status::SetLEDState(UCHAR LEDs)
		{
			LEDState = gcnew array<System::Boolean>(4);

			LEDState[0] = (LEDs & WIIMOTE_LEDS_ONE);
			LEDState[1] = (LEDs & WIIMOTE_LEDS_TWO);
			LEDState[2] = (LEDs & WIIMOTE_LEDS_THREE);
			LEDState[3] = (LEDs & WIIMOTE_LEDS_FOUR);
		}

		void Status::SetExtension(WIIMOTE_EXTENSION Extension)
		{
			switch (Extension)
			{
			case None:
				this->Extension = UserModeLib::Extension::None;
				break;
			case Nunchuck:
				this->Extension = UserModeLib::Extension::Nunchuck;
				break;
			case BalanceBoard:
				this->Extension = UserModeLib::Extension::BalanceBoard;
				break;
			case ClassicController:
				this->Extension = UserModeLib::Extension::ClassicController;
				break;
			case WiiUProController:
				this->Extension = UserModeLib::Extension::WiiUProController;
				break;
			case Guitar:
				this->Extension = UserModeLib::Extension::Guitar;
				break;
			default:
				Log::Write("Managed Status Class encountered unknown Extension!");
				break;
			}
		}
	}
}
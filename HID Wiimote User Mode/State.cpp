/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	State.cpp

Abstract:
	Implementation of the Status Class

*/
#include "stdafx.h"
#include "State.h"

#include "Log.h"

#include "../HID Wiimote/SettingsInterface.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		State::State(const WIIMOTE_STATE_IOCTL_DATA & StateData)
		{
			Status = gcnew UserModeLib::Status(StateData.Status);

			SetMode(StateData.Mode);
			
			XAxisEnabled = (StateData.Settings.XAxisEnabled > 0);
			YAxisEnabled = (StateData.Settings.YAxisEnabled > 0);
			MouseButtonsSwitched = (StateData.Settings.MouseButtonsSwitched > 0);
			TriggerAndShoulderSwitched = (StateData.Settings.TriggerAndShoulderSwitched > 0);
			TriggerSplit = (StateData.Settings.TriggerSplit > 0);
		}

		void State::SetMode(WIIMOTE_DRIVER_MODE Mode)
		{
			switch (Mode)
			{
			case PassThrough:
				this->Mode = UserModeLib::DriverMode::PassThrough;
				break;
			case Gamepad:
				this->Mode = UserModeLib::DriverMode::Gamepad;
				break;
			case IRMouse:
				this->Mode = UserModeLib::DriverMode::IRMouse;
				break;
			case DPadMouse:
				this->Mode = UserModeLib::DriverMode::DPadMouse;
				break;
			case GamepadAndIRMouse:
				this->Mode = UserModeLib::DriverMode::GamepadAndIRMouse;
				break;
			default:
				Log::Write("Managed State Class encountered unknown Driver Mode!");
				break;
			}
		}
	}
}

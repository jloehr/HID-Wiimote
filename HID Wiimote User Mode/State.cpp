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

			Mode = (UserModeLib::DriverMode)StateData.Mode;

			XAxisEnabled = (StateData.Settings.XAxisEnabled != FALSE);
			YAxisEnabled = (StateData.Settings.YAxisEnabled != FALSE);
			MouseButtonsSwitched = (StateData.Settings.MouseButtonsSwitched != FALSE);
			TriggerAndShoulderSwitched = (StateData.Settings.TriggerAndShoulderSwitched != FALSE);
			TriggerSplit = (StateData.Settings.TriggerSplit != FALSE);
		}
	}
}

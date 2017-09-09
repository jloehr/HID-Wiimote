/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	State.cpp

Abstract:
	Implementation of the Status Class

*/
#include "stdafx.h"
#include "State.h"

#include "Log.h"

#include "../HID Wiimote/DeviceInterfacePublic.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		State::State()
		{
			Status = gcnew UserModeLib::Status();	
			Mode = DriverMode::PassThrough;
		}

		State::State(const WIIMOTE_STATE_IOCTL_DATA & StateData)
		{
			Status = gcnew UserModeLib::Status(StateData.Status);

			Mode = (UserModeLib::DriverMode)StateData.Settings.Mode;

			EnableWiimoteXAxisAccelerometer = (StateData.Settings.EnableWiimoteXAxisAccelerometer != FALSE);
			EnableWiimoteYAxisAccelerometer = (StateData.Settings.EnableWiimoteYAxisAcceleromenter != FALSE);
			SwapMouseButtons = (StateData.Settings.SwapMouseButtons != FALSE);
			SwapTriggerAndShoulder = (StateData.Settings.SwapTriggerAndShoulder != FALSE);
			SplitTrigger = (StateData.Settings.SplitTriggerAxis != FALSE);
			MapTriggerAsAxis = (StateData.Settings.MapTriggerAsAxis != FALSE);
			MapTriggerAsButtons = (StateData.Settings.MapTriggerAsButtons != FALSE);
		}
	}
}

/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	WiimoteSettingsPublic.h

Abstract:
	Types for the Wiimote Settings, that are also available to User Mode Space.
*/
#pragma once

#include "WiimotePublic.h"

typedef struct _WIIMOTE_SETTINGS
{
	WIIMOTE_DRIVER_MODE Mode;

	BOOLEAN XAxisEnabled;
	BOOLEAN YAxisEnabled;
	BOOLEAN MouseButtonsSwitched;
	BOOLEAN TriggerAndShoulderSwitched;
	BOOLEAN TriggerSplit;

} WIIMOTE_SETTINGS, *PWIIMOTE_SETTINGS;
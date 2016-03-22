/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Log.h

Abstract:
	Static methods for Logging

*/
#pragma once

namespace HIDWiimote
{
	namespace UserModeLib
	{
		public ref class Log abstract sealed
		{
		public:
			static event System::EventHandler<System::String^>^ NewLine;

			static void Write(System::String^ Text);
			static void Write(System::String^ Format, ... array<System::Object^>^ Args);
		};
	}
}

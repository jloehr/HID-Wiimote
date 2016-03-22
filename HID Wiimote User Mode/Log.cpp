/*

Copyright (C) 2016 Julian Löhr
All rights reserved.

Filename:
	Log.cpp

Abstract:
	Implementation of Logging

*/
#include "stdafx.h"
#include "Log.h"


namespace HIDWiimote
{
	namespace UserModeLib
	{
		void Log::Write(System::String^ Text)
		{
#if _DEBUG
			System::Diagnostics::Debug::WriteLine(Text);
#endif
			Log::NewLine(nullptr, Text);
		}

		void Log::Write(System::String^ Format, ... array<System::Object^>^ Args)
		{
			System::String^ FormatedString = System::String::Format(Format, Args);

			Write(FormatedString);
		}
	}
}
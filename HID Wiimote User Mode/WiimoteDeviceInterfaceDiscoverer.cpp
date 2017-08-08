/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	WiimoteDeviceInterfaceDiscoverer.cpp

Abstract:
	Implementation of the WiimoteDeviceInterfaceDiscoverer Class

*/
#include "stdafx.h"
#include "WiimoteDeviceInterfaceDiscoverer.h"

#include "WiimoteDeviceInterfaceEnumerator.h"

namespace HIDWiimote
{
	namespace UserModeLib
	{
		WiimoteDeviceInterfaceDiscoverer::WiimoteDeviceInterfaceDiscoverer()
			: NativeEnumerator(new WiimoteDeviceInterfaceEnumerator()),
			SearchingThread(nullptr),
			StopThread(false),
			ActiveInterfacesPaths(gcnew System::Collections::Generic::List<System::String^>)
		{
		}

		WiimoteDeviceInterfaceDiscoverer::~WiimoteDeviceInterfaceDiscoverer()
		{
			Stop();

			if (NativeEnumerator != nullptr)
			{
				delete NativeEnumerator;
				NativeEnumerator = nullptr;
			}
		}

		void WiimoteDeviceInterfaceDiscoverer::Start()
		{
			if (SearchingThread != nullptr)
			{
				return;
			}

			System::Threading::ThreadStart^ ThreadEntry = gcnew System::Threading::ThreadStart(this, &WiimoteDeviceInterfaceDiscoverer::SearchingThreadBody);
			SearchingThread = gcnew System::Threading::Thread(ThreadEntry);
			SearchingThread->Name = "Searching Thread";
			StopThread = false;

			SearchingThread->Start();
		}

		void WiimoteDeviceInterfaceDiscoverer::Stop()
		{
			if (SearchingThread == nullptr)
			{
				return;
			}

			StopThread = true;
			SearchingThread->Join();

			SearchingThread = nullptr;
		}

		void WiimoteDeviceInterfaceDiscoverer::SearchingThreadBody()
		{
			while (!StopThread)
			{
				// Refresh Enumeration
				NativeEnumerator->Enumerate();
				WiimoteDeviceInterfaceEnumerator::DevicePathVector EnumeratedInterfaces = NativeEnumerator->GetEnumeratedDevices();

				// Enter Critical Section 
				System::Threading::Monitor::Enter(ActiveInterfacesPaths);

				for (auto & DeviceInterfacePath : EnumeratedInterfaces)
				{
					// Check if it already has been enumerated and discovered
					System::String^ ManagedDevicePath = gcnew System::String(DeviceInterfacePath.c_str());
					if (ActiveInterfacesPaths->Contains(ManagedDevicePath))
					{
						continue;
					}

					// If not create DeviceInterfaceWrapper, register onto its removed event and fire our event
					WiimoteDeviceInterface^ NewDeviceInterface = gcnew WiimoteDeviceInterface(ManagedDevicePath);
					NewDeviceInterface->DeviceRemoved += gcnew System::EventHandler(this, &HIDWiimote::UserModeLib::WiimoteDeviceInterfaceDiscoverer::OnWiimoteDeviceInterfaceRemoved);

					ActiveInterfacesPaths->Add(ManagedDevicePath);

					NewWiimoteDeviceInterfaceFound(this, NewDeviceInterface);
				}

				// Leave Critical Section and sleep for some time before we do the next enumeration
				System::Threading::Monitor::Exit(ActiveInterfacesPaths);
				System::Threading::Thread::Sleep(1000);
			}
		}

		void WiimoteDeviceInterfaceDiscoverer::OnWiimoteDeviceInterfaceRemoved(System::Object ^ sender, System::EventArgs ^ e)
		{
			WiimoteDeviceInterface^ RemovedInterferace = (WiimoteDeviceInterface^)sender;

			// Enter Critical Section 
			System::Threading::Monitor::Enter(ActiveInterfacesPaths);

			ActiveInterfacesPaths->Remove(RemovedInterferace->DeviceInterfacePath);

			// Leave Critical Section 
			System::Threading::Monitor::Exit(ActiveInterfacesPaths);
		}
	}
}

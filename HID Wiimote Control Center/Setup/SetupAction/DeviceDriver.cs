/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	DeviceDriver.cs

Abstract:
	Setup Action to install and uninstall the device driver

*/
namespace HIDWiimote.ControlCenter.Setup.SetupAction
{

    class DeviceDriver : ISetupAction
    {
        public bool IsSetUp()
        {
            return Setup.DeviceDriver.IsInstalled();
        }

        public void TrySetUp()
        {
            Setup.DeviceDriver.Install();
        }

        public void TryRevert()
        {
            Setup.DeviceDriver.Uninstall();
        }
    }
}

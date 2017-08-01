/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	DriverPackage.cs

Abstract:
	Setup Action to install and uninstall the driver package

*/
namespace HIDWiimote.ControlCenter.Setup.SetupAction
{

    class DriverPackage : ISetupAction
    {
        public bool IsSetUp()
        {
            return Setup.DriverPackage.IsInstalled();
        }

        public void TrySetUp()
        {
            Setup.DriverPackage.Install();
        }

        public void TryRevert()
        {
            Setup.DriverPackage.Uninstall();
        }
    }
}

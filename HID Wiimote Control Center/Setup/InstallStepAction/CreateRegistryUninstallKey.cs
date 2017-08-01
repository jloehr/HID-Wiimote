/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	CreateRegistryUninstallKey.cs

Abstract:
	Install Task to create the Uninstaller Registry Key

*/
namespace HIDWiimote.ControlCenter.Setup.InstallStepAction
{
    class CreateRegistryUninstallKey : IInstallStepAction
    {
        string UninstallString;
        string DPInstPath;

        public CreateRegistryUninstallKey(string UninstallString, string DPInstPath)
        {
            this.UninstallString = UninstallString;
            this.DPInstPath = DPInstPath;
        }

        public bool Do()
        {
            return DriverPackageUninstallerRegistry.CreateHIDWiimoteUninstallKey(UninstallString, DPInstPath);
        }

        public void Undo()
        {
        }

        public string GetExceptionMessage()
        {
            return "An error occured while creating the \"Programs and Features\" entry!";
        }
    }
}

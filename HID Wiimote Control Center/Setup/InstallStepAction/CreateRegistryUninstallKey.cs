using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup.InstallStepAction
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

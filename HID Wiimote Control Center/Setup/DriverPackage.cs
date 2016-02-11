using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup
{
    class DriverPackage : IInstallerTask
    {
        public DriverPackage()
        {
                
        }

        public bool IsGood()
        {
            DriverPackageUninstallerRegistry.DriverPackageState DPState = DriverPackageUninstallerRegistry.GetDriverPackageState(VersionStrings.DriverPackageVersion);           

            return (DPState != DriverPackageUninstallerRegistry.DriverPackageState.NoneInstalled);
        }

        public void TryMakeBad()
        {
            Uninstall();
        }

        public void TryMakeGood()
        {
            // Install
        }

        public static void Uninstall()
        {
            string UninstallerString = DriverPackageUninstallerRegistry.GetUninstallString();
            
            Process Uninstall = Process.Start("cmd.exe", "/C " + UninstallerString);
            Uninstall.WaitForExit();
        }
    }
}

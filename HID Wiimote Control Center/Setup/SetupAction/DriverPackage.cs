using HID_Wiimote_Control_Center.Setup.InstallStepAction;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace HID_Wiimote_Control_Center.Setup.SetupAction
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

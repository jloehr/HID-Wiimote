using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup.InstallStepAction
{
    interface IInstallStepAction
    {
        bool Do();
        void Undo();
        string GetExceptionMessage();
    }
}

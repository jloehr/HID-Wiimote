using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup.SetupAction
{
    public interface ISetupAction
    {
        bool IsSetUp();
        void TrySetUp();
        void TryRevert();
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center.Setup
{
    class Certificate : IInstallerTask
    {
        public Certificate()
        {

        }

        public bool IsGood()
        {
            return false;
        }

        public void TryMakeBad()
        {
        }

        public void TryMakeGood()
        {
        }
    }
}

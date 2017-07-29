using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HIDWiimote.UserModeLib;
using System.Threading;

namespace HIDWiimote.ControlCenter.Control_Center
{
    class DummyDeviceInterface : UserModeLib.IWiimoteDeviceInterface
    {
        private int DummySleep = 500;

        public event EventHandler DeviceRemoved
        {
            add { }
            remove { }
        }

        public event EventHandler<Status> StatusUpdate
        {
            add { }
            remove { }
        }


        public void Disconnect()
        {
        }

        public State Initialize()
        {
            return new State();
        }

        public bool SetDriverMode(DriverMode NewMode)
        {
            Thread.Sleep(DummySleep);
            return true;
        }

        public bool SetEnableWiimoteXAxisAccelerometer(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }

        public bool SetEnableWiimoteYAxisAccelerometer(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }
        
        public bool SetSwitchMouseButtons(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }

        public bool SetSwitchTriggerAndShoulder(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }

        public bool SetSplitTrigger(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }

        public bool SetMapTriggerAsAxis(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }

        public bool SetMapTriggerAsButtons(bool Enabled)
        {
            Thread.Sleep(DummySleep);
            return true;
        }
    }
}

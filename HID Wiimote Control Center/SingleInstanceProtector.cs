using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace HID_Wiimote_Control_Center
{
    class SingleInstanceProtector
    {
        private const string MutexGUID = "{29E5C640-0885-47E3-A4B2-8F4A2D96F9AD}";
        private Mutex InstanceMutex;
        private bool MutexCreated;

        public SingleInstanceProtector()
        {
            InstanceMutex = new Mutex(true, MutexGUID, out MutexCreated);
            if(MutexCreated)
            {
                App.Current.Exit += OnAppExit;
            }
        }

        public bool IsFirstInstance()
        {
            return (MutexCreated == true);
        }

        private void OnAppExit(object sender, System.Windows.ExitEventArgs e)
        {
            InstanceMutex.ReleaseMutex();
        }

    }
}

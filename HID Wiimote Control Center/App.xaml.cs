using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows;

namespace HID_Wiimote_Control_Center
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        SingleInstanceProtector SingleInstanceProtector;

        public App()
        {
            this.Startup += OnStartup;
            SingleInstanceProtector = new SingleInstanceProtector();
        }

        private void OnStartup(object sender, StartupEventArgs e)
        {
            if(!SingleInstanceProtector.IsFirstInstance())
            {
                SingleInstanceProtector.ShowOtherAppInstance();
                Shutdown();
                return;
            }

            MessageBox.Show("On StartUp");
        }
    }
}

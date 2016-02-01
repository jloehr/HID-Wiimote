using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
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
                // Bring Other Window up
                Shutdown();
                return;
            }

            MessageBox.Show("On StartUp");
        }
    }
}

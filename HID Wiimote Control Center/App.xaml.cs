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

            string InstalledDriverPackageVersionString;
            DriverPackageUninstallerRegistry.DriverPackageState DPState = DriverPackageUninstallerRegistry.GetDriverPackageState(VersionStrings.DriverPackageVersion, out InstalledDriverPackageVersionString);          

            switch (DPState)
            {
                case DriverPackageUninstallerRegistry.DriverPackageState.NoneInstalled:
                    // Installer Window
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.OlderInstalled:
                    // Update Windows
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.CurrentInstalled:
                    // Control Center
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.NewerInstalled:
                    // Show Notice & shutdown
                    MessageBox.Show("There is a newer Driver Package already installed.\nPlease use the Control Center that comes with that Driver Package.\n\nCompatible Driver Package: " + VersionStrings.DriverPackageVersion + "\nInstalled Driver Package: " + InstalledDriverPackageVersionString, "HID Wiimote Control Center");
                    Shutdown();
                    return;
            }
        }
    }
}

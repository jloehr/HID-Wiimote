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
                    StartupUri = new Uri("InstallerWindow.xaml", UriKind.Relative);
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.OlderInstalled:
                    // Update Windows
                    StartupUri = new Uri("UpdaterWindow.xaml", UriKind.Relative);
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.CurrentInstalled:
                    // Control Center
                    StartupUri = new Uri("ControlCenterWindow.xaml", UriKind.Relative);
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.NewerInstalled:
                    // Show Notice & shutdown
                    MessageBox.Show(HID_Wiimote_Control_Center.Properties.App.NewerInstalledDialog_MainMessage + "\n\n" + HID_Wiimote_Control_Center.Properties.App.NewerInstalledDialog_CompatibleDriverPackage + ": " + VersionStrings.DriverPackageVersion + "\n" + HID_Wiimote_Control_Center.Properties.App.NewerInstalledDialog_InstalledDriverPackage + ": " + InstalledDriverPackageVersionString, "HID Wiimote Control Center");
                    Shutdown();
                    return;
            }
        }
    }
}

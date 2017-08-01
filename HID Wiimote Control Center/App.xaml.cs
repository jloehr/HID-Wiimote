/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	App.cs

Abstract:
	App class

*/
using System;
using System.Windows;

namespace HIDWiimote.ControlCenter
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
            if (!SingleInstanceProtector.IsFirstInstance())
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
                    StartupUri = new Uri("Main Windows\\InstallerWindow.xaml", UriKind.Relative);
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.OlderInstalled:
                    // Update Windows
                    StartupUri = new Uri("Main Windows\\UpdaterWindow.xaml", UriKind.Relative);
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.CurrentInstalled:
                    // Control Center
                    StartupUri = new Uri("Main Windows\\ControlCenterWindow.xaml", UriKind.Relative);
                    break;
                case DriverPackageUninstallerRegistry.DriverPackageState.NewerInstalled:
                    // Show Notice & shutdown
                    MessageBox.Show(HIDWiimote.ControlCenter.Properties.App.NewerInstalledDialog_MainMessage + "\n\n" + HIDWiimote.ControlCenter.Properties.App.NewerInstalledDialog_CompatibleDriverPackage + ": " + VersionStrings.DriverPackageVersion + "\n" + HIDWiimote.ControlCenter.Properties.App.NewerInstalledDialog_InstalledDriverPackage + ": " + InstalledDriverPackageVersionString, "HID Wiimote Control Center", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    Shutdown();
                    return;
            }
        }

        public static void ChangeMainWindow(Window NewWindow, Window CallingWindow)
        {
            Current.MainWindow = NewWindow;
            NewWindow.Show();
            CallingWindow.Close();
        }
    }
}

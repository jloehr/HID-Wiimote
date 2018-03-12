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
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
        }

        private void OnStartup(object sender, StartupEventArgs e)
        {
            SingleInstanceProtector = new SingleInstanceProtector();

            if (!SingleInstanceProtector.IsFirstInstance())
            {
                SingleInstanceProtector.ShowOtherAppInstance();
                Shutdown();
                return;
            }

            DeviceDriverUninstallerRegistry.DeviceDriverState DPState = DeviceDriverUninstallerRegistry.GetDeviceDriverState(VersionStrings.DeviceDriverVersion, out string InstalledDeviceDriverVersionString);

            switch (DPState)
            {
                case DeviceDriverUninstallerRegistry.DeviceDriverState.NoneInstalled:
                    // Installer Window
                    StartupUri = new Uri("Main Windows\\InstallerWindow.xaml", UriKind.Relative);
                    break;
                case DeviceDriverUninstallerRegistry.DeviceDriverState.OlderInstalled:
                    // Update Windows
                    StartupUri = new Uri("Main Windows\\UpdaterWindow.xaml", UriKind.Relative);
                    break;
                case DeviceDriverUninstallerRegistry.DeviceDriverState.CurrentInstalled:
                    // Control Center
                    StartupUri = new Uri("Main Windows\\ControlCenterWindow.xaml", UriKind.Relative);
                    break;
                case DeviceDriverUninstallerRegistry.DeviceDriverState.NewerInstalled:
                    // Show Notice & shutdown
                    MessageBox.Show(string.Format(HIDWiimote.ControlCenter.Properties.App.NewerInstalledDialog_MainMessage, VersionStrings.DeviceDriverVersion, InstalledDeviceDriverVersionString), "HID Wiimote Control Center", MessageBoxButton.OK, MessageBoxImage.Exclamation);
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

        private void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            Exception Exception = e.ExceptionObject as Exception;

            while (Exception != null)
            {
                MessageBox.Show("An unhandled exception of type '" + Exception.GetType() + "' occured in '" + Exception.Source + "'.\n\nAdditional information: " + Exception.Message + "\n\nStack trace: \n" + Exception.StackTrace, Exception.GetType().ToString(), MessageBoxButton.OK, MessageBoxImage.Error, MessageBoxResult.None, MessageBoxOptions.DefaultDesktopOnly);

                Exception = Exception.InnerException;
            }
        }
    }
}

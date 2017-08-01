/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	SingleInstanceProtector.cs

Abstract:
    Single program instance protector

*/
using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace HIDWiimote.ControlCenter
{
    class SingleInstanceProtector
    {

        [DllImport("user32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);
        [DllImport("user32.dll")]
        private static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
        [DllImport("user32.dll")]
        private static extern bool IsIconic(IntPtr hWnd);

        private const int SW_RESTORE = 9;

        private const string MutexGUID = "{29E5C640-0885-47E3-A4B2-8F4A2D96F9AD}";
        private Mutex InstanceMutex;
        private bool MutexCreated;

        public SingleInstanceProtector()
        {
            InstanceMutex = new Mutex(true, MutexGUID, out MutexCreated);
            if (MutexCreated)
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

        public void ShowOtherAppInstance()
        {
            string AssemblyName = Assembly.GetExecutingAssembly().GetName().Name;
            int ThisProcessId = Process.GetCurrentProcess().Id;

            foreach (Process Other in Process.GetProcessesByName(AssemblyName))
            {
                if (Other.Id == ThisProcessId)
                {
                    continue;
                }

                if (IsIconic(Other.MainWindowHandle))
                {
                    ShowWindowAsync(Other.MainWindowHandle, SW_RESTORE);
                }

                SetForegroundWindow(Other.MainWindowHandle);
            }
        }
    }
}

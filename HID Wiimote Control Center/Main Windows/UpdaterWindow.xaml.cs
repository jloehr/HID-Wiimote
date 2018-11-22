/*

Copyright (C) 2018 Julian Löhr
All rights reserved.

Filename:
	InstallerWindow.xaml.cs

Abstract:
	Window for the updating process

*/
using HIDWiimote.ControlCenter.Setup;
using HIDWiimote.ControlCenter.UserControls;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace HIDWiimote.ControlCenter.Main_Windows
{
    /// <summary>
    /// Interaction logic for UpdaterWindow.xaml
    /// </summary>
    public partial class UpdaterWindow : Window
    {
        List<TaskList.Item> UpdateTasks = new List<TaskList.Item>();
        bool UpdateSuccesfull = false;
        string ErrorMessage;

        public UpdaterWindow()
        {
            if (!AskForUpdate())
            {
                this.Close();
                return;
            }

            UpdateTasks.Add(new TaskList.Item(HIDWiimote.ControlCenter.Properties.App.Updater_RemoveDeviceDriverPMessage, RemoveOldDeviceDriver));
            UpdateTasks.Add(new TaskList.Item(HIDWiimote.ControlCenter.Properties.App.Updater_InstallDeviceDriverMessage, InstallNewDeviceDriver));

            InitializeComponent();
        }

        private bool AskForUpdate()
        {
            string InstalledVersion = DeviceDriverUninstallerRegistry.GetInstalledVersionString(); 
            MessageBoxResult Result = MessageBox.Show(string.Format(HIDWiimote.ControlCenter.Properties.App.UpdaterDialog_MainMessage, VersionStrings.DeviceDriverVersion, InstalledVersion), "HID Wiimote Updater", MessageBoxButton.YesNo, MessageBoxImage.Question);

            return (Result == MessageBoxResult.Yes);
        }

        private void OnInitialized(object sender, EventArgs e)
        {
            TaskListBox.DataContext = UpdateTasks;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (UpdateTasks.Count > 0)
            {
                // Do some delay, so the user can graps the steps
                Task.Delay(TimeSpan.FromSeconds(0.25)).ContinueWith(DelayTask => { StartTaskQueue(); }, TaskScheduler.FromCurrentSynchronizationContext());
            }
        }

        private void StartTaskQueue()
        {
            UpdateTasks[0].StartTask(UpdaterTaskComplete);
        }

        private void UpdaterTaskComplete(Task<bool> CompletedTask, Object State)
        {
            TaskList.Item CompletedUpdaterTask = State as TaskList.Item;

            if ((CompletedTask.Status == TaskStatus.Faulted) || (CompletedTask.Result == false))
            {
                // Error
                CompletedUpdaterTask.Status = TaskList.Item.TaskStatus.Error;
                CloseButton.IsEnabled = true;
                if (ErrorMessage.Length != 0)
                {
                    MessageBox.Show(ErrorMessage, HIDWiimote.ControlCenter.Properties.App.Update_RemoveDeviceDriverErrorDialog_Title + CompletedUpdaterTask.DisplayMessage, MessageBoxButton.OK, MessageBoxImage.Error);
                }
                return;
            }


            CompletedUpdaterTask.Status = TaskList.Item.TaskStatus.Finished;
            int TaskIndex = UpdateTasks.IndexOf(CompletedUpdaterTask);

            if (TaskIndex == (UpdateTasks.Count - 1))
            {
                // Last one finished
                UpdateSuccesfull = true;
                CloseButton.IsEnabled = true;
                return;
            }

            TaskList.Item NextTask = UpdateTasks[TaskIndex + 1];
            NextTask.StartTask(UpdaterTaskComplete);
        }

        private bool RemoveOldDeviceDriver()
        {
            try
            {
                DeviceDriver.Uninstall();
            }
            catch (Exception e)
            {
                ErrorMessage = HIDWiimote.ControlCenter.Properties.App.Updater_RemoveDeviceDriverException + "\n\n" + e.Message;
                return false;
            }

            bool UninstallCheck = (DeviceDriverUninstallerRegistry.GetUninstallString().Length == 0);
            if (!UninstallCheck)
            {
                ErrorMessage = HIDWiimote.ControlCenter.Properties.App.Updater_RemoveDeviceDriverCheckFailed;
                return false;
            }

            return true;
        }

        private bool InstallNewDeviceDriver()
        {
            try
            {
                DeviceDriver.Install();
            }
            catch (Exception e)
            {
                ErrorMessage = HIDWiimote.ControlCenter.Properties.App.Updater_InstallDeviceDriverException + "\n\n" + e.Message;
                return false;
            }

            bool InstallCheck = (DeviceDriverUninstallerRegistry.GetUninstallString().Length != 0);
            if (!InstallCheck)
            {
                ErrorMessage = HIDWiimote.ControlCenter.Properties.App.Updater_InstallDeviceDriverCheckFailed;
                return false;
            }

            return true;
        }

        private void CloseButtonClick(object sender, RoutedEventArgs e)
        {
            if (UpdateSuccesfull)
            {
                App.ChangeMainWindow(new ControlCenterWindow(), this);
            }
            else
            {
                this.Close();
            }
        }
    }
}

/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	InstallerWindow.xaml.cs

Abstract:
	Window for the updating process

*/
using HIDWiimote.ControlCenter.Setup;
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
        List<UpdaterTask> TaskList = new List<UpdaterTask>();
        bool UpdateSuccesfull = false;
        string ErrorMessage;

        public UpdaterWindow()
        {
            if (!AskForUpdate())
            {
                this.Close();
                return;
            }

            TaskList.Add(new UpdaterTask(HIDWiimote.ControlCenter.Properties.App.Updater_RemoveDeviceDriverPMessage, RemoveOldDeviceDriver));
            TaskList.Add(new UpdaterTask(HIDWiimote.ControlCenter.Properties.App.Updater_InstallDeviceDriverMessage, InstallNewDeviceDriver));

            InitializeComponent();
        }

        private bool AskForUpdate()
        {
            string InstalledVersion = DeviceDriverUninstallerRegistry.GetInstalledVersionString(); 
            MessageBoxResult Result = MessageBox.Show(string.Format(HIDWiimote.ControlCenter.Properties.App.UpdaterDialog_MainMessage, InstalledVersion, VersionStrings.DeviceDriverVersion), "HID Wiimote Updater", MessageBoxButton.YesNo, MessageBoxImage.Question);

            return (Result == MessageBoxResult.Yes);
        }

        private void OnInitialized(object sender, EventArgs e)
        {
            TaskListBox.ItemsSource = TaskList;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (TaskList.Count > 0)
            {
                TaskList[0].StartTask(UpdaterTaskComplete);
            }
        }

        private void UpdaterTaskComplete(Task<bool> CompletedTask, Object State)
        {
            UpdaterTask CompletedUpdaterTask = State as UpdaterTask;

            if ((CompletedTask.Status == TaskStatus.Faulted) || (CompletedTask.Result == false))
            {
                // Error
                CompletedUpdaterTask.Status = UpdaterTask.TaskStatus.Error;
                CloseButton.IsEnabled = true;
                if (ErrorMessage.Length != 0)
                {
                    MessageBox.Show(ErrorMessage, HIDWiimote.ControlCenter.Properties.App.Update_RemoveDeviceDriverErrorDialog_Title + CompletedUpdaterTask.DisplayMessage, MessageBoxButton.OK, MessageBoxImage.Error);
                }
                return;
            }


            CompletedUpdaterTask.Status = UpdaterTask.TaskStatus.Finished;
            int TaskIndex = TaskList.IndexOf(CompletedUpdaterTask);

            if (TaskIndex == (TaskList.Count - 1))
            {
                // Last one finished
                UpdateSuccesfull = true;
                CloseButton.IsEnabled = true;
                return;
            }

            UpdaterTask NextTask = TaskList[TaskIndex + 1];
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

        public class UpdaterTask : INotifyPropertyChanged
        {
            public enum TaskStatus { Waiting, Running, Finished, Error };

            public event PropertyChangedEventHandler PropertyChanged;

            private string _DisplayMessage;
            private TaskStatus _Status;

            private Task<bool> QueuedTask;

            public UpdaterTask(string DisplayMessage, Func<bool> QueuedTask)
            {
                this.DisplayMessage = DisplayMessage;
                this.Status = TaskStatus.Waiting;
                this.QueuedTask = new Task<bool>(QueuedTask);
            }

            public UpdaterTask(string DisplayMessage, TaskStatus Status)
            {
                this.DisplayMessage = DisplayMessage;
                this.Status = Status;
            }

            public string DisplayMessage
            {
                get { return _DisplayMessage; }
                set
                {
                    _DisplayMessage = value;
                    OnPropertyChanged("DisplayMessage");
                }
            }

            public TaskStatus Status
            {
                get { return _Status; }
                set
                {
                    _Status = value;
                    OnPropertyChanged("Status");
                }
            }

            public void StartTask(Action<Task<bool>, Object> CompleteCallback)
            {
                Status = TaskStatus.Running;
                QueuedTask.ContinueWith(CompleteCallback, this, TaskScheduler.FromCurrentSynchronizationContext());
                QueuedTask.Start(TaskScheduler.Default);
            }

            protected void OnPropertyChanged(string PropertyName)
            {
                PropertyChangedEventHandler Handler = PropertyChanged;
                if (Handler != null)
                {
                    Handler(this, new PropertyChangedEventArgs(PropertyName));
                }
            }
        }
    }

    public class StatusToBulletPointValueConverter : IValueConverter
    {
        public ControlTemplate BulletPoint { get; set; }
        public ControlTemplate Arrow { get; set; }
        public ControlTemplate Checkmark { get; set; }
        public ControlTemplate Error { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            UpdaterWindow.UpdaterTask.TaskStatus Status = (UpdaterWindow.UpdaterTask.TaskStatus)value;

            switch (Status)
            {
                case UpdaterWindow.UpdaterTask.TaskStatus.Waiting:
                    return BulletPoint;
                case UpdaterWindow.UpdaterTask.TaskStatus.Running:
                    return Arrow;
                case UpdaterWindow.UpdaterTask.TaskStatus.Finished:
                    return Checkmark;
                case UpdaterWindow.UpdaterTask.TaskStatus.Error:
                    return Error;
                default:
                    return BulletPoint;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}

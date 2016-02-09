using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace HID_Wiimote_Control_Center
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
            InitializeComponent();

            TaskListBox.ItemsSource = TaskList;

            TaskList.Add(new UpdaterTask(HID_Wiimote_Control_Center.Properties.App.Updater_RemoveDPMessage, RemoveOldDriverPackage));
            TaskList.Add(new UpdaterTask(HID_Wiimote_Control_Center.Properties.App.Updater_InstallDPMessage, InstallNewDriverPackage));
            
            TaskList[0].StartTask(UpdaterTaskComplete);

        }

        private void OnInitialized(object sender, EventArgs e)
        {
            MessageBoxResult Result = MessageBox.Show(HID_Wiimote_Control_Center.Properties.App.UpdaterDialog_MainMessage, "HID Wiimote Updater", MessageBoxButton.YesNo, MessageBoxImage.Question);

            if(Result != MessageBoxResult.Yes)
            {
                this.Close();
                return;
            }
        }

        private void UpdaterTaskComplete(Task<bool> CompletedTask, Object State)
        {
            UpdaterTask CompletedUpdaterTask = State as UpdaterTask;

            if((CompletedTask.Status == TaskStatus.Faulted) || (CompletedTask.Result == false))
            {
                // Error
                CompletedUpdaterTask.Status = UpdaterTask.TaskStatus.Error;
                CloseButton.IsEnabled = true;
                if(ErrorMessage.Length != 0)
                {
                    MessageBox.Show(ErrorMessage);
                }
                return;
            }


            CompletedUpdaterTask.Status = UpdaterTask.TaskStatus.Finished;
            int TaskIndex = TaskList.IndexOf(CompletedUpdaterTask);

            if(TaskIndex == (TaskList.Count - 1))
            {
                // Last one finished
                UpdateSuccesfull = true;
                CloseButton.IsEnabled = true;
                return;
            }

            UpdaterTask NextTask = TaskList[TaskIndex + 1];
            NextTask.StartTask(UpdaterTaskComplete);
        }

        private bool RemoveOldDriverPackage()
        {
            string UninstallerString = DriverPackageUninstallerRegistry.GetUninstallString();                       

            try
            {
                Process Uninstall = Process.Start("cmd.exe", "/C " + UninstallerString);
                Uninstall.WaitForExit();
            }
            catch(Exception e)
            {
                ErrorMessage = HID_Wiimote_Control_Center.Properties.App.Updater_RemoveDPException + "\n\n" + e.Message;
                return false;
            }

            bool UninstallCheck = (DriverPackageUninstallerRegistry.GetUninstallString().Length == 0);
            if(!UninstallCheck)
            {
                ErrorMessage = HID_Wiimote_Control_Center.Properties.App.Updater_RemoveDPCheckFailed;
                return false;
            }

            return true;
        }

        private bool InstallNewDriverPackage()
        {
            Thread.Sleep(5000);
            // Run DPinst.exe

            return false;
        }

        private void CloseButtonClick(object sender, RoutedEventArgs e)
        {
            if(UpdateSuccesfull)
            {
                ControlCenterWindow NewWindow = new ControlCenterWindow();
                App.Current.MainWindow = NewWindow;
                NewWindow.Show();
            }

            this.Close();
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

/*

Copyright (C) 2018 Julian Löhr
All rights reserved.

Filename:
	TaskList.xaml.cs

Abstract:
	User Control for displaying a list of tasks that are processed

*/
using System;
using System.ComponentModel;
using System.Globalization;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Data;

namespace HIDWiimote.ControlCenter.UserControls
{
    /// <summary>
    /// Interaction logic for TaskList.xaml
    /// </summary>
    public partial class TaskList : UserControl
    {
        public TaskList()
        {
            InitializeComponent();
        }

        public class Item : INotifyPropertyChanged
        {
            public enum TaskStatus { Waiting, Running, Finished, Error };

            public event PropertyChangedEventHandler PropertyChanged;

            private string _DisplayMessage;
            private TaskStatus _Status;

            private Task<bool> QueuedTask;

            public Item(string DisplayMessage, Func<bool> QueuedTask)
            {
                this.DisplayMessage = DisplayMessage;
                this.Status = TaskStatus.Waiting;
                this.QueuedTask = new Task<bool>(QueuedTask);
            }

            public Item(string DisplayMessage, TaskStatus Status)
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
                // Do some delay, so the user can graps the steps
                System.Threading.Tasks.Task.Delay(TimeSpan.FromSeconds(0.5)).ContinueWith(DelayTask => { QueuedTask.Start(TaskScheduler.Default); });
            }

            protected void OnPropertyChanged(string PropertyName)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(PropertyName));
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
            TaskList.Item.TaskStatus Status = (TaskList.Item.TaskStatus)value;

            switch (Status)
            {
                case TaskList.Item.TaskStatus.Waiting:
                    return BulletPoint;
                case TaskList.Item.TaskStatus.Running:
                    return Arrow;
                case TaskList.Item.TaskStatus.Finished:
                    return Checkmark;
                case TaskList.Item.TaskStatus.Error:
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

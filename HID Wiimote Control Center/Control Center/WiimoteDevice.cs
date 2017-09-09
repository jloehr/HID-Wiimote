/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	WiimoteDevice.cs

Abstract:
	Instance of a single Wii Remote Device used for data binding on UI

*/
using System;
using System.ComponentModel;
using System.Threading.Tasks;

namespace HIDWiimote.ControlCenter.Control_Center
{
    public class WiimoteDevice : INotifyPropertyChanged
    {
        public class Option<Type> : INotifyPropertyChanged
        {
            private Type _Value;
            private bool _UIEnabled = true;
            private Func<Type, bool> InterfaceDelegate;

            public event PropertyChangedEventHandler PropertyChanged;

            public Option(Func<Type, bool> InterfaceDelegate)
            {
                this.InterfaceDelegate = InterfaceDelegate;
            }

            public Type Value
            {
                get { return _Value; }
                set { ChangeValue(value); }
            }

            public bool UIEnabled
            {
                get { return _UIEnabled; }
                protected set { _UIEnabled = value; OnPropertyChanged("UIEnabled"); }
            }

            public void SetValue(Type Value)
            {
                _Value = Value;
                OnPropertyChanged("Value");
            }

            protected void ChangeValue(Type RequestedValue)
            {
                UIEnabled = false;

                Task.Factory.StartNew(() => {
                    if ((InterfaceDelegate == null) || (InterfaceDelegate(RequestedValue)))
                    {
                        SetValue(RequestedValue);
                    }

                    UIEnabled = true;
                });
            }

            protected void OnPropertyChanged(string PropertyName)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(PropertyName));
            }
        }

        private UserModeLib.IWiimoteDeviceInterface DeviceInterface;
        private bool _Initilized = false;

        private UserModeLib.Extension _Extension = UserModeLib.Extension.None;
        private byte _BatteryLevel = 0;
        private bool[] _LEDState = { false, false, false, false };

        public Option<UserModeLib.DriverMode> Mode { get; set; }
        public Option<bool> EnableWiimoteXAxisAccelerometer { get; set; }
        public Option<bool> EnableWiimoteYAxisAccelerometer { get; set; }
        public Option<bool> SwapMouseButtons { get; set; }
        public Option<bool> SwapTriggerAndShoulder { get; set; }
        public Option<bool> SplitTriggerAxis { get; set; }
        public Option<bool> MapTriggerAsAxis { get; set; }
        public Option<bool> MapTriggerAsButtons { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;
        public event EventHandler Disconneted;
        
        public WiimoteDevice(UserModeLib.IWiimoteDeviceInterface DeviceInterface)
        {
            this.DeviceInterface = DeviceInterface;

            DeviceInterface.StatusUpdate += OnStatusUpdate;
            DeviceInterface.DeviceRemoved += OnDeviceRemoved;

            Mode = new Option<UserModeLib.DriverMode>(DeviceInterface.SetDriverMode);

            EnableWiimoteXAxisAccelerometer = new Option<bool>(DeviceInterface.SetEnableWiimoteXAxisAccelerometer);
            EnableWiimoteYAxisAccelerometer = new Option<bool>(DeviceInterface.SetEnableWiimoteYAxisAccelerometer);
            SwapMouseButtons = new Option<bool>(DeviceInterface.SetSwapMouseButtons);
            SwapTriggerAndShoulder = new Option<bool>(DeviceInterface.SetSwapTriggerAndShoulder);
            SplitTriggerAxis = new Option<bool>(DeviceInterface.SetSplitTrigger);
            MapTriggerAsAxis = new Option<bool>(DeviceInterface.SetMapTriggerAsAxis);
            MapTriggerAsButtons = new Option<bool>(DeviceInterface.SetMapTriggerAsButtons);

            EnableWiimoteXAxisAccelerometer.PropertyChanged += OnEnableAccelerometerAxisChanged;
            EnableWiimoteYAxisAccelerometer.PropertyChanged += OnEnableAccelerometerAxisChanged;
        }

        public UserModeLib.Extension Extension
        {
            get { return _Extension; }
            set
            {
                _Extension = value;
                OnPropertyChanged("Extension");
            }
        }

        public byte BatteryLevel
        {
            get { return (byte)(0x00FF - _BatteryLevel); }
            set
            {
                _BatteryLevel = value;
                OnPropertyChanged("BatteryLevel");
            }
        }

        public bool[] LEDState
        {
            get { return _LEDState; }
            set
            {
                _LEDState = value;
                OnPropertyChanged("LEDState");
            }
        }

        public bool EnableAccelerometers
        {
            get { return EnableWiimoteXAxisAccelerometer.Value || EnableWiimoteYAxisAccelerometer.Value; }
            set
            {
                EnableWiimoteXAxisAccelerometer.Value = value;
                EnableWiimoteYAxisAccelerometer.Value = value;
                OnPropertyChanged("EnableAccelerometers");
            }
        }

        private void OnEnableAccelerometerAxisChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "Value")
            {
                OnPropertyChanged("EnableAccelerometers");
            }
        }

        public bool Initilized
        {
            get { return _Initilized; }
            set
            {
                _Initilized = value;
                OnPropertyChanged("Initilized");
            }
        }

        protected void OnPropertyChanged(string PropertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(PropertyName));
        }

        public void Disconnect()
        {
            DeviceInterface.Disconnect();
        }

        public void Initilize()
        {
            Task.Factory.StartNew(InitilizeAction);
        }

        protected void InitilizeAction()
        {
            UserModeLib.State InitinalState = DeviceInterface.Initialize();

            if(InitinalState == null)
            {
                DeviceInterface.Disconnect();
                return;
            }

            ApplyState(InitinalState);
            Initilized = true;
        }

        private void OnDeviceRemoved(object sender, System.EventArgs e)
        {
            DeviceInterface.StatusUpdate -= OnStatusUpdate;
            DeviceInterface.DeviceRemoved -= OnDeviceRemoved;

            Disconneted?.Invoke(this, null);
        }

        private void OnStatusUpdate(object sender, UserModeLib.Status StatusUpdate)
        {
            ApplyStatus(StatusUpdate);
        }

        protected void ApplyState(UserModeLib.State State)
        {
            Mode.SetValue(State.Mode);

            EnableWiimoteXAxisAccelerometer.SetValue(State.EnableWiimoteXAxisAccelerometer);
            EnableWiimoteYAxisAccelerometer.SetValue(State.EnableWiimoteYAxisAccelerometer);
            SwapMouseButtons.SetValue(State.SwapMouseButtons);
            SwapTriggerAndShoulder.SetValue(State.SwapTriggerAndShoulder);
            SplitTriggerAxis.SetValue(State.SplitTrigger);
            MapTriggerAsAxis.SetValue(State.MapTriggerAsAxis);
            MapTriggerAsButtons.SetValue(State.MapTriggerAsButtons);

            ApplyStatus(State.Status);
        }

        protected void ApplyStatus(UserModeLib.Status Status)
        {
            Extension = Status.Extension;
            BatteryLevel = Status.BatteryLevel;
            LEDState = Status.LEDState;
        } 
    }
}

using System;
using System.ComponentModel;
using System.Threading.Tasks;

namespace HIDWiimote.ControlCenter.Control_Center
{
    public class WiimoteDevice : INotifyPropertyChanged
    {
        private UserModeLib.Extension _Extension = UserModeLib.Extension.None;
        private UserModeLib.DriverMode _Mode = UserModeLib.DriverMode.Gamepad;
        private bool _ModeUIEnabled = true;
        private byte _BatteryLevel = 0;
        private bool[] _LEDState = { false, false, false, false };
        
        private bool _XAxisEnabled = false;
        private bool _XAxisUIEnabled = true;
        private bool _YAxisEnabled = false;
        private bool _YAxisUIEnabled = true;
        private bool _MouseButtonsSwitched = false;
        private bool _MouseButtonsSwitchedUIEnabled = true;
        private bool _TriggerAndShoulderSwitched = false;
        private bool _TriggerAndShoulderSwitchedUIEnabled = true;
        private bool _TriggerSplit = false;
        private bool _TriggerSplitUIEnabled = true;

        private UserModeLib.WiimoteDeviceInterface DeviceInterface;
        private bool _Initilized = false;

        public event PropertyChangedEventHandler PropertyChanged;
        public event EventHandler Disconneted;
        
        public WiimoteDevice(UserModeLib.WiimoteDeviceInterface DeviceInterface)
        {
            this.DeviceInterface = DeviceInterface;

            DeviceInterface.StatusUpdate += OnStatusUpdate;
            DeviceInterface.DeviceRemoved += OnDeviceRemoved;
        }

        public WiimoteDevice(UserModeLib.Extension Extension, UserModeLib.DriverMode Mode, byte BatteryLevel, bool[] LEDState)
        {
            this.Extension = Extension;
            this.Mode = Mode;
            this.BatteryLevel = BatteryLevel;
            this.LEDState = LEDState;

            this.Initilized = true;
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

        public UserModeLib.DriverMode Mode
        {
            get { return _Mode; }
            set
            {
                ChangeSetting<UserModeLib.DriverMode>(value, SetMode, SetModeUIEnabled, DummyDelegate<UserModeLib.DriverMode>);
            }
        }

        private void SetMode(UserModeLib.DriverMode Value)
        {
            _Mode = Value;
            OnPropertyChanged("Mode");
        }

        public bool ModeUIEnabled
        {
            get { return _ModeUIEnabled; }
        }

        private void SetModeUIEnabled(bool Value)
        {
            _ModeUIEnabled = Value;
            OnPropertyChanged("ModeUIEnabled");
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

        public bool AccelerometersEnabled
        {
            get { return XAxisEnabled || YAxisEnabled; }
            set
            {
                XAxisEnabled = value;
                YAxisEnabled = value;
            }
        }

        public bool XAxisEnabled
        {
            get { return _XAxisEnabled; }
            set
            {
                ChangeSetting<bool>(value, SetXAxisEnabled, SetXAxisUIEnabled, DummyDelegate<bool>);
            }
        }

        private void SetXAxisEnabled(bool Value)
        {
            _XAxisEnabled = Value;
            OnPropertyChanged("XAxisEnabled");
            OnPropertyChanged("AccelerometersEnabled");
        }

        public bool XAxisUIEnabled
        {
            get { return _XAxisUIEnabled; }
        }

        private void SetXAxisUIEnabled(bool Value)
        {
            _XAxisUIEnabled = Value;
            OnPropertyChanged("XAxisUIEnabled");
        }

        public bool YAxisEnabled
        {
            get { return _YAxisEnabled; }
            set
            {
                ChangeSetting<bool>(value, SetYAxisEnabled, SetYAxisUIEnabled, DummyDelegate<bool>);
            }
        }

        private void SetYAxisEnabled(bool Value)
        {
            _YAxisEnabled = Value;
            OnPropertyChanged("YAxisEnabled");
            OnPropertyChanged("AccelerometersEnabled");
        }

        public bool YAxisUIEnabled
        {
            get { return _YAxisUIEnabled; }
        }

        private void SetYAxisUIEnabled(bool Value)
        {
            _YAxisUIEnabled = Value;
            OnPropertyChanged("YAxisUIEnabled");
        }

        public bool MouseButtonsSwitched
        {
            get { return _MouseButtonsSwitched; }
            set
            {
                ChangeSetting<bool>(value, SetMouseButtonsSwitched, SetMouseButtonsSwitchedUIEnabled, DummyDelegate<bool>);
            }
        }

        private void SetMouseButtonsSwitched(bool Value)
        {
            _MouseButtonsSwitched = Value;
            OnPropertyChanged("MouseButtonsSwitched");
        }

        public bool MouseButtonsSwitchedUIEnabled
        {
            get { return _MouseButtonsSwitchedUIEnabled; }
        }

        private void SetMouseButtonsSwitchedUIEnabled(bool Value)
        {
            _MouseButtonsSwitchedUIEnabled = Value;
            OnPropertyChanged("MouseButtonsSwitchedUIEnabled");
        }

        public bool TriggerAndShoulderSwitched
        {
            get { return _TriggerAndShoulderSwitched; }
            set
            {
                ChangeSetting<bool>(value, SetTriggerAndShoulderSwitched, SetTriggerAndShoulderSwitchedUIEnabled, DummyDelegate<bool>);
            }
        }

        private void SetTriggerAndShoulderSwitched(bool Value)
        {
            _TriggerAndShoulderSwitched = Value;
            OnPropertyChanged("TriggerAndShoulderSwitched");
        }

        public bool TriggerAndShoulderSwitchedUIEnabled
        {
            get { return _TriggerAndShoulderSwitchedUIEnabled; }
        }

        private void SetTriggerAndShoulderSwitchedUIEnabled(bool Value)
        {
            _TriggerAndShoulderSwitchedUIEnabled = Value;
            OnPropertyChanged("TriggerAndShoulderSwitchedUIEnabled");
        }

        public bool TriggerSplit
        {
            get { return _TriggerSplit; }
            set
            {
                ChangeSetting<bool>(value, SetTriggerSplit, SetTriggerSplitUIEnabled, DummyDelegate<bool>);
            }
        }

        private void SetTriggerSplit(bool Value)
        {
            _TriggerSplit = Value;
            OnPropertyChanged("TriggerSplit");
        }

        public bool TriggerSplitUIEnabled
        {
            get { return _TriggerSplitUIEnabled; }
        }

        private void SetTriggerSplitUIEnabled(bool Value)
        {
            _TriggerSplitUIEnabled = Value;
            OnPropertyChanged("TriggerSplitUIEnabled");
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
            PropertyChangedEventHandler Handler = PropertyChanged;
            if (Handler != null)
            {
                Handler(this, new PropertyChangedEventArgs(PropertyName));
            }
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

            if(Disconneted != null)
            {
                Disconneted(this, null);
            }
        }

        private void OnStatusUpdate(object sender, UserModeLib.Status StatusUpdate)
        {
            ApplyStatus(StatusUpdate);
        }

        protected void ApplyState(UserModeLib.State State)
        {
            SetMode(State.Mode);
            SetXAxisEnabled(State.XAxisEnabled);
            SetYAxisEnabled(State.YAxisEnabled);
            SetMouseButtonsSwitched(State.MouseButtonsSwitched);
            SetTriggerAndShoulderSwitched(State.TriggerAndShoulderSwitched);
            SetTriggerSplit(State.TriggerSplit);           

            ApplyStatus(State.Status);
        }

        protected void ApplyStatus(UserModeLib.Status Status)
        {
            Extension = Status.Extension;
            BatteryLevel = Status.BatteryLevel;
            LEDState = Status.LEDState;
        }

        
        protected void ChangeSetting<T>(T RequestedValue, Action<T> ValueSetter, Action<bool> UIEnableSetter, Func<T, T> InterfaceDelegate)
        {
            UIEnableSetter(false);

            Task.Factory.StartNew(() => {
                T ActualNewValue = InterfaceDelegate(RequestedValue);

                ValueSetter(ActualNewValue);
                UIEnableSetter(true);
            });
        }

        protected T DummyDelegate<T>(T RequestedValue)
        {
            System.Threading.Thread.Sleep(1000);
            return RequestedValue;
        }
    }
}

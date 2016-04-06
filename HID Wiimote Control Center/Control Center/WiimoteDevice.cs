using System.ComponentModel;
using System.Threading.Tasks;

namespace HIDWiimote.ControlCenter.Control_Center
{
    public class WiimoteDevice : INotifyPropertyChanged
    {
        private UserModeLib.Extension _Extension = UserModeLib.Extension.None;
        private UserModeLib.DriverMode _Mode = UserModeLib.DriverMode.Gamepad;
        private byte _BatteryLevel = 0;
        private bool[] _LEDState = { false, false, false, false };

        private bool _AccelerometersEnabled = true;
        private bool _XAxisEnabled = false;
        private bool _YAxisEnabled = false;
        private bool _MouseButtonsSwitched = false;
        private bool _TriggerAndShoulderSwitched = false;
        private bool _TriggerSplit = false;

        private UserModeLib.WiimoteDeviceInterface DeviceInterface;
        private bool _Initilized = false;

        public event PropertyChangedEventHandler PropertyChanged;
        
        public WiimoteDevice(UserModeLib.WiimoteDeviceInterface DeviceInterface)
        {
            this.DeviceInterface = DeviceInterface;

            Task.Factory.StartNew(Initilize);
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
                _Mode = value;
                OnPropertyChanged("Mode");
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

        public bool AccelerometersEnabled
        {
            get { return _AccelerometersEnabled; }
            set
            {
                _AccelerometersEnabled = value;
                OnPropertyChanged("AccelerometersEnabled");
            }
        }

        public bool XAxisEnabled
        {
            get { return _XAxisEnabled; }
            set
            {
                _XAxisEnabled = value;
                OnPropertyChanged("XAxisEnabled");
            }
        }

        public bool YAxisEnabled
        {
            get { return _YAxisEnabled; }
            set
            {
                _YAxisEnabled = value;
                OnPropertyChanged("YAxisEnabled");
            }
        }

        public bool MouseButtonsSwitched
        {
            get { return _MouseButtonsSwitched; }
            set
            {
                _MouseButtonsSwitched = value;
                OnPropertyChanged("MouseButtonsSwitched");
            }
        }

        public bool TriggerAndShoulderSwitched
        {
            get { return _TriggerAndShoulderSwitched; }
            set
            {
                _TriggerAndShoulderSwitched = value;
                OnPropertyChanged("TriggerAndShoulderSwitched");
            }
        }

        public bool TriggerSplit
        {
            get { return _TriggerSplit; }
            set
            {
                _TriggerSplit = value;
                OnPropertyChanged("TriggerSplit");
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
            PropertyChangedEventHandler Handler = PropertyChanged;
            if (Handler != null)
            {
                Handler(this, new PropertyChangedEventArgs(PropertyName));
            }
        }

        protected void Initilize()
        {
            UserModeLib.State InitinalState = DeviceInterface.Initialize();

            if(InitinalState == null)
            {
                // Error
                // Call Disconnect
                return;
            }

            ApplyState(InitinalState);
            Initilized = true;
        }       

        protected void ApplyState(UserModeLib.State State)
        {
            Mode = State.Mode;
            XAxisEnabled = State.XAxisEnabled;
            YAxisEnabled = State.YAxisEnabled;
            MouseButtonsSwitched = State.MouseButtonsSwitched;
            TriggerAndShoulderSwitched = State.TriggerAndShoulderSwitched;
            TriggerSplit = State.TriggerSplit;           

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

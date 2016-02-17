using System.ComponentModel;

namespace HID_Wiimote_Control_Center.Control_Center
{
    public class WiimoteDevice : INotifyPropertyChanged
    {
        public enum ExtensionType { None, Nunchuck, BalanceBoard, ClassicController, WiiUProController, GuitarHero };
        public enum ModeType { Gamepad, PassThrough, GamepadAndIRMouse, IRMouse, DPadMouse };

        private ExtensionType _Extension;
        private ModeType _Mode;
        private byte _BatteryLevel;
        private bool[] _LEDState;

        private bool _AccelerometersEnabled;
        private bool _XAxisEnabled;
        private bool _YAxisEnabled;
        private bool _MouseButtonsSwitched;
        private bool _TriggerAndShoulderSwitched;
        private bool _TriggerSplit;

        public event PropertyChangedEventHandler PropertyChanged;
        
        public WiimoteDevice(ExtensionType Extension, ModeType Mode, byte BatteryLevel, bool[] LEDState)
        {
            this.Extension = Extension;
            this.Mode = Mode;
            this.BatteryLevel = BatteryLevel;
            this.LEDState = LEDState;
        }

        public ExtensionType Extension
        {
            get { return _Extension; }
            set
            {
                _Extension = value;
                OnPropertyChanged("Extension");
            }
        }

        public ModeType Mode
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

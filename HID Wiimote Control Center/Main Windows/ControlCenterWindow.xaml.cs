using HIDWiimote.ControlCenter.Control_Center;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace HIDWiimote.ControlCenter.Main_Windows
{
    /// <summary>
    /// Interaction logic for ControlCenterWindow.xaml
    /// </summary>
    public partial class ControlCenterWindow : Window
    {
        List<WiimoteDevice> ConnectedWiimoteDevices = new List<WiimoteDevice>();
        HIDWiimote.UserModeLib.WiimoteDeviceInterfaceDiscoverer DeviceInterfaceDiscoverer;

        public ControlCenterWindow()
        {
            InitializeComponent();

            /*
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.None, UserModeLib.DriverMode.IRMouse, 0x3F, new bool[4] { true, true, true, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.None, UserModeLib.DriverMode.DPadMouse, 0xFF, new bool[4] { true, false, true, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.Nunchuck, UserModeLib.DriverMode.GamepadAndIRMouse, 0x1A, new bool[4] { true, true, true, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.WiiUProController, UserModeLib.DriverMode.Gamepad, 0x8C, new bool[4] { true, true, false, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.BalanceBoard, UserModeLib.DriverMode.Gamepad, 0x45, new bool[4] { true, true, false, false }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.ClassicController, UserModeLib.DriverMode.Gamepad, 0x9A, new bool[4] { false, true, true, false }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(UserModeLib.Extension.Guitar, UserModeLib.DriverMode.Gamepad, 0x21, new bool[4] { false, false, true, true }));
            */
        }
        
        private void OnInitialized(object sender, System.EventArgs e)
        {
            ConnectedDevicesListBox.ItemsSource = ConnectedWiimoteDevices;

            DeviceInterfaceDiscoverer = new UserModeLib.WiimoteDeviceInterfaceDiscoverer();
            DeviceInterfaceDiscoverer.NewWiimoteDeviceInterfaceFound += OnNewWiimoteDeviceInterfaceFound;
            DeviceInterfaceDiscoverer.Start();
        }

        private void OnNewWiimoteDeviceInterfaceFound(object sender, UserModeLib.WiimoteDeviceInterface NewInterface)
        {
            ConnectedWiimoteDevices.Add(new WiimoteDevice(NewInterface));
        }

        private void OnInstallerClick(object sender, RoutedEventArgs e)
        {
            App.ChangeMainWindow(new InstallerWindow(), this);
        }

        private void OnAboutClick(object sender, RoutedEventArgs e)
        {
            Secondary_Windows.About AboutDialog = new Secondary_Windows.About();
            AboutDialog.Owner = this;

            AboutDialog.ShowDialog();
        }

        private static Dictionary<UserModeLib.DriverMode, string> WiimoteDeviceModeTypeDescriptionsDictionary = new Dictionary<UserModeLib.DriverMode, string>()
        {
            { UserModeLib.DriverMode.Gamepad, HIDWiimote.ControlCenter.Properties.ControlCenter.ModeString_Gamepad },
            { UserModeLib.DriverMode.PassThrough, HIDWiimote.ControlCenter.Properties.ControlCenter.ModeString_PassThrough },
            { UserModeLib.DriverMode.GamepadAndIRMouse, HIDWiimote.ControlCenter.Properties.ControlCenter.ModeString_GamepadAndIRMouse },
            { UserModeLib.DriverMode.IRMouse, HIDWiimote.ControlCenter.Properties.ControlCenter.ModeString_IRMouse },
            { UserModeLib.DriverMode.DPadMouse, HIDWiimote.ControlCenter.Properties.ControlCenter.ModeString_DPadMouse }
        };

        public Dictionary<UserModeLib.DriverMode, string> WiimoteDeviceModeTypeDescriptions
        {
            get
            {
                return WiimoteDeviceModeTypeDescriptionsDictionary;
            }
        }

        private void OnClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if(DeviceInterfaceDiscoverer != null)
            {
                DeviceInterfaceDiscoverer.Stop();
            }

            // Remove/Close all Devices
        }
    }

    public class ExtensionToStringConverter : IValueConverter
    {
        public string WiimoteString { get; set; }
        public string NunchuckString { get; set; }
        public string ClassicControllerString { get; set; }
        public string WiiUProControllerString { get; set; }
        public string BalanceBoardString { get; set; }
        public string GuitarHeroString { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            UserModeLib.Extension Extension = (UserModeLib.Extension)value;

            switch (Extension)
            {
                case UserModeLib.Extension.None:
                    return WiimoteString;
                case UserModeLib.Extension.Nunchuck:
                    return NunchuckString;
                case UserModeLib.Extension.BalanceBoard:
                    return BalanceBoardString;
                case UserModeLib.Extension.ClassicController:
                    return ClassicControllerString;
                case UserModeLib.Extension.WiiUProController:
                    return WiiUProControllerString;
                case UserModeLib.Extension.Guitar:
                    return GuitarHeroString;
                default:
                    return "Error";
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class ModeToStringConverter : IValueConverter
    {
        public string GamepadString { get; set; }
        public string GamepadAndIRMouseString { get; set; }
        public string IRMouseString { get; set; }
        public string DPadMouseString { get; set; }
        public string PassThroughString { get; set; }


        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            UserModeLib.DriverMode Mode = (UserModeLib.DriverMode)value;

            switch (Mode)
            {
                case UserModeLib.DriverMode.Gamepad:
                    return GamepadString;
                case UserModeLib.DriverMode.PassThrough:
                    return PassThroughString;
                case UserModeLib.DriverMode.GamepadAndIRMouse:
                    return GamepadAndIRMouseString;
                case UserModeLib.DriverMode.IRMouse:
                    return IRMouseString;
                case UserModeLib.DriverMode.DPadMouse:
                    return DPadMouseString;
                default:
                    return "Error";
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}

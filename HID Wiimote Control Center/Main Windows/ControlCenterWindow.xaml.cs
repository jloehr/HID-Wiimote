using HID_Wiimote_Control_Center.Control_Center;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace HID_Wiimote_Control_Center.Main_Windows
{
    /// <summary>
    /// Interaction logic for ControlCenterWindow.xaml
    /// </summary>
    public partial class ControlCenterWindow : Window
    {
        List<WiimoteDevice> ConnectedWiimoteDevices = new List<WiimoteDevice>();

        public ControlCenterWindow()
        {
            InitializeComponent();

            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.None, WiimoteDevice.ModeType.IRMouse, 0x3F, new bool[4] { true, true, true, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.None, WiimoteDevice.ModeType.DPadMouse, 0xFF, new bool[4] { true, false, true, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.Nunchuck, WiimoteDevice.ModeType.GamepadAndIRMouse, 0x1A, new bool[4] { true, true, true, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.WiiUProController, WiimoteDevice.ModeType.Gamepad, 0x8C, new bool[4] { true, true, false, true }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.BalanceBoard, WiimoteDevice.ModeType.Gamepad, 0x45, new bool[4] { true, true, false, false }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.ClassicController, WiimoteDevice.ModeType.Gamepad, 0x9A, new bool[4] { false, true, true, false }));
            ConnectedWiimoteDevices.Add(new WiimoteDevice(WiimoteDevice.ExtensionType.GuitarHero, WiimoteDevice.ModeType.Gamepad, 0x21, new bool[4] { false, false, true, true }));
        }
        
        private void OnInitialized(object sender, System.EventArgs e)
        {
            ConnectedDevicesListBox.ItemsSource = ConnectedWiimoteDevices;
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

        private static Dictionary<WiimoteDevice.ModeType, string> WiimoteDeviceModeTypeDescriptionsDictionary = new Dictionary<WiimoteDevice.ModeType, string>()
        {
            { WiimoteDevice.ModeType.Gamepad, HID_Wiimote_Control_Center.Properties.ControlCenter.ModeString_Gamepad },
            { WiimoteDevice.ModeType.PassThrough, HID_Wiimote_Control_Center.Properties.ControlCenter.ModeString_PassThrough },
            { WiimoteDevice.ModeType.GamepadAndIRMouse, HID_Wiimote_Control_Center.Properties.ControlCenter.ModeString_GamepadAndIRMouse },
            { WiimoteDevice.ModeType.IRMouse, HID_Wiimote_Control_Center.Properties.ControlCenter.ModeString_IRMouse },
            { WiimoteDevice.ModeType.DPadMouse, HID_Wiimote_Control_Center.Properties.ControlCenter.ModeString_DPadMouse }
        };

        public Dictionary<WiimoteDevice.ModeType, string> WiimoteDeviceModeTypeDescriptions
        {
            get
            {
                return WiimoteDeviceModeTypeDescriptionsDictionary;
            }
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
            WiimoteDevice.ExtensionType Extension = (WiimoteDevice.ExtensionType)value;

            switch (Extension)
            {
                case WiimoteDevice.ExtensionType.None:
                    return WiimoteString;
                case WiimoteDevice.ExtensionType.Nunchuck:
                    return NunchuckString;
                case WiimoteDevice.ExtensionType.BalanceBoard:
                    return BalanceBoardString;
                case WiimoteDevice.ExtensionType.ClassicController:
                    return ClassicControllerString;
                case WiimoteDevice.ExtensionType.WiiUProController:
                    return WiiUProControllerString;
                case WiimoteDevice.ExtensionType.GuitarHero:
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
            WiimoteDevice.ModeType Mode = (WiimoteDevice.ModeType)value;

            switch (Mode)
            {
                case WiimoteDevice.ModeType.Gamepad:
                    return GamepadString;
                case WiimoteDevice.ModeType.PassThrough:
                    return PassThroughString;
                case WiimoteDevice.ModeType.GamepadAndIRMouse:
                    return GamepadAndIRMouseString;
                case WiimoteDevice.ModeType.IRMouse:
                    return IRMouseString;
                case WiimoteDevice.ModeType.DPadMouse:
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

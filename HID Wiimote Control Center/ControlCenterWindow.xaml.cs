using System.Windows;

namespace HID_Wiimote_Control_Center
{
    /// <summary>
    /// Interaction logic for ControlCenterWindow.xaml
    /// </summary>
    public partial class ControlCenterWindow : Window
    {
        public ControlCenterWindow()
        {
            InitializeComponent();
        }
        
        private void OnInstallerClick(object sender, RoutedEventArgs e)
        {
            App.ChangeMainWindow(new InstallerWindow(), this);
        }
    }
}

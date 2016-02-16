using System.Windows;

namespace HID_Wiimote_Control_Center.Main_Windows
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

        private void OnAboutClick(object sender, RoutedEventArgs e)
        {
            Secondary_Windows.About AboutDialog = new Secondary_Windows.About();
            AboutDialog.Owner = this;

            AboutDialog.ShowDialog();
        }
    }
}

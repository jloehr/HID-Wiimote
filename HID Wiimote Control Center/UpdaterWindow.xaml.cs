using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
        public UpdaterWindow()
        {
            InitializeComponent();
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
    }
}

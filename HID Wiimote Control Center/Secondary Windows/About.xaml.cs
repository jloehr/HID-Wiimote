/*

Copyright (C) 2017 Julian Löhr
All rights reserved.

Filename:
	About.xaml.cs

Abstract:
	Window with some additional info

*/
using System.Diagnostics;
using System.Windows;

namespace HIDWiimote.ControlCenter.Secondary_Windows
{
    /// <summary>
    /// Interaction logic for About.xaml
    /// </summary>
    public partial class About : Window
    {
        public About()
        {
            InitializeComponent();
        }
        private void OnInitialized(object sender, System.EventArgs e)
        {
            DeviceDriverHeader.Content += VersionStrings.DeviceDriverVersion;
            ControlCenterHeader.Content += VersionStrings.ControlCenterVersion;
        }

        private void HyperlinkRequestNavigate(object sender, System.Windows.Navigation.RequestNavigateEventArgs e)
        {
            Process.Start(e.Uri.ToString());
        }

        private void OnCloseClick(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}

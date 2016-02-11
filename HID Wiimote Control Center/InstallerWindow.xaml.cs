using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
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
    /// Interaction logic for InstallerWindow.xaml
    /// </summary>
    public partial class InstallerWindow : Window
    {
        List<InstallerAction> ActionList = new List<InstallerAction>();

        public InstallerWindow()
        {
            ActionList.Add(new InstallerAction(
                HID_Wiimote_Control_Center.Properties.Installer.TestMode_Title,
                false,
                HID_Wiimote_Control_Center.Properties.Installer.TestMode_Description,
                HID_Wiimote_Control_Center.Properties.Installer.TestMode_SmallDescription,
                HID_Wiimote_Control_Center.Properties.Installer.TestMode_RedNote,
                HID_Wiimote_Control_Center.Properties.Installer.ButtonDisable,
                HID_Wiimote_Control_Center.Properties.Installer.ButtonEnable
                ));

            ActionList.Add(new InstallerAction(
                HID_Wiimote_Control_Center.Properties.Installer.Certificate_Title,
                false,
                HID_Wiimote_Control_Center.Properties.Installer.Certificate_Description,
                HID_Wiimote_Control_Center.Properties.Installer.Certificate_SmallDescription,
                string.Empty,
                HID_Wiimote_Control_Center.Properties.Installer.ButtonUninstall,
                HID_Wiimote_Control_Center.Properties.Installer.ButtonInstall
                ));

            ActionList.Add(new InstallerAction(
                HID_Wiimote_Control_Center.Properties.Installer.DriverPackage_Title,
                true,
                HID_Wiimote_Control_Center.Properties.Installer.DriverPackage_Description,
                HID_Wiimote_Control_Center.Properties.Installer.DriverPackage_SmallDescription,
                string.Empty,
                HID_Wiimote_Control_Center.Properties.Installer.ButtonUninstall,
                HID_Wiimote_Control_Center.Properties.Installer.ButtonInstall
                ));

            InitializeComponent();
        }
        
        private void OnInitialized(object sender, EventArgs e)
        {
            ActionPanel.ItemsSource = ActionList;
        }

        private void InstallerActionButtonClick(object sender, RoutedEventArgs e)
        {
            FrameworkElement Button = sender as FrameworkElement;

            if(Button != null)
            {
                InstallerAction Action = Button.DataContext as InstallerAction;
                if(Action != null)
                {
                    Action.ButtonClicked();
                }
            }

        }

        private void CloseButtonClick(object sender, RoutedEventArgs e)
        {
            if (AllRequirementsFulfilled())
            {
                App.ChangeMainWindow(new ControlCenterWindow(), this);
            }
            else
            {
                Close();
            }

        }

        private bool AllRequirementsFulfilled()
        {
            foreach (InstallerAction Action in ActionList)
            {
                if (Action.Required)
                {
                    if(!Action.IsGood)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
    }

    public class InstallerAction : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private string _Title;
        private bool _Required;
        private string _Description;
        private string _SmallDescription;
        private string _RedNote;
        private bool _ShowRedNote;

        private bool _IsGood;
        private string GoodButtonText;
        private string BadButtonText;

        public InstallerAction(string Title, bool Required, string Description, string SmallDescription, string RedNote, string GoodButtonText, string BadButtonText)
        {
            this.Title = Title;
            this._Required = Required;
            this.Description = Description;
            this.SmallDescription = SmallDescription;
            this._RedNote = RedNote;
            this.GoodButtonText = GoodButtonText;
            this.BadButtonText = BadButtonText;

            this.ShowRedNote = false;
            this.IsGood = false;
        }

        public string Title
        {
            get { return _Title; }
            set
            {
                _Title = value;
                OnPropertyChanged("Title");
            }
        }

        public bool Required
        {
            get
            {
                return _Required;
            }
        }

        public string RequiredOptional
        {
            get {
                if(Required)
                {
                    return "- " + HID_Wiimote_Control_Center.Properties.Installer.ActionRequired;
                }
                else
                {
                    return "- " + HID_Wiimote_Control_Center.Properties.Installer.ActionOptional;
                }
            }
        }

        public string Description
        {
            get { return _Description; }
            set
            {
                _Description = value;
                OnPropertyChanged("Description");
            }
        }
        
        public string SmallDescription
        {
            get { return _SmallDescription; }
            set
            {
                _SmallDescription = value;
                OnPropertyChanged("SmallDescription");
            }
        }

        public string RedNote
        {
            get {
                if(ShowRedNote)
                {
                    return _RedNote;
                }
                else
                {
                    return string.Empty;
                }
            }
        }

        public bool ShowRedNote
        {
            get { return _ShowRedNote; }
            set
            {
                _ShowRedNote = value;
                OnPropertyChanged("ShowRedNote");
                OnPropertyChanged("RedNote");
            }
        }
        
        public string ButtonText
        {
            get
            {
                if (IsGood)
                {
                    return GoodButtonText;
                }
                else
                {
                    return BadButtonText;
                }
            }
        }

        public bool IsGood
        {
            get { return _IsGood; }
            set
            {
                _IsGood = value;
                OnPropertyChanged("IsGood");
                OnPropertyChanged("ButtonText");
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

        public void ButtonClicked()
        {
            IsGood = !IsGood;
            ShowRedNote = !ShowRedNote;
        }       
    }

    public class BooleanToColorConverter : IValueConverter
    {
        public SolidColorBrush TrueColor { get; set; }
        public SolidColorBrush FalseColor { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool Value = (bool)value;

            return Value ? TrueColor : FalseColor;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}

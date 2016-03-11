using System;
using System.Globalization;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

namespace HIDWiimote.ControlCenter.UserControls
{
    /// <summary>
    /// Interaction logic for LEDDisplay.xaml
    /// </summary>
    public partial class LEDDisplay : UserControl
    {
        public LEDDisplay()
        {
            InitializeComponent();
        }
    }

    public class LEDStateToColorConverter : IValueConverter
    {
        public SolidColorBrush EnabledColor { get; set; }
        public SolidColorBrush DisabledColor { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool[] LEDState = (bool[])value;
            uint LEDIndex = uint.MaxValue;

            if (!uint.TryParse((string)parameter, out LEDIndex))
            {
                return DisabledColor;
            }

            if (LEDIndex >= LEDState.Length)
            {
                return DisabledColor;
            }

            return (LEDState[LEDIndex]) ? EnabledColor : DisabledColor;
        }       

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}

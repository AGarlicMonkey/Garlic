using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;

namespace Garlic.Bulb
{
    /// <summary>
    /// A UserControl for displaying 3 component vectors
    /// </summary>
    public partial class Vector3Box : UserControl
    {
        public string Label
        {
            get { return (string)GetValue(LabelProperty); }
            set { SetValue(LabelProperty, value); }
        } 
        
        public string LeftFieldText
        {
            get { return (string)GetValue(LeftTextProperty); }
            set { SetValue(LeftTextProperty, value); }
        }
        public string LeftFieldValue
        {
            get { return (string)GetValue(LeftValueProperty); }
            set { SetValue(LeftValueProperty, value); }
        }

        public string CenterFieldText
        {
            get { return (string)GetValue(CenterTextProperty); }
            set { SetValue(CenterTextProperty, value); }
        }
        public string CenterFieldValue
        {
            get { return (string)GetValue(CenterValueProperty); }
            set { SetValue(CenterValueProperty, value); }
        }

        public string RightFieldText
        {
            get { return (string)GetValue(RightTextProperty); }
            set { SetValue(RightTextProperty, value); }
        }
        public string RightFieldValue
        {
            get { return (string)GetValue(RightValueProperty); }
            set { SetValue(RightValueProperty, value); }
        }

        public static DependencyProperty LabelProperty = DependencyProperty.Register(nameof(Label), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));

        public static DependencyProperty LeftTextProperty = DependencyProperty.Register(nameof(LeftFieldText), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));
        public static DependencyProperty LeftValueProperty = DependencyProperty.Register(nameof(LeftFieldValue), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));

        public static DependencyProperty CenterTextProperty = DependencyProperty.Register(nameof(CenterFieldText), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));
        public static DependencyProperty CenterValueProperty = DependencyProperty.Register(nameof(CenterFieldValue), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));

        public static DependencyProperty RightTextProperty = DependencyProperty.Register(nameof(RightFieldText), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));
        public static DependencyProperty RightValueProperty = DependencyProperty.Register(nameof(RightFieldValue), typeof(string), typeof(Vector3Box), new PropertyMetadata(null));

        public Vector3Box()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Allows the Enter key to update a TextBox's text binding.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnKeyUp(object sender, KeyEventArgs e) {
            if(e.Key == Key.Enter) {
                TextBox box = (TextBox)sender;
                DependencyProperty textProperty = TextBox.TextProperty;

                BindingExpression binding = BindingOperations.GetBindingExpression(box, textProperty);
                if(binding != null) {
                    binding.UpdateSource();
				}
			}
		}
	}
}
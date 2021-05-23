using System.Windows;
using System.Windows.Input;

using Membrane = garlic.membrane;

namespace Garlic.Bulb {
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window {
		public MainWindow() {
			InitializeComponent();
		}

		private void EditorViewport_Key(object sender, KeyEventArgs e) {
			var message = new Membrane.Editor_ViewportKeyEvent();
			message.key = e.Key;
			message.type = e.IsDown ? Membrane.Editor_ViewportKeyEvent.Type.Pressed : Membrane.Editor_ViewportKeyEvent.Type.Released;

			Membrane.MessageHandler.sendMessage(message);

			e.Handled = true;
		}

		private void EditorViewport_MouseButton(object sender, MouseButtonEventArgs e) {
			if(!EditorViewport.IsFocused) {
				EditorViewport.Focus();
			}

			var message = new Membrane.Editor_ViewportMouseButtonEvent();
			message.button = e.ChangedButton;
			message.state = e.ButtonState;
			message.position = e.GetPosition(EditorViewport);

			Membrane.MessageHandler.sendMessage(message);

			e.Handled = true;
		}

		//Capture mouse move on the window if the user moves their mouse out of the viewport will moving the camera
		private void Window_MouseMove(object sender, MouseEventArgs e) {
			if(EditorViewport.IsFocused) {
				var message = new Membrane.Editor_ViewportMouseMoveEvent();
				message.position = e.GetPosition(EditorViewport);

				Membrane.MessageHandler.sendMessage(message);

				e.Handled = true;
			}
		}
	}
}

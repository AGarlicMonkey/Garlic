using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;

namespace Garlic.Bulb {
    public partial class Viewport : UserControl {
        public delegate void RenderDelegate(IntPtr backBuffer);

        public object ResizeMutex { get; } = new object();

        public Size Size { get; private set; }

        private WriteableBitmap imageSource;
        private IntPtr backBuffer; //Keep our own reference to back buffer to work around thread ownership issues

        public Viewport() {
            InitializeComponent();

            SizeChanged += OnSizeChanged;

            UpdateBackBufferSize(new Size(1, 1));
        }

        public void WriteToBackBuffer(RenderDelegate writeFunction) {
            writeFunction(backBuffer);

            Application.Current.Dispatcher.BeginInvoke(DispatcherPriority.Render, (Action)(() => {
                imageSource.Lock();
                imageSource.AddDirtyRect(new Int32Rect(0, 0, (int)imageSource.Width, (int)imageSource.Height));
                imageSource.Unlock();
            }));
        }

        private void OnSizeChanged(object sender, SizeChangedEventArgs e) => UpdateBackBufferSize(e.NewSize);

        private void UpdateBackBufferSize(Size size) {
            lock (ResizeMutex) {
                Size = size;

                imageSource = new WriteableBitmap((int)size.Width, (int)size.Height, 96, 96, PixelFormats.Pbgra32, null);

                Image.Source = imageSource;
                backBuffer = imageSource.BackBuffer;
            }
        }
    }
}
using System;
using System.Linq;
using System.Threading;
using System.Windows;
using System.Windows.Threading;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.ComponentModel;

using Membrane = garlic.membrane;

namespace Garlic.Bulb {
    /// <summary>
    /// Interaction logic for Editor.xaml
    /// </summary>
    public partial class EditorApp : Application {
        private MainWindow editorWindow;
        private EditorSessionViewModel sessionViewModel;

        private Membrane.Application engineApp;

        private Thread engineThread;
        private bool exitThread = false;

        private Size size = new Size(1, 1);

        private void EditorStartup(object sender, StartupEventArgs e) {
            sessionViewModel = new EditorSessionViewModel();

            Membrane.Log.addSink((string message) => sessionViewModel.Log.LogText += message, "%v");

            //Initialise the editor window
            editorWindow = new MainWindow();

            editorWindow.DataContext = sessionViewModel;
            editorWindow.Closing += StopEngine;

            editorWindow.Show();
            MainWindow = editorWindow;

            //Initialise the engine
            engineApp = new Membrane.Application((int)size.Width, (int)size.Height);

            engineThread = new Thread(new ThreadStart(RunEngineApplication));
            engineThread.Name = "Garlic application thread";
            engineThread.Start();
        }

        private void StopEngine(object sender, CancelEventArgs e) {
            exitThread = true;
            engineThread.Join();
        }

        private void RunEngineApplication() {
            while (!exitThread) {
                if (engineApp.isRunning()) {
                    //Send any editor events to the engine
                    Membrane.MessageHandler.flushEditor();

                    //Update the application
                    engineApp.tick();

                    //Render to image
                    if (size != editorWindow.EditorViewport.Size) {
                        size = editorWindow.EditorViewport.Size;

                        engineApp.resize((int)size.Width, (int)size.Height);
                    }
                    editorWindow.EditorViewport.WriteToBackBuffer(engineApp.render);

                    //Send any engine events to the editor
                    Membrane.MessageHandler.flushEngine(Application.Current.Dispatcher);
                } else {
                    //Return to avoid calling shutdown if the app exits by itself
                    return;
                }
            }

            engineApp.shutdown();
        }
    }
}

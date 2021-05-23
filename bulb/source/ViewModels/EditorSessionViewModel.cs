using System;
using System.Windows.Input;

using Membrane = garlic.membrane;

namespace Garlic.Bulb {
	/// <summary>
	/// The View Model for the entire editor's session. Used to manage the viewmodels that make up the editor itself.
	/// </summary>
	public class EditorSessionViewModel : ViewModel {
		public ICommand LoadSceneCommand { get; }
		public ICommand SaveSceneCommand { get; }
		public ICommand PlayCommand { get; }
		public ICommand StopCommand { get; }

		public SceneViewModel Scene {
			get { return scene; }
			set {
				scene = value;
				OnPropertyChanged(nameof(Scene));
			}
		}
		private SceneViewModel scene = new SceneViewModel();

		public LogViewModel Log { get; } = new LogViewModel();

		public bool IsPlayButtonEnabled {
            get { return isPlayButtonEnabled; }
            set {
				isPlayButtonEnabled = value;
				OnPropertyChanged(nameof(IsPlayButtonEnabled));
            }
        }
		private bool isPlayButtonEnabled = true;

		public bool IsStopButtonEnabled {
			get { return isStopButtonEnabled; }
			set {
				isStopButtonEnabled = value;
				OnPropertyChanged(nameof(IsStopButtonEnabled));
			}
		}
		private bool isStopButtonEnabled = false;

		public EditorSessionViewModel() {
			//Bind to messages
			Membrane.MessageHandler.bindToMessage<Membrane.Engine_OnSceneLoaded>(OnSceneLoaded);

			//Set up commands
			LoadSceneCommand = new RelayCommand(() => { 
				Membrane.MessageHandler.sendMessage(new Membrane.Editor_LoadScene());
			});
			SaveSceneCommand = new RelayCommand(() => {
				Membrane.MessageHandler.sendMessage(new Membrane.Editor_SaveScene());
			});
			PlayCommand = new RelayCommand(Play);
			StopCommand = new RelayCommand(Stop);
		}

		private void OnSceneLoaded(Membrane.Engine_OnSceneLoaded message) {
			Scene = new SceneViewModel(message.entities);
		}

		private void Play() {
			IsPlayButtonEnabled = false;
			IsStopButtonEnabled = true;
			Membrane.MessageHandler.sendMessage(new Membrane.Editor_Play());
		}

		private void Stop() {
			IsPlayButtonEnabled = true;
			IsStopButtonEnabled = false;
			Membrane.MessageHandler.sendMessage(new Membrane.Editor_Stop());
		}
	}
}
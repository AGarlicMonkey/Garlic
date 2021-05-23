namespace Garlic.Bulb{
    public class LogViewModel : ViewModel{
        public string LogText
        {
            get { return logText; }
            set
            {
                logText = value;
                OnPropertyChanged(nameof(LogText));
            }
        }
        private string logText;
    }
}
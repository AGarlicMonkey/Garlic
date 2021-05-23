using System.Windows.Input;
using Membrane = garlic.membrane;

namespace Garlic.Bulb {
    public class ComponentMenuItemViewModel : ViewModel {
        public Membrane.ComponentType ComponentType { get; }
        public ICommand OnSelectedCommand { get; }

        public ComponentMenuItemViewModel(Membrane.ComponentType componentType, ICommand onSelectedCommand) {
            ComponentType = componentType;
            OnSelectedCommand = onSelectedCommand;
        }
    }
}
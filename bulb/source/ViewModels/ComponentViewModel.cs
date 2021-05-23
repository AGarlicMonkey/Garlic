using Membrane = garlic.membrane;

namespace Garlic.Bulb {
    /// <summary>
    /// The base viewmodel for all components
    /// </summary>
    public class ComponentViewModel : ViewModel {
        public string Name { get; }
        public Membrane.ComponentType Type { get; }

        public ComponentViewModel(string name, Membrane.ComponentType type) {
            Name = name;
            Type = type;
        }
    }
}
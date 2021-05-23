using Membrane = garlic.membrane;

namespace Garlic.Bulb {
    public class RigidBodyComponentViewModel : ComponentViewModel {
        public string Mass {
            get { return mass.ToString(); }
            set {
                float number;
                float.TryParse(value, out number);
                OnRigidBodyChanged?.Invoke(number);
            }
        }

        private float mass = 1.0f;//TEMP: Initialise to 1 - should init to what the data is

        public delegate void RigdBodyChangedHandler(float mass);
        public RigdBodyChangedHandler OnRigidBodyChanged;

        public RigidBodyComponentViewModel(string name, Membrane.ComponentType type) : base(name, type) { }

        public void Update(float mass) {
            this.mass = mass;

            OnPropertyChanged(nameof(Mass));
        }
    }
}
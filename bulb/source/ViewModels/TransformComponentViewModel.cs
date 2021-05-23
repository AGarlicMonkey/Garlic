using Membrane = garlic.membrane;

namespace Garlic.Bulb
{
    /// <summary>
    /// Viewmodel representing a Transform Component
    /// </summary>
    public class TransformComponentViewModel : ComponentViewModel
    {
        //Position
        public string PositionXValue
        {
            get { return position.x.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(new Membrane.Vector3(number, position.y, position.z), rotation, scale);
            }
        }
        public string PositionYValue
        {
            get { return position.y.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(new Membrane.Vector3(position.x, number, position.z), rotation, scale);
            }
        }
        public string PositionZValue
        {
            get { return position.z.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(new Membrane.Vector3(position.x, position.y, number), rotation, scale);
            }
        }

        //Rotation
        public string RotationXValue
        {
            get { return rotation.x.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(position, new Membrane.Vector3(number, rotation.y, rotation.z), scale);
            }
        }
        public string RotationYValue
        {
            get { return rotation.y.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(position, new Membrane.Vector3(rotation.x, number, rotation.z), scale);
            }
        }
        public string RotationZValue
        {
            get { return rotation.z.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(position, new Membrane.Vector3(rotation.x, rotation.y, number), scale);
            }
        }

        //Scale
        public string ScaleXValue
        {
            get { return scale.x.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(position, rotation, new Membrane.Vector3(number, scale.y, scale.z));
            }
        }
        public string ScaleYValue
        {
            get { return scale.y.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(position, rotation, new Membrane.Vector3(scale.x, number, scale.z));
            }
        }
        public string ScaleZValue
        {
            get { return scale.z.ToString(); }
            set
            {
                float number;
                float.TryParse(value, out number);
                OnTransformChanged?.Invoke(position, rotation, new Membrane.Vector3(scale.x, scale.y, number));
            }
        }

        private Membrane.Vector3 position = new Membrane.Vector3();
        private Membrane.Vector3 rotation = new Membrane.Vector3();
        private Membrane.Vector3 scale = new Membrane.Vector3(1, 1, 1);

        public delegate void TransformChangedHandler(Membrane.Vector3 position, Membrane.Vector3 rotation, Membrane.Vector3 scale);
        public TransformChangedHandler OnTransformChanged;

        public TransformComponentViewModel(string name, Membrane.ComponentType type) : base(name, type) { }

        public void Update(Membrane.Vector3 position, Membrane.Vector3 rotation, Membrane.Vector3 scale){
            this.position = position;
            this.rotation = rotation;
            this.scale = scale;

            OnPropertyChanged(nameof(PositionXValue));
            OnPropertyChanged(nameof(PositionYValue));
            OnPropertyChanged(nameof(PositionZValue));

            OnPropertyChanged(nameof(RotationXValue));
            OnPropertyChanged(nameof(RotationYValue));
            OnPropertyChanged(nameof(RotationZValue));

            OnPropertyChanged(nameof(ScaleXValue));
            OnPropertyChanged(nameof(ScaleYValue));
            OnPropertyChanged(nameof(ScaleZValue));
        }
    }
}
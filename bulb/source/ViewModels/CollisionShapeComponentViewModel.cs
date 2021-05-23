using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using Membrane = garlic.membrane;

namespace Garlic.Bulb {
    public class CollisionShapeComponentViewModel : ComponentViewModel {
        public enum ShapeType {
            Sphere,
            Cube
        };

        public class ShapeTypeViewModel {
            public string Name { get; }
            public ShapeType Type { get; }
            public ICommand OnSelectedCommand { get; }

            public ShapeTypeViewModel(string name, ShapeType type, ICommand onSelectedCommand) {
                Name = name;
                Type = type;
                OnSelectedCommand = onSelectedCommand;
            }
        }

        public string Radius {
            get { return radius.ToString(); }
            set {
                float.TryParse(value, out var number);
                OnSphereShapeChanged?.Invoke(number);
            }
        }
        private float radius = 1.0f;

        public string HalfExtentsX {
            get { return halfExtents.x.ToString(); }
            set {
                float.TryParse(value, out var number);
                OnCubeShapeChanged?.Invoke(new Membrane.Vector3(number, halfExtents.y, halfExtents.z));
            }
        }
        public string HalfExtentsY {
            get { return halfExtents.y.ToString(); }
            set {
                float.TryParse(value, out var number);
                OnCubeShapeChanged?.Invoke(new Membrane.Vector3(halfExtents.x, number, halfExtents.z));
            }
        }
        public string HalfExtentsZ {
            get { return halfExtents.z.ToString(); }
            set {
                float.TryParse(value, out var number);
                OnCubeShapeChanged?.Invoke(new Membrane.Vector3(halfExtents.x, halfExtents.y, number));
            }
        }
        private Membrane.Vector3 halfExtents = new Membrane.Vector3(0.5f, 0.5f, 0.5f);

        public ShapeType CurrentShapeType {
            get { return currentShapeType; }
            private set {
                currentShapeType = value;
                OnPropertyChanged(nameof(CurrentShapeType));
            }
        }
        private ShapeType currentShapeType;

        public ObservableCollection<ShapeTypeViewModel> AvailableShapes { get; } = new ObservableCollection<ShapeTypeViewModel>();

        public Visibility RadiusVisibility {
            get { return radiusVisibility; }
            private set {
                radiusVisibility = value;
                OnPropertyChanged(nameof(RadiusVisibility));
            }
        }
        private Visibility radiusVisibility = Visibility.Visible;

        public Visibility HalfExtentsVisibility {
            get { return halfExtentsVisibility; }
            private set {
                halfExtentsVisibility = value;
                OnPropertyChanged(nameof(HalfExtentsVisibility));
            }
        }
        private Visibility halfExtentsVisibility = Visibility.Visible;

        public delegate void CollisionShapeSphereChanged(float radius);
        public delegate void CollisionShapeCubeChanged(Membrane.Vector3 halfExtents);

        public CollisionShapeSphereChanged OnSphereShapeChanged;
        public CollisionShapeCubeChanged OnCubeShapeChanged;

        public CollisionShapeComponentViewModel(string name, Membrane.ComponentType type) : base(name, type) {
            //TODO: Hard coding as a sphere. Should be synced to what ever the component is added as
            OnShapeTypeChanged(ShapeType.Sphere, false);
        }

        public void UpdateSphereShape(float radius) {
            this.radius = radius;
            OnPropertyChanged(nameof(Radius));
        }

        public void UpdateCubeShape(Membrane.Vector3 halfExtents) {
            this.halfExtents = halfExtents;
            OnPropertyChanged(nameof(HalfExtentsX));
            OnPropertyChanged(nameof(HalfExtentsY));
            OnPropertyChanged(nameof(HalfExtentsZ));
        }

        private void OnShapeTypeChanged(ShapeType newType, bool invokeDelegates = true) {
            CurrentShapeType = newType;

            switch (CurrentShapeType) {
                case ShapeType.Sphere:
                    RadiusVisibility = Visibility.Visible;
                    HalfExtentsVisibility = Visibility.Collapsed;
                    if (invokeDelegates) {
                        OnSphereShapeChanged?.Invoke(radius);
                    }
                    break;
                case ShapeType.Cube:
                    RadiusVisibility = Visibility.Collapsed;
                    HalfExtentsVisibility = Visibility.Visible;
                    if (invokeDelegates) {
                        OnCubeShapeChanged?.Invoke(halfExtents);
                    }
                    break;
            }

            //Update the available shapes to remove the selected one
            var shapeTypes = Enum.GetValues(typeof(ShapeType)).Cast<ShapeType>();

            AvailableShapes.Clear();
            foreach (ShapeType shape in shapeTypes) {
                if (shape != CurrentShapeType) {
                    AvailableShapes.Add(new ShapeTypeViewModel($"{shape}", shape, new RelayCommand(() => OnShapeTypeChanged(shape))));
                }
            }
        }
    }
}
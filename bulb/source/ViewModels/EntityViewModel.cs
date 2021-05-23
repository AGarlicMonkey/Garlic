using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows;
using System.Windows.Input;

using Membrane = garlic.membrane;

namespace Garlic.Bulb {
    public class EntityViewModel : ViewModel {
        public uint EntityId { get; set; }

        public string Name {
            get { return name; }
            set {
                name = value;
                OnPropertyChanged(nameof(Name));

                var message = new Membrane.Editor_UpdateName();
                message.entity = EntityId;
                message.name = name;

                Membrane.MessageHandler.sendMessage(message);
            }
        }
        private string name = "New Entity";

        public ObservableCollection<ComponentViewModel> Components { get; } = new ObservableCollection<ComponentViewModel>();

        public ICommand SelectedCommand { get; }

        public delegate void SelectionHandler(EntityViewModel viewModel);
        public SelectionHandler OnSelected;

        public EntityViewModel() {
            //Bind to messages
            Membrane.MessageHandler.bindToMessage<Membrane.Engine_OnComponentCreated>(OnComponentCreated);
            Membrane.MessageHandler.bindToMessage<Membrane.Engine_OnRigidBodyChanged>(OnRigidBodyChanged);
            Membrane.MessageHandler.bindToMessage<Membrane.Engine_OnTransformChanged>(OnTransformChanged);
            Membrane.MessageHandler.bindToMessage<Membrane.Engine_OnSphereShapeChanged>(OnSphereShapeChanged);
            Membrane.MessageHandler.bindToMessage<Membrane.Engine_OnCubeShapeChanged>(OnCubeShapeChanged);

            //Set up commands
            SelectedCommand = new RelayCommand(() => OnSelected?.Invoke(this));
        }

        public EntityViewModel(List<Membrane.ComponentType> components) : this() {
            foreach (var componentType in components) {
                Components.Add(CreateComponentViewModel(componentType));
            }
        }

        public void AddComponent(Membrane.ComponentType type) {
            var message = new Membrane.Editor_CreateComponent();
            message.entity = EntityId;
            message.componentType = type;

            Membrane.MessageHandler.sendMessage(message);
        }

        private void OnComponentCreated(Membrane.Engine_OnComponentCreated message) {
            if (EntityId == message.entity) {
                Components.Add(CreateComponentViewModel(message.componentType));
            }
        }

        private ComponentViewModel CreateComponentViewModel(Membrane.ComponentType type) {
            ComponentViewModel componentVm;

            switch (type) {
                case Membrane.ComponentType.Transform: {
                    var transformComp = new TransformComponentViewModel($"{type}", type);
                    transformComp.OnTransformChanged = UpdateTransform;

                    componentVm = transformComp;
                }
                break;
                case Membrane.ComponentType.RigidBody: {
                    var rigidBodyComp = new RigidBodyComponentViewModel($"{type}", type);
                    rigidBodyComp.OnRigidBodyChanged = UpdateRigidBody;

                    componentVm = rigidBodyComp;
                }
                break;
                case Membrane.ComponentType.CollisionShape: {
                    var collisionShapeComp = new CollisionShapeComponentViewModel($"{type}", type);
                    collisionShapeComp.OnSphereShapeChanged = UpdateSphereShape;
                    collisionShapeComp.OnCubeShapeChanged = UpdateCubeShape;

                    componentVm = collisionShapeComp;
                }
                break;
                default:
                    componentVm = new ComponentViewModel($"{type}", type);
                    break;
            }

            return componentVm;
        }

        private void UpdateTransform(Membrane.Vector3 position, Membrane.Vector3 rotation, Membrane.Vector3 scale) {
            var message = new Membrane.Editor_UpdateTransform();
            message.entity = EntityId;
            message.position = position;
            message.rotation = rotation;
            message.scale = scale;

            Membrane.MessageHandler.sendMessage(message);
        }

        private void UpdateRigidBody(float mass) {
            var message = new Membrane.Editor_UpdateRigidBody();
            message.entity = EntityId;
            message.mass = mass;

            Membrane.MessageHandler.sendMessage(message);
        }

        private void UpdateSphereShape(float radius) {
            var message = new Membrane.Editor_UpdateSphereShape();
            message.entity = EntityId;
            message.radius = radius;

            Membrane.MessageHandler.sendMessage(message);
        }

        private void UpdateCubeShape(Membrane.Vector3 halfExtents) {
            var message = new Membrane.Editor_UpdateCubeShape();
            message.entity = EntityId;
            message.halfExtents = halfExtents;

            Membrane.MessageHandler.sendMessage(message);
        }

        private void OnRigidBodyChanged(Membrane.Engine_OnRigidBodyChanged message) {
            if (EntityId == message.entity) {
                foreach (ComponentViewModel comp in Components) {
                    if (comp.GetType() == typeof(RigidBodyComponentViewModel)) {
                        var rigidBody = (RigidBodyComponentViewModel)comp;
                        rigidBody.Update(message.mass);
                        break;
                    }
                }
            }
        }

        private void OnTransformChanged(Membrane.Engine_OnTransformChanged message) {
            if (EntityId == message.entity) {
                foreach (ComponentViewModel comp in Components) {
                    if (comp.GetType() == typeof(TransformComponentViewModel)) {
                        var transform = (TransformComponentViewModel)comp;
                        transform.Update(message.position, message.rotation, message.scale);
                        break;
                    }
                }
            }
        }

        private void OnSphereShapeChanged(Membrane.Engine_OnSphereShapeChanged message) {
            if (EntityId == message.entity) {
                foreach (ComponentViewModel comp in Components) {
                    if (comp.GetType() == typeof(CollisionShapeComponentViewModel)) {
                        var collisionShape = (CollisionShapeComponentViewModel)comp;
                        collisionShape.UpdateSphereShape(message.radius);
                        break;
                    }
                }
            }

        }

        private void OnCubeShapeChanged(Membrane.Engine_OnCubeShapeChanged message) {
            if (EntityId == message.entity) {
                foreach(ComponentViewModel comp in Components) {
                    if (comp.GetType() == typeof(CollisionShapeComponentViewModel)) {
                        var collisionShape = (CollisionShapeComponentViewModel)comp;
                        collisionShape.UpdateCubeShape(message.halfExtents);
                        break;
                    }
                }
            }
        }
    }
}
using System;
using garlic.membrane;

namespace Garlic.Bulb
{
    /// <summary>
    /// The View Model for the entire editor's session. Used to manage the viewmodels that make up the editor itself.
    /// </summary>
    public class EditorSessionViewModel : ViewModel
    {
        public SceneViewModel Scene
        {
            get { return scene; }
            set
            {
                scene = value;
                OnPropertyChanged(nameof(Scene));
            }
        }
        private SceneViewModel scene;

        public LogViewModel Log { get; } = new LogViewModel();

        public delegate uint AddEntityEventHandler(); //Expecting a uint here as that's what a 
        public AddEntityEventHandler OnCreateEntity;

        public delegate void AddComponentEventHandler(uint entityId, ComponentType componentType);
        public AddComponentEventHandler OnCreateComponent;

        public EditorSessionViewModel()
        {
            Scene = new SceneViewModel();
            Scene.OnCreateEntity = CreateEntity;
        }

        private EntityViewModel CreateEntity()
        {
            var entityId = OnCreateEntity.Invoke();
            var entityVm = new EntityViewModel();

            entityVm.OnAddComponent = (componentType) => CreateComponent(entityId, componentType);

            return entityVm;
        }

        private ComponentViewModel CreateComponent(uint entityId, ComponentType componentType)
        {
            var vm = new ComponentViewModel();
            vm.Name = $"{componentType}";

            OnCreateComponent.Invoke(entityId, componentType);

            return vm;
        }
    }
}
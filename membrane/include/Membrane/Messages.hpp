#pragma once

#include "Membrane/EditorTypes.hpp"

namespace garlic::membrane {
    // clang-format off
    public ref class Editor_CreateEntity {};
    public ref class Engine_OnEntityCreated {
    public:
        System::UInt32 entity;
        System::String ^name;
    };

    public ref class Editor_CreateComponent {
    public:
        System::UInt32 entity;
        ComponentType componentType;
    };
    public ref class Engine_OnComponentCreated {
    public:
        System::UInt32 entity;
        ComponentType componentType;
    };

    public ref class Editor_UpdateTransform{
    public:
        System::UInt32 entity;

        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
    };
    public ref class Engine_OnTransformChanged{
    public:
        System::UInt32 entity;
        
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
    };

    public ref class Editor_UpdateName{
    public:
        System::UInt32 entity;

        System::String^ name;
    };

    public ref class Editor_ViewportKeyEvent{
    public:
        enum class Type{
            Pressed,
            Released,
        };

    public:
        System::Windows::Input::Key key;
        Type type;
    };

    public ref class Editor_ViewportMouseButtonEvent{
    public:
        System::Windows::Input::MouseButton button;
        System::Windows::Input::MouseButtonState state;
        System::Windows::Point position;
    };

    public ref class Editor_ViewportMouseMoveEvent{
    public:
        System::Windows::Point position;
    };

    public ref class Editor_SaveScene{};

    public ref struct Entity{
        System::UInt32 id;
        System::String ^name;
        System::Collections::Generic::List<ComponentType> ^components;
    };

    public ref class Editor_LoadScene{};
    public ref class Engine_OnSceneLoaded{
    public:
        System::Collections::Generic::List<Entity^> ^entities;
    };
    // clang-format on
}
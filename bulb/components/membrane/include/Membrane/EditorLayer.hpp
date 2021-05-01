#pragma once

#include <Clove/ECS/Entity.hpp>
#include <Clove/Layer.hpp>
#include <Clove/Rendering/Camera.hpp>

namespace garlic::membrane {
    /**
     * @brief The layer that holds all the editor time components
     * @details This will be things such as the gizmos, editor camera etc. 
     * Anything that doesn't contribute to the 'game' which will be removed during play.
     */
    class EditorLayer : public clove::Layer {
        //VARIABLES
    private:
        clove::Entity editorCamera;

        bool moveMouse{ false };
        clove::vec2i mousePos{};
        clove::vec2i prevMousePos{};
        float mouseLookYaw{};
        float mouseLookPitch{};

        //FUNCTIONS
    public:
        EditorLayer();

        void onAttach() override;
        clove::InputResponse onInputEvent(clove::InputEvent const &inputEvent) override;
        void onUpdate(clove::DeltaTime const deltaTime) override;
        void onDetach() override;
    };
}
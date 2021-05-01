#include "Membrane/EditorLayer.hpp"

#include "Membrane/MessageHandler.hpp"

#include <Clove/Application.hpp>
#include <Clove/Components/CameraComponent.hpp>
#include <Clove/Components/TransformComponent.hpp>
#include <Clove/ECS/EntityManager.hpp>
#include <Clove/Maths/MathsHelpers.hpp>
#include <Clove/ModelLoader.hpp>
#include <Clove/Surface.hpp>

namespace garlic::membrane {
    EditorLayer::EditorLayer()
        : clove::Layer{ "Editor Layer" } {
    }

    void EditorLayer::onAttach() {
        auto *const entityManager{ clove::Application::get().getEntityManager() };

        editorCamera                                                                  = entityManager->create();
        entityManager->addComponent<clove::TransformComponent>(editorCamera).position = clove::vec3f{ 0.0f, 0.0f, -10.0f };
        entityManager->addComponent<clove::CameraComponent>(editorCamera, clove::Camera{ clove::Camera::ProjectionMode::Perspective });
    }

    clove::InputResponse EditorLayer::onInputEvent(clove::InputEvent const &inputEvent) {
        if(inputEvent.eventType == clove::InputEvent::Type::Mouse) {
            auto event{ std::get<clove::Mouse::Event>(inputEvent.event) };

            if(event.getButton() == clove::MouseButton::Right) {
                if(event.getType() == clove::Mouse::Event::Type::Pressed) {
                    //Make sure prev + pos gets updated when the event happens.
                    //This fixes situations where the mouse will be up, move and then go down
                    mousePos     = event.getPos();
                    prevMousePos = event.getPos();

                    moveMouse    = true;
                    return clove::InputResponse::Consumed;
                } else {
                    moveMouse = false;
                    return clove::InputResponse::Consumed;
                }
            }

            if(event.getType() == clove::Mouse::Event::Type::Move && moveMouse) {
                mousePos = event.getPos();
                return clove::InputResponse::Consumed;
            }
        }
        return clove::InputResponse::Ignored;
    }

    void EditorLayer::onUpdate(clove::DeltaTime const deltaTime) {
        auto &keyBoard{ clove::Application::get().getSurface()->getKeyboard() };
        auto &mouse{ clove::Application::get().getSurface()->getMouse() };
        auto *const entityManager{ clove::Application::get().getEntityManager() };

        clove::vec3f pos{};
        clove::vec3f rot{};

        //Camera Movement: Keyboard
        if(keyBoard.isKeyPressed(clove::Key::W)) {
            pos.z += 1.0f;
        }
        if(keyBoard.isKeyPressed(clove::Key::S)) {
            pos.z -= 1.0f;
        }
        if(keyBoard.isKeyPressed(clove::Key::A)) {
            pos.x -= 1.0f;
        }
        if(keyBoard.isKeyPressed(clove::Key::D)) {
            pos.x += 1.0f;
        }
        if(keyBoard.isKeyPressed(clove::Key::Space) || keyBoard.isKeyPressed(clove::Key::E)) {
            pos.y += 1.0f;
        }
        if(keyBoard.isKeyPressed(clove::Key::Shift_Left) || keyBoard.isKeyPressed(clove::Key::Q)) {
            pos.y -= 1.0f;
        }

        //Camera Movement: Mouse
        if(moveMouse) {
            float constexpr speed = 10.0f;
            clove::vec2i const delta{ mousePos - prevMousePos };

            mouseLookYaw += static_cast<float>(delta.x) * speed * deltaTime;
            mouseLookPitch += static_cast<float>(delta.y) * speed * deltaTime;

            prevMousePos = mousePos;
        }

        rot.x = clove::asRadians(mouseLookPitch);
        rot.y = clove::asRadians(mouseLookYaw);

        auto &camTrans{ entityManager->getComponent<clove::TransformComponent>(editorCamera) };
        camTrans.rotation = rot;
        camTrans.position += camTrans.rotation * pos * 10.0f * deltaTime.getDeltaSeconds();
    }

    void EditorLayer::onDetach() {
        clove::Application::get().getEntityManager()->destroy(editorCamera);
    }
}
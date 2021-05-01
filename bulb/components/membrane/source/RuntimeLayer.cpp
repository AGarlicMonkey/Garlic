#include "Membrane/RuntimeLayer.hpp"

#include "Membrane/Messages.hpp"
#include "Membrane/NameComponent.hpp"

#include <Clove/Application.hpp>
#include <Clove/Components/PointLightComponent.hpp>
#include <Clove/Components/StaticModelComponent.hpp>
#include <Clove/Components/TransformComponent.hpp>
#include <Clove/ECS/EntityManager.hpp>
#include <Clove/ModelLoader.hpp>
#include <msclr/marshal_cppstd.h>

namespace garlic::membrane {
    // clang-format off
    /**
     * @brief 
     */
    private ref class RuntimeLayerMessageProxy {
        //VARIABLES
    private:
        class RuntimeLayer *layer{ nullptr };

        //FUNCTIONS
    public:
        RuntimeLayerMessageProxy(class RuntimeLayer *layer)
            : layer{ layer } {
            MessageHandler::bindToMessage(gcnew MessageSentHandler<Editor_CreateEntity ^>(this, &RuntimeLayerMessageProxy::createEntity));
            MessageHandler::bindToMessage(gcnew MessageSentHandler<Editor_CreateComponent ^>(this, &RuntimeLayerMessageProxy::createComponent));
            MessageHandler::bindToMessage(gcnew MessageSentHandler<Editor_UpdateTransform ^>(this, &RuntimeLayerMessageProxy::updateTransform));
            MessageHandler::bindToMessage(gcnew MessageSentHandler<Editor_UpdateName ^>(this, &RuntimeLayerMessageProxy::updateName));

            MessageHandler::bindToMessage(gcnew MessageSentHandler<Editor_SaveScene ^>(this, &RuntimeLayerMessageProxy::saveScene));
            MessageHandler::bindToMessage(gcnew MessageSentHandler<Editor_LoadScene ^>(this, &RuntimeLayerMessageProxy::loadScene));
        }

    private:
        void createEntity(Editor_CreateEntity ^ message) {
            layer->createEntity();
        }

        void createComponent(Editor_CreateComponent ^ message){
            layer->createComponent(message->entity, message->componentType);
        }

        void updateTransform(Editor_UpdateTransform ^ message){
            clove::vec3f pos{message->position.x, message->position.y, message->position.z};
            clove::vec3f rot{clove::asRadians(message->rotation.x), clove::asRadians(message->rotation.y), clove::asRadians(message->rotation.z)};
            clove::vec3f scale{message->scale.x, message->scale.y, message->scale.z};

            layer->updateTransform(message->entity, pos, rot, scale);
        }

        void updateName(Editor_UpdateName ^ message){
            System::String ^name{ message->name };
            layer->updateName(message->entity, msclr::interop::marshal_as<std::string>(name));
        }

        void saveScene(Editor_SaveScene ^message){
            layer->saveScene();
        }

        void loadScene(Editor_LoadScene ^message){
            layer->loadScene();
        }
    };
    // clang-format on
}

namespace garlic::membrane {
    RuntimeLayer::RuntimeLayer()
        : clove::Layer{ "Runtime Layer" }
        , currentScene{ clove::Application::get().getEntityManager(), "scene.yaml" } {
        proxy = gcnew RuntimeLayerMessageProxy(this);
    }

    void RuntimeLayer::onAttach() {
        auto lightEnt{ createEntity("Point Light") };
        createComponent(lightEnt, ComponentType::Transform);
        createComponent(lightEnt, ComponentType::PointLight);

        currentScene.getComponent<clove::TransformComponent>(lightEnt).position = clove::vec3f{ 5.0f, 0.0f, 0.0f };
    }

    void RuntimeLayer::onUpdate(clove::DeltaTime const deltaTime) {
        static float time{ 0.0f };
        time += deltaTime;

        for(auto &entity : currentScene.getKnownEntities()) {
            if(currentScene.hasComponent<clove::TransformComponent>(entity)) {
                auto const &pos{ currentScene.getComponent<clove::TransformComponent>(entity).position };
                auto const &rot{ clove::quaternionToEuler(currentScene.getComponent<clove::TransformComponent>(entity).rotation) };
                auto const &scale{ currentScene.getComponent<clove::TransformComponent>(entity).scale };

                Engine_OnTransformChanged ^ message { gcnew Engine_OnTransformChanged };
                message->entity   = entity;
                message->position = Vector3(pos.x, pos.y, pos.z);
                message->rotation = Vector3(clove::asDegrees(rot.x), clove::asDegrees(rot.y), clove::asDegrees(rot.z));
                message->scale    = Vector3(scale.x, scale.y, scale.z);
                MessageHandler::sendMessage(message);
            }
        }
    }

    void RuntimeLayer::onDetach() {
        currentScene.destroyAllEntities();
    }

    void RuntimeLayer::saveScene() {
        currentScene.save();
    }

    void RuntimeLayer::loadScene() {
        currentScene.load();

        Engine_OnSceneLoaded ^ loadMessage { gcnew Engine_OnSceneLoaded };
        loadMessage->entities = gcnew System::Collections::Generic::List<Entity ^>{};
        for(auto entity : currentScene.getKnownEntities()) {
            Entity ^ editorEntity { gcnew Entity };
            editorEntity->id         = entity;
            editorEntity->name       = gcnew System::String(currentScene.getComponent<NameComponent>(entity).name.c_str());
            editorEntity->components = gcnew System::Collections::Generic::List<ComponentType>{};

            //Add all of the component types for an entity
            if(currentScene.hasComponent<clove::TransformComponent>(entity)) {
                editorEntity->components->Add(ComponentType::Transform);
            }
            if(currentScene.hasComponent<clove::StaticModelComponent>(entity)) {
                editorEntity->components->Add(ComponentType::Mesh);
            }
            if(currentScene.hasComponent<clove::PointLightComponent>(entity)) {
                editorEntity->components->Add(ComponentType::PointLight);
            }

            loadMessage->entities->Add(editorEntity);
        }

        MessageHandler::sendMessage(loadMessage);
    }

    clove::Entity RuntimeLayer::createEntity(std::string_view name) {
        clove::Entity entity{ currentScene.createEntity() };
        currentScene.addComponent<NameComponent>(entity, std::string{ std::move(name) });

        Engine_OnEntityCreated ^ message { gcnew Engine_OnEntityCreated };
        message->entity = entity;
        message->name   = gcnew System::String(std::string{ std::begin(name), std::end(name) }.c_str());
        MessageHandler::sendMessage(message);

        return entity;
    }

    void RuntimeLayer::createComponent(clove::Entity entity, ComponentType componentType) {
        bool added{ false };
        switch(componentType) {
            case ComponentType::Transform:
                if(!currentScene.hasComponent<clove::TransformComponent>(entity)) {
                    currentScene.addComponent<clove::TransformComponent>(entity);
                    added = true;
                }
                break;
            case ComponentType::Mesh:
                if(!currentScene.hasComponent<clove::StaticModelComponent>(entity)) {
                    currentScene.addComponent<clove::StaticModelComponent>(entity, clove::ModelLoader::loadStaticModel(ASSET_DIR "/cube.obj"));
                    added = true;
                }
                break;
            case ComponentType::PointLight:
                if(!currentScene.hasComponent<clove::PointLightComponent>(entity)) {
                    currentScene.addComponent<clove::PointLightComponent>(entity);
                    added = true;
                }
                break;
        }

        if(added) {
            Engine_OnComponentCreated ^ message { gcnew Engine_OnComponentCreated };
            message->entity        = entity;
            message->componentType = componentType;
            MessageHandler::sendMessage(message);
        }
    }

    void RuntimeLayer::updateTransform(clove::Entity entity, clove::vec3f position, clove::vec3f rotation, clove::vec3f scale) {
        if(currentScene.hasComponent<clove::TransformComponent>(entity)) {
            auto &transform{ currentScene.getComponent<clove::TransformComponent>(entity) };
            transform.position = position;
            transform.rotation = rotation;
            transform.scale    = scale;
        }
    }

    void RuntimeLayer::updateName(clove::Entity entity, std::string name) {
        if(currentScene.hasComponent<NameComponent>(entity)) {
            currentScene.getComponent<NameComponent>(entity).name = std::move(name);
        }
    }
}
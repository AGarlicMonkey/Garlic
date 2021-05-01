#include "Membrane/Scene.hpp"

#include "Membrane/NameComponent.hpp"

#include <Clove/Components/PointLightComponent.hpp>
#include <Clove/Components/StaticModelComponent.hpp>
#include <Clove/Components/TransformComponent.hpp>
#include <Clove/ModelLoader.hpp>
#include <Clove/Serialisation/Node.hpp>
#include <Clove/Serialisation/Yaml.hpp>
#include <fstream>
#include <typeinfo>

using namespace garlic::clove;

namespace garlic::membrane {
    namespace {
        template<typename ComponentType>
        void serialiseComponent(serialiser::Node &entityNode, EntityManager &manager, Entity entity) {
            if(manager.hasComponent<ComponentType>(entity)) {
                auto const &comp{ manager.getComponent<ComponentType>(entity) };

                serialiser::Node componentNode{ serialise(comp) };
                componentNode["id"] = typeid(comp).hash_code();
                entityNode["components"].pushBack(componentNode);
            }
        }
    }

    Scene::Scene(EntityManager *manager, std::filesystem::path saveData)
        : manager{ manager }
        , sceneFile{ std::move(saveData) } {
    }

    Scene::~Scene() = default;

    void Scene::save() {
        serialiser::Node rootNode{};
        for(Entity entity : knownEntities) {
            serialiser::Node entityNode{};
            entityNode["id"] = entity;

            serialiseComponent<NameComponent>(entityNode, *manager, entity);
            serialiseComponent<TransformComponent>(entityNode, *manager, entity);
            serialiseComponent<StaticModelComponent>(entityNode, *manager, entity);
            serialiseComponent<PointLightComponent>(entityNode, *manager, entity);

            rootNode["entities"].pushBack(entityNode);
        }

        std::ofstream fileStream{ sceneFile, std::ios::out | std::ios::trunc };
        fileStream << emittYaml(rootNode);
    }

    void Scene::load() {
        //Only destroy known entities to avoid deleting meta ones (such as the camera)
        for(Entity entity : knownEntities) {
            manager->destroy(entity);
        }
        knownEntities.clear();

        auto loadResult{ loadYaml(sceneFile) };
        serialiser::Node rootNode{ loadResult.getValue() };

        for(auto const &entityNode : rootNode["entities"]) {
            Entity entity{ manager->create() };
            for(auto const &componentNode : entityNode["components"]) {
                if(componentNode["id"].as<size_t>() == typeid(NameComponent).hash_code()) {
                    manager->addComponent<NameComponent>(entity, componentNode.as<NameComponent>());
                } else if(componentNode["id"].as<size_t>() == typeid(TransformComponent).hash_code()) {
                    manager->addComponent<TransformComponent>(entity, componentNode.as<TransformComponent>());
                } else if(componentNode["id"].as<size_t>() == typeid(StaticModelComponent).hash_code()) {
                    manager->addComponent<StaticModelComponent>(entity, componentNode.as<StaticModelComponent>());
                } else if(componentNode["id"].as<size_t>() == typeid(PointLightComponent).hash_code()) {
                    manager->addComponent<PointLightComponent>(entity, componentNode.as<PointLightComponent>());
                }
            }

            knownEntities.push_back(entity);
        }
    }
}
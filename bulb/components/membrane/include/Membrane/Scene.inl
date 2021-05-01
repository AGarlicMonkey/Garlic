namespace garlic::membrane {
    clove::Entity Scene::createEntity() {
        clove::Entity entity{ manager->create() };
        knownEntities.push_back(entity);
        return entity;
    }

    std::vector<clove::Entity> Scene::getKnownEntities() const {
        return knownEntities;
    }

    template<typename ComponentType, typename... ConstructArgs>
    ComponentType &Scene::addComponent(clove::Entity entity, ConstructArgs &&...args) {
        return manager->addComponent<ComponentType>(entity, std::forward<ConstructArgs>(args)...);
    }

    template<typename ComponentType>
    ComponentType &Scene::getComponent(clove::Entity entity) {
        return manager->getComponent<ComponentType>(entity);
    }

    template<typename ComponentType>
    bool Scene::hasComponent(clove::Entity entity) {
        return manager->hasComponent<ComponentType>(entity);
    }

    void Scene::destroyAllEntities() {
        for(auto entity : knownEntities) {
            manager->destroy(entity);
        }
    }
}
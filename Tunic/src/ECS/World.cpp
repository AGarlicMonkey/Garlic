#include "Tunic/ECS/World.hpp"

#include "Tunic/ECS/Entity.hpp"
#include "Tunic/ECS/System.hpp"

using namespace clv;

namespace tnc::ecs{
	EntityID World::nextID = 1;

	World::World(){
		componentManager.componentAddedDelegate.bind(&World::onComponentAdded, this);
		componentManager.componentRemovedDelegate.bind(&World::onComponentRemoved, this);
	}

	World::~World() = default;

	void World::update(utl::DeltaTime deltaTime){
		CLV_PROFILE_FUNCTION();

		for(const auto& system : systems) {
			system->preUpdate(*this);
		}

		for(const auto& system : systems) {
			system->update(*this, deltaTime);
		}

		for(const auto& system : systems) {
			system->postUpdate(*this);
		}
	}

	Entity World::createEntity(){
		EntityID ID = INVALID_ENTITY_ID;

		ID = nextID++;

		activeIDs.push_back(ID);

		return { ID, this };
	}

	Entity World::cloneEntitiesComponents(EntityID ID){
		Entity clonedEntity = createEntity();

		componentManager.cloneEntitiesComponents(ID, clonedEntity.getID());

		return clonedEntity;
	}

	Entity World::getEntity(EntityID ID){
		if(ID == INVALID_ENTITY_ID){
			return {};
		} else{
			return { ID, this };
		}
	}

	void World::destroyEntity(EntityID ID){
		auto foundIDIter = std::find(activeIDs.begin(), activeIDs.end(), ID);

		if(ID == INVALID_ENTITY_ID || foundIDIter == activeIDs.end()){
			return;
		}
		componentManager.onEntityDestroyed(ID);

		activeIDs.erase(foundIDIter);
	}

	void World::destroyAllEntites() {
		for(EntityID id : activeIDs) {
			componentManager.onEntityDestroyed(id);
		}
		activeIDs.clear();
	}

	void World::onComponentAdded(ComponentInterface* component){
		std::for_each(systems.begin(), systems.end(), [component](const std::unique_ptr<System>& system){
			system->onComponentCreated(component);
		});
	}

	void World::onComponentRemoved(ComponentInterface* component){
		std::for_each(systems.begin(), systems.end(), [component](const std::unique_ptr<System>& system){
			system->onComponentDestroyed(component);
		});
	}
}

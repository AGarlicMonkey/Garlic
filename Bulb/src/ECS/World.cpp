#include "Bulb/ECS/World.hpp"

#include "Bulb/ECS/Entity.hpp"
#include "Bulb/ECS/System.hpp"

using namespace clv;

namespace blb::ecs {
	EntityID World::nextID = 1;

	World::World() {
		componentManager.componentAddedDelegate.bind(&World::onComponentAdded, this);
		componentManager.componentRemovedDelegate.bind(&World::onComponentRemoved, this);
	}

	World::~World() = default;

	void World::update(utl::DeltaTime deltaTime) {
		CLV_PROFILE_FUNCTION();

		if(pendingDestroyIDs.size() > 0) {
			std::set<EntityID> idSet{ pendingDestroyIDs.begin(), pendingDestroyIDs.end() };
			for(int i = activeIDs.size() - 1; i >= 0; --i) {
				if(idSet.find(activeIDs[i]) != idSet.end()) {
					activeIDs.erase(activeIDs.begin() + i);
				}
			}
		}

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

	Entity World::createEntity() {
		EntityID ID = nextID++;

		activeIDs.push_back(ID);

		return { ID, this };
	}

	Entity World::cloneEntitiesComponents(EntityID ID) {
		Entity clonedEntity = createEntity();

		componentManager.cloneEntitiesComponents(ID, clonedEntity.getID());

		return clonedEntity;
	}

	bool World::isEntityValid(EntityID ID) {
		if(ID != INVALID_ENTITY_ID) {
			return std::find(activeIDs.begin(), activeIDs.end(), ID) != activeIDs.end();
		} else {
			return false;
		}
	}

	Entity World::getEntity(EntityID ID) {
		if(isEntityValid(ID)) {
			return { ID, this };
		} else {
			return {};
		}
	}

	void World::destroyEntity(EntityID ID) {
		auto foundIDIter = std::find(activeIDs.begin(), activeIDs.end(), ID);

		if(ID == INVALID_ENTITY_ID || foundIDIter == activeIDs.end()) {
			return;
		}

		pendingDestroyIDs.push_back(ID);
	}

	void World::destroyAllEntites() {
		for(EntityID id : activeIDs) {
			componentManager.onEntityDestroyed(id);
		}
		activeIDs.clear();
	}

	void World::onComponentAdded(ComponentInterface* component) {
		for(const auto& system : systems) {
			system->onComponentCreated(component);
		}
	}

	void World::onComponentRemoved(ComponentInterface* component) {
		for(const auto& system : systems) {
			system->onComponentDestroyed(component);
		}
	}
}

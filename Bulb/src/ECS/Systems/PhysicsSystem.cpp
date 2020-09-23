#include "Bulb/ECS/Systems/PhysicsSystem.hpp"

#include "Bulb/ECS/Components/CollisionResponseComponent.hpp"
#include "Bulb/ECS/Components/CubeColliderComponent.hpp"
#include "Bulb/ECS/Components/RigidBodyComponent.hpp"
#include "Bulb/ECS/Components/TransformComponent.hpp"
#include "Bulb/ECS/World.hpp"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <Clove/Event/EventDispatcher.hpp>
#include <btBulletDynamicsCommon.h>

using namespace clv;

namespace blb::ecs {
    PhysicsSystem::PhysicsSystem() {
        collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher             = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
        overlappingPairCache   = std::make_unique<btDbvtBroadphase>();
        solver                 = std::make_unique<btSequentialImpulseConstraintSolver>();

        dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), overlappingPairCache.get(), solver.get(), collisionConfiguration.get());
    }

    PhysicsSystem::PhysicsSystem(PhysicsSystem&& other) noexcept {
        collisionConfiguration = std::move(other.collisionConfiguration);
        dispatcher             = std::move(other.dispatcher);
        overlappingPairCache   = std::move(other.overlappingPairCache);
        solver                 = std::move(other.solver);

        dynamicsWorld = std::move(other.dynamicsWorld);

        cubeColliderAddedHandle   = std::move(other.cubeColliderAddedHandle);
        cubeColliderRemovedHandle = std::move(other.cubeColliderRemovedHandle);

        rigidBodyAddedHandle   = std::move(other.rigidBodyAddedHandle);
        rigidBodyRemovedHandle = std::move(other.rigidBodyRemovedHandle);

        currentCollisions = std::move(other.currentCollisions);
    }

    PhysicsSystem& PhysicsSystem::operator=(PhysicsSystem&& other) noexcept = default;

    PhysicsSystem::~PhysicsSystem() = default;

    void PhysicsSystem::registerToEvents(EventDispatcher& dispatcher) {
        cubeColliderAddedHandle = dispatcher.bindToEvent<ComponentAddedEvent<CubeColliderComponent>>([this](const ComponentAddedEvent<CubeColliderComponent>& event) {
            onCubeColliderAdded(event);
        });

        cubeColliderRemovedHandle = dispatcher.bindToEvent<ComponentRemovedEvent<CubeColliderComponent>>([this](const ComponentRemovedEvent<CubeColliderComponent>& event) {
            onCubeColliderRemoved(event);
        });

        rigidBodyAddedHandle = dispatcher.bindToEvent<ComponentAddedEvent<RigidBodyComponent>>([this](const ComponentAddedEvent<RigidBodyComponent>& event) {
            onRigidBodyAdded(event);
        });

        rigidBodyRemovedHandle = dispatcher.bindToEvent<ComponentRemovedEvent<RigidBodyComponent>>([this](const ComponentRemovedEvent<RigidBodyComponent>& event) {
            onRigidBodyRemoved(event);
        });
    }

    void PhysicsSystem::preUpdate(World& world) {
        CLV_PROFILE_FUNCTION();

        //Make sure any colliders are properly paired with their rigid body
        for(auto&& [cubeCollider, rigidBody] : world.getComponentSets<CubeColliderComponent, RigidBodyComponent>()) {
            if(cubeCollider->collisionObject != nullptr) {
                dynamicsWorld->removeCollisionObject(cubeCollider->collisionObject.get());
                cubeCollider->collisionObject.reset();
            }

            if(rigidBody->standInShape != nullptr) {
                dynamicsWorld->removeCollisionObject(rigidBody->body.get());
                rigidBody->body->setCollisionShape(cubeCollider->collisionShape.get());
                addBodyToWorld(*rigidBody);

                rigidBody->standInShape.reset();
            }
        }

        //Make sure any recently un-paired colliders are updated
        for(auto&& [cubeCollider] : world.getComponentSets<CubeColliderComponent>()) {
            if(world.hasComponent<RigidBodyComponent>(cubeCollider->getEntityID())) {
                continue;
            }

            if(cubeCollider->collisionObject == nullptr) {
                cubeCollider->constructCollisionObject();
                addColliderToWorld(*cubeCollider);
            }
        }

        //Make sure any recently un-paired rigid bodies are updated
        for(auto&& [rigidBody] : world.getComponentSets<RigidBodyComponent>()) {
            if(world.hasComponent<CubeColliderComponent>(rigidBody->getEntityID())) {
                continue;
            }

            if(rigidBody->standInShape == nullptr) {
                btRigidBody* body = rigidBody->body.get();

                dynamicsWorld->removeCollisionObject(body);
                rigidBody->standInShape = RigidBodyComponent::createStandInShape();
                body->setCollisionShape(rigidBody->standInShape.get());
                addBodyToWorld(*rigidBody);
            }
        }
    }

    void PhysicsSystem::update(World& world, utl::DeltaTime deltaTime) {
        CLV_PROFILE_FUNCTION();

        const auto cubeColliders = world.getComponentSets<TransformComponent, CubeColliderComponent>();
        const auto rigidBodies   = world.getComponentSets<TransformComponent, RigidBodyComponent>();

        const auto updateCollider = [](const TransformComponent& transform, btCollisionObject& collisionObject) {
            const auto pos = transform.getPosition(TransformSpace::World);
            const auto rot = transform.getRotation(TransformSpace::World);

            btTransform btTrans = collisionObject.getWorldTransform();
            btTrans.setOrigin({ pos.x, pos.y, pos.z });
            btTrans.setRotation({ rot.x, rot.y, rot.z, rot.w });
            collisionObject.setWorldTransform(btTrans);
        };

        //Notify Bullet of the location of the colliders
        for(auto&& [transform, cubeCollider] : cubeColliders) {
            if(world.hasComponent<RigidBodyComponent>(cubeCollider->getEntityID())) {
                continue;
            }

            updateCollider(*transform, *cubeCollider->collisionObject);
        }
        for(auto&& [transform, rigidBody] : rigidBodies) {
            updateCollider(*transform, *rigidBody->body);
        }

        //Step physics world
        dynamicsWorld->stepSimulation(deltaTime.getDeltaSeconds());

        const auto updateTransform = [](TransformComponent& transform, const btCollisionObject& collisionObject) {
            const btTransform& btTrans = collisionObject.getWorldTransform();
            const btVector3& pos       = btTrans.getOrigin();
            const btQuaternion& rot    = btTrans.getRotation();

            transform.setPosition({ pos.x(), pos.y(), pos.z() }, TransformSpace::World);
            transform.setRotation({ rot.getW(), rot.getX(), rot.getY(), rot.getZ() }, TransformSpace::World);
        };

        //Apply any simulation updates
        for(auto&& [transform, cubeCollider] : cubeColliders) {
            if(world.hasComponent<RigidBodyComponent>(cubeCollider->getEntityID())) {
                continue;
            }

            updateTransform(*transform, *cubeCollider->collisionObject);
        }
        for(auto&& [transform, rigidBody] : rigidBodies) {
            updateTransform(*transform, *rigidBody->body);
        }
    }

    void PhysicsSystem::postUpdate(World& world) {
        //Gather collision manifolds
        const int numManifolds = dispatcher->getNumManifolds();
        std::unordered_set<CollisionManifold, ManifoldHasher> newCollisions;

        for(int i = 0; i < numManifolds; ++i) {
            btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);
            if(manifold->getNumContacts() > 0) {
                const btCollisionObject* obA = manifold->getBody0();
                const btCollisionObject* obB = manifold->getBody1();

                EntityID entityA = obA->getUserIndex();
                EntityID entityB = obB->getUserIndex();

                newCollisions.emplace(CollisionManifold{ entityA, entityB });
            }
        }

        std::vector<CollisionManifold> collisionBeginManifolds;
        std::vector<CollisionManifold> collisionEndManifolds;

        //Copy any elements not in our cached set
        collisionBeginManifolds.reserve(std::size(newCollisions));
        std::copy_if(std::begin(newCollisions), std::end(newCollisions), std::back_inserter(collisionBeginManifolds), [this](const CollisionManifold& manifold) {
            return !currentCollisions.contains(manifold);
        });

        //Copy any elements not in our new array
        collisionEndManifolds.reserve(std::size(currentCollisions));
        std::copy_if(std::begin(currentCollisions), std::end(currentCollisions), std::back_inserter(collisionEndManifolds), [&](const CollisionManifold& manifold) {
            return !newCollisions.contains(manifold);
        });

        //Broadcast collision begin events
        for(const auto& manifold : collisionBeginManifolds) {
            Entity entityA = world.getEntity(manifold.entityA);
            Entity entityB = world.getEntity(manifold.entityB);

            if(auto entityAComp = entityA.getComponent<CollisionResponseComponent>()) {
                entityAComp->onCollisionBegin.broadcast(Collision{ entityA, entityB });
            }

            if(auto entityBComp = entityB.getComponent<CollisionResponseComponent>()) {
                entityBComp->onCollisionBegin.broadcast(Collision{ entityB, entityA });
            }

            currentCollisions.emplace(manifold);
        }

        //Broadcast collision end events
        for(const auto& manifold : collisionEndManifolds) {
            Entity entityA = world.getEntity(manifold.entityA);
            Entity entityB = world.getEntity(manifold.entityB);

            if(auto entityAComp = entityA.getComponent<CollisionResponseComponent>()) {
                entityAComp->onCollisionEnd.broadcast(Collision{ entityA, entityB });
            }

            if(auto entityBComp = entityB.getComponent<CollisionResponseComponent>()) {
                entityBComp->onCollisionEnd.broadcast(Collision{ entityB, entityA });
            }

            currentCollisions.erase(manifold);
        }
    }

    EntityID PhysicsSystem::rayCast(const clv::mth::vec3f& begin, const clv::mth::vec3f& end) {
        btVector3 btBegin{ begin.x, begin.y, begin.z };
        btVector3 btEnd{ end.x, end.y, end.z };

        btCollisionWorld::ClosestRayResultCallback callBack{ btBegin, btEnd };
        dynamicsWorld->rayTest(btBegin, btEnd, callBack);

        if(callBack.m_collisionObject != nullptr) {
            return callBack.m_collisionObject->getUserIndex();
        } else {
            return INVALID_ENTITY_ID;
        }
    }

    void PhysicsSystem::onCubeColliderAdded(const ComponentAddedEvent<CubeColliderComponent>& event) {
        addColliderToWorld(*event.component);
    }

    void PhysicsSystem::onCubeColliderRemoved(const ComponentRemovedEvent<CubeColliderComponent>& event) {
        if(btCollisionObject* object = event.component->collisionObject.get()) {
            dynamicsWorld->removeCollisionObject(object);
        }
    }

    void PhysicsSystem::onRigidBodyAdded(const ComponentAddedEvent<RigidBodyComponent>& event) {
        addBodyToWorld(*event.component);
    }

    void PhysicsSystem::onRigidBodyRemoved(const ComponentRemovedEvent<RigidBodyComponent>& event) {
        dynamicsWorld->removeCollisionObject(event.component->body.get());
    }

    void PhysicsSystem::addBodyToWorld(const RigidBodyComponent& rigidBodyComponent) {
        dynamicsWorld->addRigidBody(rigidBodyComponent.body.get(), rigidBodyComponent.descriptor.collisionGroup, rigidBodyComponent.descriptor.collisionMask);
        rigidBodyComponent.body->setUserIndex(rigidBodyComponent.getEntityID());
    }

    void PhysicsSystem::addColliderToWorld(const CubeColliderComponent& colliderComponent) {
        dynamicsWorld->addCollisionObject(colliderComponent.collisionObject.get(), ~0, ~0);//Add the collider to every group and collide with every other group
        colliderComponent.collisionObject->setUserIndex(colliderComponent.getEntityID());
    }
}
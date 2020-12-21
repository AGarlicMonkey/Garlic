#include <Clove/ECS/EntityManager.hpp>
#include <gtest/gtest.h>

using namespace garlic::clove;

class ValueComponent {
public:
    int32_t value{ 0 };
};

class BoolComponent {
public:
    bool value{ false };
};

void makeTrue(BoolComponent &comp) {
    comp.value = true;
}

TEST(ECSSystemTests, CanUseMemberFunction) {
    EntityManager entityManager;

    class TestSystem {
    public:
        int32_t const entityValue{ 18 };
        bool const boolVal{ true };

        bool excludeCalled{ false };

        void update(ValueComponent &component) {
            component.value = entityValue;
        }
        void update_const(BoolComponent &component) const {
            component.value = boolVal;
        }

        void update_exclude(ValueComponent &component) {
            excludeCalled = true;
        }
    };
    TestSystem system;

    std::vector<Entity> entities;
    for(int i = 0; i < 5; ++i) {
        auto entity{ entityManager.create() };
        entityManager.addComponent<ValueComponent>(entity);
        entityManager.addComponent<BoolComponent>(entity);
        entities.push_back(entity);
    }

    entityManager.forEach(&TestSystem::update, &system);
    entityManager.forEach(&TestSystem::update_const, &system);

    for(auto entity : entities) {
        EXPECT_EQ(entityManager.getComponent<ValueComponent>(entity).value, system.entityValue);
        EXPECT_TRUE(entityManager.getComponent<BoolComponent>(entity).value);
    }

    entityManager.forEach(&TestSystem::update_exclude, &system, Exclude<BoolComponent>{});

    EXPECT_FALSE(system.excludeCalled);

    auto entity{ entityManager.create() };
    entityManager.addComponent<ValueComponent>(entity);
    entities.push_back(entity);

    entityManager.forEach(&TestSystem::update_exclude, &system, Exclude<BoolComponent>{});

    EXPECT_TRUE(system.excludeCalled);
}

TEST(ECSSystemTests, CanUseFreeFunction) {
    EntityManager entityManager;

    std::vector<Entity> entities;
    for(int i = 0; i < 5; ++i) {
        auto entity{ entityManager.create() };
        entityManager.addComponent<ValueComponent>(entity);
        entityManager.addComponent<BoolComponent>(entity);
        entities.push_back(entity);
    }

    entityManager.forEach(&makeTrue);

    for(auto entity : entities) {
        EXPECT_TRUE(entityManager.getComponent<BoolComponent>(entity).value);
    }
}

TEST(ECSSystemTests, CanUseLambdaFunction) {
    EntityManager entityManager;

    std::vector<Entity> entities;
    for(int i = 0; i < 5; ++i) {
        auto entity{ entityManager.create() };
        entityManager.addComponent<ValueComponent>(entity);
        entityManager.addComponent<BoolComponent>(entity);
        entities.push_back(entity);
    }

    int32_t constexpr entityVal{ 32 };
    entityManager.forEach([entityVal](ValueComponent &valComp, BoolComponent &boolComp) {
        valComp.value  = entityVal;
        boolComp.value = true;
    });

    for(auto entity : entities) {
        EXPECT_EQ(entityManager.getComponent<ValueComponent>(entity).value, entityVal);
        EXPECT_TRUE(entityManager.getComponent<BoolComponent>(entity).value);
    }

    auto func = [entityVal](BoolComponent &boolComp) {
        boolComp.value = false;
    };
    entityManager.forEach(func);

    for(auto entity : entities) {
        EXPECT_FALSE(entityManager.getComponent<BoolComponent>(entity).value);
    }

    bool excludeCalled{ false };
    auto excludeTestFunc = [&excludeCalled](BoolComponent &boolComp) {
        excludeCalled = true;
    };
    entityManager.forEach(excludeTestFunc, Exclude<ValueComponent>{});

    EXPECT_FALSE(excludeCalled);

    auto entity{ entityManager.create() };
    entityManager.addComponent<BoolComponent>(entity);
    entities.push_back(entity);

    entityManager.forEach(excludeTestFunc, Exclude<ValueComponent>{});

    EXPECT_TRUE(excludeCalled);
}

TEST(ECSSystemTests, CanGetEntityId) {
    EntityManager entityManager;

    std::vector<Entity> entities;
    for(int i = 0; i < 5; ++i) {
        auto entity{ entityManager.create() };
        entityManager.addComponent<ValueComponent>(entity);
        entityManager.addComponent<BoolComponent>(entity);
        entities.push_back(entity);
    }

    entityManager.forEach([&](Entity entity, BoolComponent &boolComp) {
        EXPECT_TRUE(entity != NullEntity);
        EXPECT_TRUE(entityManager.hasComponent<ValueComponent>(entity));
    });
}
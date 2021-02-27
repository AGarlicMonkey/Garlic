#include "Clove/Layers/RenderLayer.hpp"

#include "Clove/Application.hpp"
#include "Clove/Components/AnimatedModelComponent.hpp"
#include "Clove/Components/CameraComponent.hpp"
#include "Clove/Components/DirectionalLightComponent.hpp"
#include "Clove/Components/PointLightComponent.hpp"
#include "Clove/Components/StaticModelComponent.hpp"
#include "Clove/Components/TransformComponent.hpp"
#include "Clove/Rendering/ForwardRenderer3D.hpp"
#include "Clove/Rendering/Renderables/Mesh.hpp"

#include <Clove/ECS/EntityManager.hpp>
#include <Clove/Maths/Maths.hpp>
#include <Clove/Maths/MathsHelpers.hpp>

namespace garlic::clove {
    RenderLayer::RenderLayer(ForwardRenderer3D *renderer, EntityManager *entityManager)
        : Layer("Render")
        , renderer{ renderer }
        , entityManager{ entityManager } {
    }

    RenderLayer::RenderLayer(RenderLayer &&other) noexcept = default;

    RenderLayer &RenderLayer::operator=(RenderLayer &&other) noexcept = default;

    RenderLayer::~RenderLayer() = default;

    void RenderLayer::onUpdate(DeltaTime const deltaTime) {
        CLOVE_PROFILE_FUNCTION();

        Entity activeCamera{};

        //Transform and submit cameras
        entityManager->forEach([this, &activeCamera](Entity entity, TransformComponent const &transform, CameraComponent &camera) {
            vec3f const position{ decomposeTranslation(transform.worldMatrix) };

            vec3f const camFront{ transform.getForward() };
            vec3f const camUp{ transform.getUp() };

            camera.camera.setView(lookAt(position, position + camFront, camUp));

            renderer->submitCamera(camera.camera, position);

            activeCamera = entity;
        });

        //Submit static meshes
        entityManager->forEach([this](TransformComponent const &transform, StaticModelComponent const &staticModel) {
            mat4f const modelTransform{ transform.worldMatrix };
            std::array<mat4f, MAX_JOINTS> matrixPalet{};
            matrixPalet.fill(mat4f{ 1.0f });

            std::set<GeometryPass::Id> passIds;
            for(auto const &technique : staticModel.model.getTechniques()) {
                passIds.insert(technique.passIds.begin(), technique.passIds.end());
            }
            for(auto const &mesh : staticModel.model.getMeshes()) {
                renderer->submitMesh(ForwardRenderer3D::MeshInfo{ mesh, staticModel.model.getMaterial(), modelTransform, matrixPalet }, passIds);
            }
        });
        //Submit animated meshes
        entityManager->forEach([this, &deltaTime](TransformComponent const &transform, AnimatedModelComponent &animatedModel) {
            mat4f const modelTransform{ transform.worldMatrix };
            auto const matrixPalet{ animatedModel.model.update(deltaTime) };

            std::set<GeometryPass::Id> passIds;
            for(auto const &technique : animatedModel.model.getTechniques()) {
                passIds.insert(technique.passIds.begin(), technique.passIds.end());
            }
            for(auto const &mesh : animatedModel.model.getMeshes()) {
                renderer->submitMesh(ForwardRenderer3D::MeshInfo{ mesh, animatedModel.model.getMaterial(), modelTransform, matrixPalet }, passIds);
            }
        });

        //Submit directional lights
        vec3f const dirLightPos{ activeCamera != NullEntity ? entityManager->getComponent<TransformComponent>(activeCamera).position : vec3f{ 0.0f } };
        entityManager->forEach([this, &dirLightPos](DirectionalLightComponent &light) {
            float constexpr mapSize{ 30.0f };
            float constexpr nearDist{ -100.0f };//Negative near dist means we don't have to move the light along it's direction vector
            float constexpr farDist{ 100.0f };
            mat4f const shadowProj{ createOrthographicMatrix(-mapSize, mapSize, -mapSize, mapSize, nearDist, farDist) };

            DirectionalLight lightProxy{
                .data = {
                    .direction = light.direction,
                    .ambient   = light.ambientColour,
                    .diffuse   = light.diffuseColour,
                    .specular  = light.specularColour,
                },
                .shadowTransform = shadowProj * lookAt(dirLightPos, dirLightPos + light.direction, vec3f{ 0.0f, 1.0f, 0.0f }),
            };

            renderer->submitLight(lightProxy);
        });
        //Submit point lights
        entityManager->forEach([this](TransformComponent const &transform, PointLightComponent &light) {
            float constexpr farDist{ 100.0f };
            mat4f const shadowProj{ createPerspectiveMatrix(asRadians(90.0f), 1.0f, 0.5f, farDist) };

            vec3f const &position{ transform.position };

            PointLight lightProxy{
                .data = {
                    .position = position,
                    .ambient  = light.ambientColour,
                    .diffuse  = light.diffuseColour,
                    .specular = light.specularColour,
                    .farPlane = farDist,
                },
                .shadowTransforms = {
                    shadowProj * lookAt(position, position + vec3f{ 1.0f, 0.0f, 0.0f }, vec3f{ 0.0f, 1.0f, 0.0f }),
                    shadowProj * lookAt(position, position + vec3f{ -1.0f, 0.0f, 0.0f }, vec3f{ 0.0f, 1.0f, 0.0f }),
                    shadowProj * lookAt(position, position + vec3f{ 0.0f, 1.0f, 0.0f }, vec3f{ 0.0f, 0.0f, -1.0f }),
                    shadowProj * lookAt(position, position + vec3f{ 0.0f, -1.0f, 0.0f }, vec3f{ 0.0f, 0.0f, 1.0f }),
                    shadowProj * lookAt(position, position + vec3f{ 0.0f, 0.0f, 1.0f }, vec3f{ 0.0f, 1.0f, 0.0f }),
                    shadowProj * lookAt(position, position + vec3f{ 0.0f, 0.0f, -1.0f }, vec3f{ 0.0f, 1.0f, 0.0f }),
                }
            };

            renderer->submitLight(lightProxy);
        });
    }
}

#include "Clove/Rendering/RenderPasses/PointLightPass.hpp"

#include "Clove/Rendering/Renderables/Mesh.hpp"
#include "Clove/Rendering/RenderingHelpers.hpp"
#include "Clove/Rendering/Vertex.hpp"

#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaGraphicsCommandBuffer.hpp>
#include <Clove/Graphics/GhaGraphicsPipelineObject.hpp>
#include <Clove/Graphics/GhaRenderPass.hpp>

extern "C" const char constants[];
extern "C" const size_t constantsLength;

extern "C" const char meshcubeshadowmap_v[];
extern "C" const size_t meshcubeshadowmap_vLength;
extern "C" const char meshcubeshadowmap_p[];
extern "C" const size_t meshcubeshadowmap_pLength;

namespace garlic::clove {
    PointLightPass::PointLightPass(GhaFactory &ghaFactory, std::shared_ptr<GhaRenderPass> ghaRenderPass) {
        //Build include map
        std::unordered_map<std::string, std::string> shaderIncludes;
        shaderIncludes["Constants.glsl"] = { constants, constantsLength };

        //Pipeline
        std::vector<VertexAttributeDescriptor> const vertexAttributes{
            VertexAttributeDescriptor{
                .format   = VertexAttributeFormat::R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, position),
            },
        };

        PushConstantDescriptor const vertexPushConstant{
            .stage  = GhaShader::Stage::Vertex,
            .offset = 0,
            .size   = sizeof(mat4f),
        };
        PushConstantDescriptor const pixelPushConstant{
            .stage  = GhaShader::Stage::Pixel,
            .offset = vertexPushConstant.size,
            .size   = sizeof(vec3f) + sizeof(float),
        };

        AreaDescriptor const viewScissorArea{
            .state    = ElementState::Static,
            .position = { 0.0f, 0.0f },
            .size     = { shadowMapSize, shadowMapSize }
        };

        pipeline = *ghaFactory.createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor{
            .vertexShader         = *ghaFactory.createShaderFromSource({ meshcubeshadowmap_v, meshcubeshadowmap_vLength }, shaderIncludes, "Cube Shadow Map - Animated Mesh (vertex)", GhaShader::Stage::Vertex),
            .pixelShader          = *ghaFactory.createShaderFromSource({ meshcubeshadowmap_p, meshcubeshadowmap_pLength }, shaderIncludes, "Cube Shadow Map (pixel)", GhaShader::Stage::Pixel),
            .vertexInput          = Vertex::getInputBindingDescriptor(),
            .vertexAttributes     = vertexAttributes,
            .viewportDescriptor   = viewScissorArea,
            .scissorDescriptor    = viewScissorArea,
            .enableBlending       = false,
            .renderPass           = std::move(ghaRenderPass),
            .descriptorSetLayouts = {
                createMeshDescriptorSetLayout(ghaFactory),
            },
            .pushConstants = {
                vertexPushConstant,
                pixelPushConstant,
            },
        });
    }

    PointLightPass::PointLightPass(PointLightPass &&other) noexcept = default;

    PointLightPass &PointLightPass::operator=(PointLightPass &&other) noexcept = default;

    PointLightPass::~PointLightPass() = default;

    void PointLightPass::execute(GhaGraphicsCommandBuffer &commandBuffer, FrameData const &frameData) {
        mat4f const *vertPushConstantData{ frameData.currentPointLightTransform };
        size_t const vertPushConstantSize{ sizeof(mat4f) };

        struct {
            vec3f pos{};
            float farPlane{};
        } const pixelPushConstantData{
            .pos      = frameData.currentPointLightPosition,
            .farPlane = frameData.currentPointLightFarPlane,
        };
        size_t const pixelPushConstantOffset{ vertPushConstantSize };
        size_t const pixelPushConstantSize{ sizeof(pixelPushConstantData) };

        commandBuffer.bindPipelineObject(*pipeline);
        for(auto const &job : getJobs()) {
            commandBuffer.pushConstant(GhaShader::Stage::Vertex, 0, vertPushConstantSize, vertPushConstantData);
            commandBuffer.pushConstant(GhaShader::Stage::Pixel, pixelPushConstantOffset, pixelPushConstantSize, &pixelPushConstantData);
            commandBuffer.bindDescriptorSet(*frameData.meshDescriptorSets[job.meshDescriptorIndex], 0);

            commandBuffer.bindVertexBuffer(*job.mesh->getCombinedBuffer(), job.mesh->getVertexOffset());
            commandBuffer.bindIndexBuffer(*job.mesh->getCombinedBuffer(), job.mesh->getIndexOffset(), IndexType::Uint16);

            commandBuffer.drawIndexed(job.mesh->getIndexCount());
        }
    }
}

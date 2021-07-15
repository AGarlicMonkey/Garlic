#include "Clove/Rendering/RenderPasses/DirectionalLightPass.hpp"

#include "Clove/Rendering/Renderables/Mesh.hpp"
#include "Clove/Rendering/RenderingHelpers.hpp"
#include "Clove/Rendering/Vertex.hpp"

#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaGraphicsCommandBuffer.hpp>
#include <Clove/Graphics/GhaGraphicsPipelineObject.hpp>
#include <Clove/Graphics/GhaRenderPass.hpp>

extern "C" const char constants[];
extern "C" const size_t constantsLength;

extern "C" const char meshshadowmap_v[];
extern "C" const size_t meshshadowmap_vLength;
extern "C" const char meshshadowmap_p[];
extern "C" const size_t meshshadowmap_pLength;

namespace garlic::clove {
    DirectionalLightPass::DirectionalLightPass(GhaFactory &ghaFactory, std::shared_ptr<GhaRenderPass> ghaRenderPass) {
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

        PushConstantDescriptor const pushConstant{
            .stage = GhaShader::Stage::Vertex,
            .size  = sizeof(mat4f),
        };

        AreaDescriptor const viewScissorArea{
            .state    = ElementState::Static,
            .position = { 0.0f, 0.0f },
            .size     = { shadowMapSize, shadowMapSize }
        };

        pipeline = *ghaFactory.createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor{
            .vertexShader         = *ghaFactory.createShaderFromSource({ meshshadowmap_v, meshshadowmap_vLength }, shaderIncludes, "Shadow Map - Animated Mesh (vertex)", GhaShader::Stage::Vertex),
            .pixelShader          = *ghaFactory.createShaderFromSource({ meshshadowmap_p, meshshadowmap_pLength }, shaderIncludes, "Shadow Map (pixel)", GhaShader::Stage::Pixel),
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
                pushConstant,
            },
        });
    }

    DirectionalLightPass::DirectionalLightPass(DirectionalLightPass &&other) noexcept = default;

    DirectionalLightPass &DirectionalLightPass::operator=(DirectionalLightPass &&other) noexcept = default;

    DirectionalLightPass::~DirectionalLightPass() = default;

    void DirectionalLightPass::execute(GhaGraphicsCommandBuffer &commandBuffer, FrameData const &frameData) {
        commandBuffer.bindPipelineObject(*pipeline);

        for(auto const &job : getJobs()) {
            mat4f const *pushConstantData{ frameData.currentDirLightTransform };
            size_t const pushConstantSize{ sizeof(mat4f) };

            commandBuffer.pushConstant(GhaShader::Stage::Vertex, 0, pushConstantSize, pushConstantData);
            commandBuffer.bindDescriptorSet(*frameData.meshDescriptorSets[job.meshDescriptorIndex], 0);

            commandBuffer.bindVertexBuffer(*job.mesh->getCombinedBuffer(), job.mesh->getVertexOffset());
            commandBuffer.bindIndexBuffer(*job.mesh->getCombinedBuffer(), job.mesh->getIndexOffset(), IndexType::Uint16);

            commandBuffer.drawIndexed(job.mesh->getIndexCount());
        }
    }
}
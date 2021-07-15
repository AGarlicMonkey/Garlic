#include "Clove/Rendering/RenderPasses/ForwardColourPass.hpp"

#include "Clove/Rendering/Renderables/Mesh.hpp"
#include "Clove/Rendering/RenderingHelpers.hpp"
#include "Clove/Rendering/Vertex.hpp"

#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaGraphicsCommandBuffer.hpp>
#include <Clove/Graphics/GhaGraphicsPipelineObject.hpp>
#include <Clove/Graphics/GhaRenderPass.hpp>

extern "C" const char constants[];
extern "C" const size_t constantsLength;

extern "C" const char mesh_v[];
extern "C" const size_t mesh_vLength;
extern "C" const char mesh_p[];
extern "C" const size_t mesh_pLength;

namespace garlic::clove {
    ForwardColourPass::ForwardColourPass(GhaFactory &ghaFactory, std::shared_ptr<GhaRenderPass> ghaRenderPass) {
        //Build include map
        std::unordered_map<std::string, std::string> shaderIncludes;
        shaderIncludes["Constants.glsl"] = { constants, constantsLength };

        //Create attributes for animated meshes
        std::vector<VertexAttributeDescriptor> const vertexAttributes{
            VertexAttributeDescriptor{
                .format   = VertexAttributeFormat::R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, position),
            },
            VertexAttributeDescriptor{
                .format   = VertexAttributeFormat::R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, normal),
            },
            VertexAttributeDescriptor{
                .format   = VertexAttributeFormat::R32G32_SFLOAT,
                .offset   = offsetof(Vertex, texCoord),
            },
            VertexAttributeDescriptor{
                .format   = VertexAttributeFormat::R32G32B32_SFLOAT,
                .offset   = offsetof(Vertex, colour),
            },
        };

        AreaDescriptor const viewScissorArea{
            .state = ElementState::Dynamic,
        };

        pipeline = *ghaFactory.createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor{
            .vertexShader         = *ghaFactory.createShaderFromSource({ mesh_v, mesh_vLength }, shaderIncludes, "Animated Mesh (vertex)", GhaShader::Stage::Vertex),
            .pixelShader          = *ghaFactory.createShaderFromSource({ mesh_p, mesh_pLength }, shaderIncludes, "Mesh (pixel)", GhaShader::Stage::Pixel),
            .vertexInput          = Vertex::getInputBindingDescriptor(),
            .vertexAttributes     = vertexAttributes,
            .viewportDescriptor   = viewScissorArea,
            .scissorDescriptor    = viewScissorArea,
            .renderPass           = std::move(ghaRenderPass),
            .descriptorSetLayouts = {
                createMeshDescriptorSetLayout(ghaFactory),
                createViewDescriptorSetLayout(ghaFactory),
                createLightingDescriptorSetLayout(ghaFactory),
            },
            .pushConstants = {},
        });
    }

    ForwardColourPass::ForwardColourPass(ForwardColourPass &&other) noexcept = default;

    ForwardColourPass &ForwardColourPass::operator=(ForwardColourPass &&other) noexcept = default;

    ForwardColourPass::~ForwardColourPass() = default;

    void ForwardColourPass::execute(GhaGraphicsCommandBuffer &commandBuffer, FrameData const &frameData) {
        commandBuffer.bindPipelineObject(*pipeline);

        commandBuffer.bindDescriptorSet(*frameData.viewDescriptorSet, 1);
        commandBuffer.bindDescriptorSet(*frameData.lightingDescriptorSet, 2);

        for(auto const &job : getJobs()) {
            commandBuffer.bindDescriptorSet(*frameData.meshDescriptorSets[job.meshDescriptorIndex], 0);

            commandBuffer.bindVertexBuffer(*job.mesh->getCombinedBuffer(), job.mesh->getVertexOffset());
            commandBuffer.bindIndexBuffer(*job.mesh->getCombinedBuffer(), job.mesh->getIndexOffset(), IndexType::Uint16);

            commandBuffer.drawIndexed(job.mesh->getIndexCount());
        }
    }
}
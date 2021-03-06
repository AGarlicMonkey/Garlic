#include "Clove/Rendering/RenderPasses/SkinningPass.hpp"

#include "Clove/Rendering/RenderingHelpers.hpp"
#include "Clove/Rendering/Renderables/Mesh.hpp"

#include <Clove/Graphics/GhaComputePipelineObject.hpp>
#include <Clove/Graphics/GhaDescriptorSetLayout.hpp>
#include <Clove/Graphics/GhaFactory.hpp>

extern "C" const char constants[];
extern "C" const size_t constantsLength;

extern "C" const char skinning_c[];
extern "C" const size_t skinning_cLength;

namespace garlic::clove {
    SkinningPass::SkinningPass(GhaFactory &ghaFactory) {
        //Build include map
        std::unordered_map<std::string, std::string> shaderIncludes;
        shaderIncludes["Constants.glsl"] = { constants, constantsLength };

        //Create pipeline
        GhaComputePipelineObject::Descriptor const skinningPipelineDescriptor{
            .shader               = *ghaFactory.createShaderFromSource({ skinning_c, skinning_cLength }, shaderIncludes, "Skinning (compute)", GhaShader::Stage::Compute),
            .descriptorSetLayouts = {
                createSkinningDescriptorSetLayout(ghaFactory),
            }
        };

        pipeline = *ghaFactory.createComputePipelineObject(skinningPipelineDescriptor);
    }

    SkinningPass::SkinningPass(SkinningPass &&other) noexcept = default;

    SkinningPass &SkinningPass::operator=(SkinningPass &&other) noexcept = default;

    SkinningPass::~SkinningPass() = default;

    void SkinningPass::execute(GhaComputeCommandBuffer &commandBuffer, FrameData const &frameData) {
        commandBuffer.bindPipelineObject(*pipeline);

        for(auto const &job : getJobs()) {
            commandBuffer.bindDescriptorSet(*frameData.skinningMeshSets[job.meshDescriptorIndex], 0);
            commandBuffer.disptach({ (job.mesh->getVertexCount() / AVERAGE_WORK_GROUP_SIZE) + 1, 1, 1 });
        }
    }
}
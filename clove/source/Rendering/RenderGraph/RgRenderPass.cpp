#include "Clove/Rendering/RenderGraph/RgRenderPass.hpp"

#include "Clove/Rendering/RenderGraph/RgGlobalCache.hpp"

namespace garlic::clove {
    RgRenderPass::RgRenderPass(RgPassIdType id, Descriptor descriptor)
        : RgPass{ id }
        , descriptor{ std::move(descriptor) } {
    }

    RgRenderPass::RgRenderPass(RgRenderPass &&other) noexcept = default;

    RgRenderPass &RgRenderPass::operator=(RgRenderPass &&other) noexcept = default;

    RgRenderPass::~RgRenderPass() = default;

    std::unordered_set<RgResourceIdType> RgRenderPass::getInputResources() const {
        std::unordered_set<RgResourceIdType> inputResources{};
        for(auto const &submission : submissions) {
            inputResources.emplace(submission.vertexBuffer);
            inputResources.emplace(submission.indexBuffer);
            for(auto const &ubo : submission.shaderUbos){
                inputResources.emplace(ubo.buffer);
            }
            for(auto const &image : submission.shaderCombinedImageSamplers){
                inputResources.emplace(image.image);
            }
        }
        return inputResources;
    }

    std::unordered_set<RgResourceIdType> RgRenderPass::getOutputResources() const {
        std::unordered_set<RgResourceIdType> outputResources{};
        for(auto const &renderTarget : descriptor.renderTargets){
            outputResources.emplace(renderTarget.target);
        }
        outputResources.emplace(descriptor.depthStencil.target);
        return outputResources;
    }
}
#pragma once

#include "Clove/Graphics/GhaGraphicsPipelineObject.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
    class MetalGraphicsPipelineObject : public GhaGraphicsPipelineObject {
        //VARIABLES
    private:
        Descriptor descriptor{};

        id<MTLRenderPipelineState> pipeline;
        id<MTLDepthStencilState> depthStencil;

        //FUNCTIONS
    public:
        MetalGraphicsPipelineObject() = delete;
        MetalGraphicsPipelineObject(Descriptor descriptor, id<MTLRenderPipelineState> pipeline, id<MTLDepthStencilState> depthStencil);

        MetalGraphicsPipelineObject(MetalGraphicsPipelineObject const &other) = delete;
        MetalGraphicsPipelineObject(MetalGraphicsPipelineObject &&other) noexcept;

        MetalGraphicsPipelineObject &operator=(MetalGraphicsPipelineObject const &other) = delete;
        MetalGraphicsPipelineObject &operator=(MetalGraphicsPipelineObject &&other) noexcept;

        ~MetalGraphicsPipelineObject();

        Descriptor const &getDescriptor() const override;

        id<MTLRenderPipelineState> getPipeline() const;
    };
}

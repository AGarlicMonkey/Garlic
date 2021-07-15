#pragma once

#include "Clove/Rendering/RenderGraph/RgId.hpp"

#include <Clove/Graphics/GhaGraphicsCommandBuffer.hpp>
#include <Clove/Graphics/GhaRenderPass.hpp>
#include <Clove/Graphics/GhaSampler.hpp>

namespace garlic::clove {
    struct RgBufferBinding {
        uint32_t slot{};
        RgResourceIdType buffer{};
    };

    struct RgImageBindng {
        uint32_t slot{};
        GhaSampler::Descriptor samplerState{};//TODO: RG version?
        RgResourceIdType image{};
    };

    struct RgRenderTargetBinding {
        LoadOperation loadOp{};
        StoreOperation storeOp{};
        ColourValue clearColour{};
        RgResourceIdType target{};
    };

    struct RgDepthStencilBinding {
        LoadOperation loadOp{};
        StoreOperation storeOp{};
        DepthStencilValue clearValue{};
        RgResourceIdType target{};
    };
}
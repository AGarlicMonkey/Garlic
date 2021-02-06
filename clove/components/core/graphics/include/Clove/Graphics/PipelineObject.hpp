#pragma once

#include "Clove/Graphics/GhaShader.hpp"

namespace garlic::clove {
    enum class PipelineStage {
        //External Stages
        Top,

        //Transfer stages
        Transfer,

        //Compute stages
        //...

        //Graphics stages
        PixelShader,
        EarlyPixelTest,
        ColourAttachmentOutput,
    };

    struct PushConstantDescriptor {
        GhaShader::Stage stage;
        size_t offset{ 0 }; /**< Offset specified in the shader (layout(offset = x)). Required if using different push constants for different stages. */
        size_t size{ 0 };
    };
}
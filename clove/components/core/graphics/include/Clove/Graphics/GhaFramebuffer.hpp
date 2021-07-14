#pragma once

#include <memory>
#include <vector>

namespace garlic::clove {
    class GhaRenderPass;
    class GhaImageView;
}

namespace garlic::clove {
    /**
     * @brief Provides attachments (as GraphicsImageViews) to a given GhaRenderPass.
     */
    class GhaFramebuffer {
        //TYPES
    public:
        struct Descriptor {
            std::shared_ptr<GhaRenderPass> renderPass;
            std::vector<std::shared_ptr<GhaImageView>> attachments; /**< The order of the attachments here has to match the those in the corresponding render pass. ColourAttachments + DepthStencilAttachment. */
            uint32_t width{ 0 };
            uint32_t height{ 0 };
        };

        //VARIABLES
    public:
        virtual ~GhaFramebuffer() = default;
    };
}

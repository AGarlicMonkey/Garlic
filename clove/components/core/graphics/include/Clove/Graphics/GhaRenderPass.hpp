#pragma once

#include "Clove/Graphics/GhaImage.hpp"
#include "Clove/Graphics/PipelineObject.hpp"
#include "Clove/Graphics/Resource.hpp"

#include <optional>
#include <vector>

namespace garlic::clove {
    enum class LoadOperation {
        Load,
        Clear,
        DontCare,
    };

    enum class StoreOperation {
        Store,
        DontCare,
    };

    /**
     * @brief Describes what happens to an attachment when it is loaded and stored.
     */
    struct AttachmentDescriptor {
        GhaImage::Format format{ GhaImage::Format::Unkown };

        LoadOperation loadOperation{ LoadOperation::DontCare };    /**< What to do with the attachment when it is loaded. */
        StoreOperation storeOperation{ StoreOperation::DontCare }; /**< How to store the attachment. */

        GhaImage::Layout initialLayout{ GhaImage::Layout::Undefined }; /**< What layout the render pass will expect the attachment to be in. */
        GhaImage::Layout usedLayout{ GhaImage::Layout::Undefined };    /**< What layout the attachment will transition to when used during the subpass. */
        GhaImage::Layout finalLayout{ GhaImage::Layout::Undefined };   /**< What layout the attachment will be transitioned to at the end of the render pass.*/
    };

    /**
     * @brief A GhaRenderPass describes a set of attachments that a GhaGraphicsPipelineStateObject can render to.
     * @details The actual attachments provided to the pipeline are stored in a GhaFramebuffer.
     */
    class GhaRenderPass {
        //TYPES
    public:
        struct Descriptor {
            std::vector<AttachmentDescriptor> colourAttachments{};
            AttachmentDescriptor depthAttachment{};
        };

        //FUNCTIONS
    public:
        virtual ~GhaRenderPass() = default;
    };
}
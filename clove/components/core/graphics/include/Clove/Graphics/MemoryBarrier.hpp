#pragma once

#include "Clove/Graphics/GhaImage.hpp"
#include "Clove/Graphics/Resource.hpp"

namespace garlic::clove {
    enum class QueueType {
        None,
        Graphics,
        Transfer,
        Compute
    };

    struct BufferMemoryBarrierInfo {
        AccessFlags currentAccess{ AccessFlags::None }; /**< How the buffer is currently being used / accessed. */
        AccessFlags newAccess{ AccessFlags::None };     /**< How the buffer is going to be used / accessed. */
        QueueType sourceQueue{ QueueType::None };       /**< If set signifies which queue this is being transfered from. */
        QueueType destinationQueue{ QueueType::None };  /**< If set signifies which queue this is being transfered to. */
    };

    struct ImageMemoryBarrierInfo {
        AccessFlags currentAccess{ AccessFlags::None };                     /**< How the image is currently being used / accessed. */
        AccessFlags newAccess{ AccessFlags::None };                         /**< How the image is going to be used / accessed. */
        GhaImage::Layout currentImageLayout{ GhaImage::Layout::Undefined }; /**< The current layout of the image. */
        GhaImage::Layout newImageLayout{ GhaImage::Layout::Undefined };     /**< The layout of the image to transition to after this barrier. */
        QueueType sourceQueue{ QueueType::None };                           /**< If set signifies which queue this is being transfered from. */
        QueueType destinationQueue{ QueueType::None };                      /**< If set signifies which queue this is being transfered to. */
        uint32_t baseArrayLayer{ 0 };                                       /**< The lower bound image array layer to include in the barrier. */
        uint32_t layerCount{ 1 };                                           /**< How many layers from the base layer to include in the barrier. */
    };
}
#pragma once

#include "Clove/Graphics/CommandBuffer.hpp"
#include "Clove/Graphics/GhaImage.hpp"
#include "Clove/Graphics/MemoryBarrier.hpp"
#include "Clove/Graphics/PipelineObject.hpp"

#include <Clove/Maths/Vector.hpp>

namespace garlic::clove {
    class GhaBuffer;
}

namespace garlic::clove {
    /**
     * @brief A buffer for recording transfer commands.
     * @details Allocated from a GhaTransferQueue.
     */
    class GhaTransferCommandBuffer {
        //FUNCTIONS
    public:
        virtual ~GhaTransferCommandBuffer() = default;

        /**
         * @brief Begin recording commands on the buffer. This will implicitly reset the buffer.
         * @param usageFlag 
         */
        virtual void beginRecording(CommandBufferUsage usageFlag) = 0;
        virtual void endRecording()                               = 0;

        virtual void copyBufferToBuffer(GhaBuffer &source, size_t const sourceOffset, GhaBuffer &destination, size_t const destinationOffset, size_t const sizeBytes)        = 0;
        virtual void copyBufferToImage(GhaBuffer &source, size_t const sourceOffset, GhaImage &destination, vec3i const &destinationOffset, vec3ui const &destinationExtent) = 0;

        virtual void copyImageToBuffer(GhaImage &source, vec3i const &sourceOffset, vec3ui const &sourceExtent, GhaBuffer &destination, size_t const destinationOffset) = 0;

        /**
         * @brief Creates a memory barrier for a buffer. Controlling execution order of commands on the buffer.
         * @details Any commands done on the buffer before the barrier are guarenteed to happen before commands
         * executed after the barrier. This barrier can also handle command queue transition.
         * @param buffer The buffer to create the barrier for.
         * @param barrierInfo The information about the barrier.
         * @param sourceStage The pipeline stage that gets executed before the barrier.
         * @param destinationStage The pipeline stage executed after the barrier that waits for the results of the sourceStage.
         */
        virtual void bufferMemoryBarrier(GhaBuffer &buffer, BufferMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) = 0;
        /**
         * @brief Creates a memory barrier for an image. Controlling execution order of commands on the image.
         * @details Any commands done on the image before the barrier are guarenteed to happen before commands
         * executed after the barrier. This barrier can also handle command queue and image layout transition.
         * @param image The image to create the barrier for.
         * @param barrierInfo The information about the barrier.
         * @param sourceStage The pipeline stage that gets executed before the barrier.
         * @param destinationStage The pipeline stage executed after the barrier that waits for the results of the sourceStage.
         */
        virtual void imageMemoryBarrier(GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) = 0;
    };
}
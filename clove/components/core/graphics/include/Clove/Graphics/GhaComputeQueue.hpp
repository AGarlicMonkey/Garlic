#pragma once

#include "Clove/Graphics/GhaComputeCommandBuffer.hpp"
#include "Clove/Graphics/GhaGraphicsPipelineObject.hpp"

namespace garlic::clove {
    class GhaFence;
    class GhaSemaphore;

    struct ComputeSubmitInfo {
        std::vector<std::pair<std::shared_ptr<GhaSemaphore>, PipelineStage>> waitSemaphores; /**< What semaphores to wait on at what stage */
        std::vector<std::shared_ptr<GhaComputeCommandBuffer>> commandBuffers;                /**< The command buffers to execute */
        std::vector<std::shared_ptr<GhaSemaphore>> signalSemaphores;                         /**< The semaphores that will be signaled when completed */
    };
}

namespace garlic::clove {
    /**
     * @brief Creates buffers that can be recorded for compute commands and then submitted.
     */
    class GhaComputeQueue {
        //FUNCTIONS
    public:
        virtual ~GhaComputeQueue() = default;

        virtual std::unique_ptr<GhaComputeCommandBuffer> allocateCommandBuffer() = 0;
        virtual void freeCommandBuffer(GhaComputeCommandBuffer &buffer)          = 0;

        /**
         * @brief Submit command buffers to be processed.
         * @details All buffers in a single submission will start in order but will likely finish out of order.
         * Batch them together like this if they can run at the same time. Each call to this submit function
         * will need to wait on previous submissions.
         * @param signalFence An optional fence that will be signaled when all submissions are complete.
         */
        virtual void submit(std::vector<ComputeSubmitInfo> const &submissions, GhaFence const *signalFence) = 0;
    };
}
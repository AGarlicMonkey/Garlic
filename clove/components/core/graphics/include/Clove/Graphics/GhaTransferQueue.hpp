#pragma once

#include "Clove/Graphics/GhaTransferCommandBuffer.hpp"
#include "Clove/Graphics/PipelineObject.hpp"

#include <vector>

namespace garlic::clove {
    class GhaFence;
    class GhaSemaphore;

    struct TransferSubmitInfo {
        std::vector<std::pair<std::shared_ptr<GhaSemaphore>, PipelineStage>> waitSemaphores; /**< What semaphores to wait on at what stage */
        std::vector<std::shared_ptr<GhaTransferCommandBuffer>> commandBuffers;               /**< The command buffers to execute */
        std::vector<std::shared_ptr<GhaSemaphore>> signalSemaphores;                         /**< The semaphores that will be signaled when completed */
    };
}

namespace garlic::clove {
    /**
     * @brief Creates buffers that can record transfer commands and then be submitted.
     */
    class GhaTransferQueue {
        //FUNCTIONS
    public:
        virtual ~GhaTransferQueue() = default;

        virtual std::unique_ptr<GhaTransferCommandBuffer> allocateCommandBuffer() = 0;
        virtual void freeCommandBuffer(GhaTransferCommandBuffer &buffer)          = 0;

        /**
         * @brief Submit command buffers to be processed.
         * @details All buffers in a single submission will start in order but will likely finish out of order.
         * Batch them together like this if they can run at the same time. Each call to this submit function
         * will need to wait on previous submissions.
         * @param signalFence An optional fence that will be signaled when all submissions are complete.
         */
        virtual void submit(std::vector<TransferSubmitInfo> const &submissions, GhaFence const *signalFence) = 0;
    };
}
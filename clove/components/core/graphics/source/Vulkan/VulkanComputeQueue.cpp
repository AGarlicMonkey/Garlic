#include "Clove/Graphics/Vulkan/VulkanComputeQueue.hpp"

#include "Clove/Graphics/Vulkan/VulkanComputeCommandBuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanFence.hpp"
#include "Clove/Graphics/Vulkan/VulkanPipelineObject.hpp"
#include "Clove/Graphics/Vulkan/VulkanSemaphore.hpp"

#include <Clove/Cast.hpp>
#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    VulkanComputeQueue::VulkanComputeQueue(DevicePointer device, VkQueue queue, VkCommandPool commandPool, QueueFamilyIndices queueFamilyIndices)
        : device{ std::move(device) }
        , queue{ queue }
        , commandPool{ commandPool }
        , queueFamilyIndices{ queueFamilyIndices } {
    }

    VulkanComputeQueue::VulkanComputeQueue(VulkanComputeQueue &&other) noexcept = default;

    VulkanComputeQueue &VulkanComputeQueue::operator=(VulkanComputeQueue &&other) noexcept = default;

    VulkanComputeQueue::~VulkanComputeQueue() {
        vkDestroyCommandPool(device.get(), commandPool, nullptr);
    }

    std::unique_ptr<GhaComputeCommandBuffer> VulkanComputeQueue::allocateCommandBuffer() {
        VkCommandBuffer commandBuffer{ nullptr };

        VkCommandBufferAllocateInfo allocInfo{
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = commandPool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        if(vkAllocateCommandBuffers(device.get(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Failed to allocate command buffer");
            return nullptr;
        }

        return std::make_unique<VulkanComputeCommandBuffer>(commandBuffer, queueFamilyIndices);
    }

    void VulkanComputeQueue::freeCommandBuffer(GhaComputeCommandBuffer &buffer) {
        VkCommandBuffer const commandBuffer{ polyCast<VulkanComputeCommandBuffer>(&buffer)->getCommandBuffer() };
        vkFreeCommandBuffers(device.get(), commandPool, 1, &commandBuffer);
    }

    void VulkanComputeQueue::submit(std::vector<ComputeSubmitInfo> const &submissions, GhaFence *signalFence) {
        auto const submissioncount{ std::size(submissions) };
        std::vector<VkSubmitInfo> vkSubmissions;
        vkSubmissions.reserve(submissioncount);

        std::vector<std::vector<VkSemaphore>> waitSemaphores(submissioncount);
        std::vector<std::vector<VkPipelineStageFlags>> waitStages(submissioncount);
        std::vector<std::vector<VkCommandBuffer>> commandBuffers(submissioncount);
        std::vector<std::vector<VkSemaphore>> signalSemaphores(submissioncount);

        for(size_t i{ 0 }; i < submissioncount; ++i) {
            //Wait semaphores / stages
            size_t const waitSemaphoreCount{ std::size(submissions[i].waitSemaphores) };
            waitSemaphores[i].resize(waitSemaphoreCount);
            waitStages[i].resize(waitSemaphoreCount);

            for(size_t j = 0; j < waitSemaphoreCount; ++j) {
                waitSemaphores[i][j] = polyCast<VulkanSemaphore>(submissions[i].waitSemaphores[j].first.get())->getSemaphore();
                waitStages[i][j]     = convertStage(submissions[i].waitSemaphores[j].second);
            }

            //Command buffers
            size_t const commandBufferCount{ std::size(submissions[i].commandBuffers) };
            commandBuffers[i].resize(commandBufferCount);
            for(size_t j = 0; j < commandBufferCount; ++j) {
                commandBuffers[i][j] = polyCast<VulkanComputeCommandBuffer>(submissions[i].commandBuffers[j].get())->getCommandBuffer();
            }

            //Signal semaphores
            size_t const signalSemaphoreCount{ std::size(submissions[i].signalSemaphores) };
            signalSemaphores[i].resize(signalSemaphoreCount);
            for(size_t j = 0; j < signalSemaphoreCount; ++j) {
                signalSemaphores[i][j] = polyCast<VulkanSemaphore>(submissions[i].signalSemaphores[j].get())->getSemaphore();
            }

            vkSubmissions.emplace_back(VkSubmitInfo{
                .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount   = static_cast<uint32_t>(waitSemaphoreCount),
                .pWaitSemaphores      = waitSemaphores[i].data(),
                .pWaitDstStageMask    = waitStages[i].data(),
                .commandBufferCount   = static_cast<uint32_t>(commandBufferCount),
                .pCommandBuffers      = commandBuffers[i].data(),
                .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphoreCount),
                .pSignalSemaphores    = signalSemaphores[i].data(),
            });
        }

        VkFence const vkFence{ signalFence != nullptr ? polyCast<VulkanFence const>(signalFence)->getFence() : VK_NULL_HANDLE };

        if(vkQueueSubmit(queue, vkSubmissions.size(), vkSubmissions.data(), vkFence) != VK_SUCCESS) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Failed to submit compute command buffer(s)");
        }
    }
}

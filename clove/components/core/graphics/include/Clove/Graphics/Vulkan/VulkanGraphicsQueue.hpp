#pragma once

#include "Clove/Graphics/GhaGraphicsQueue.hpp"
#include "Clove/Graphics/Vulkan/DevicePointer.hpp"
#include "Clove/Graphics/Vulkan/VulkanTypes.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    class VulkanGraphicsQueue : public GhaGraphicsQueue {
        //VARIABLES
    private:
        DevicePointer device;
        VkQueue queue{ VK_NULL_HANDLE };
        VkCommandPool commandPool{ VK_NULL_HANDLE };

        QueueFamilyIndices queueFamilyIndices;

        //FUNCTIONS
    public:
        VulkanGraphicsQueue() = delete;
        VulkanGraphicsQueue(DevicePointer device, VkQueue queue, VkCommandPool commandPool, QueueFamilyIndices queueFamilyIndices);

        VulkanGraphicsQueue(VulkanGraphicsQueue const &other) = delete;
        VulkanGraphicsQueue(VulkanGraphicsQueue &&other) noexcept;

        VulkanGraphicsQueue &operator=(VulkanGraphicsQueue const &other) = delete;
        VulkanGraphicsQueue &operator=(VulkanGraphicsQueue &&other) noexcept;

        ~VulkanGraphicsQueue();

        std::unique_ptr<GhaGraphicsCommandBuffer> allocateCommandBuffer() override;
        void freeCommandBuffer(GhaGraphicsCommandBuffer &buffer) override;

        void submit(std::vector<GraphicsSubmitInfo> const &submissions, GhaFence *signalFence) override;
    };
}

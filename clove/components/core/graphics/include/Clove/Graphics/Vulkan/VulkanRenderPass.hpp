#pragma once

#include "Clove/Graphics/GhaRenderPass.hpp"
#include "Clove/Graphics/Vulkan/DevicePointer.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    class VulkanRenderPass : public GhaRenderPass {
        //VARIABLES
    private:
        Descriptor descriptor{};

        DevicePointer device;

        VkRenderPass renderPass{ VK_NULL_HANDLE };

        //FUNCTIONS
    public:
        VulkanRenderPass() = delete;
        VulkanRenderPass(Descriptor descriptor, DevicePointer device, VkRenderPass renderPass);

        VulkanRenderPass(VulkanRenderPass const &other) = delete;
        VulkanRenderPass(VulkanRenderPass &&other) noexcept;

        VulkanRenderPass &operator=(VulkanRenderPass const &other) = delete;
        VulkanRenderPass &operator=(VulkanRenderPass &&other) noexcept;

        ~VulkanRenderPass();

        Descriptor const &getDescriptor() const override;

        VkRenderPass getRenderPass() const;
    };
}
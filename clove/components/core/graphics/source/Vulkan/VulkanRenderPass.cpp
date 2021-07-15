#include "Clove/Graphics/Vulkan/VulkanRenderPass.hpp"

#include "Clove/Graphics/Vulkan/VulkanGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Vulkan/VulkanImage.hpp"

#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    VulkanRenderPass::VulkanRenderPass(Descriptor descriptor, DevicePointer device, VkRenderPass renderPass)
        : descriptor{ std::move(descriptor) } 
        , device{ std::move(device) }
        , renderPass{ renderPass } {
    }

    VulkanRenderPass::VulkanRenderPass(VulkanRenderPass &&other) noexcept = default;

    VulkanRenderPass &VulkanRenderPass::operator=(VulkanRenderPass &&other) noexcept = default;

    VulkanRenderPass::~VulkanRenderPass() {
        vkDestroyRenderPass(device.get(), renderPass, nullptr);
    }

    VulkanRenderPass::Descriptor const &VulkanRenderPass::getDescriptor() const {
        return descriptor;
    }

    VkRenderPass VulkanRenderPass::getRenderPass() const {
        return renderPass;
    }
}
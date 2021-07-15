#include "Clove/Graphics/Vulkan/VulkanGraphicsPipelineObject.hpp"

namespace garlic::clove {
    VulkanGraphicsPipelineObject::VulkanGraphicsPipelineObject(Descriptor descriptor, DevicePointer device, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
        : descriptor{ std::move(descriptor) }
        , device{ std::move(device) }
        , pipeline{ pipeline }
        , pipelineLayout{ pipelineLayout } {
    }

    VulkanGraphicsPipelineObject::VulkanGraphicsPipelineObject(VulkanGraphicsPipelineObject &&other) noexcept = default;

    VulkanGraphicsPipelineObject &VulkanGraphicsPipelineObject::operator=(VulkanGraphicsPipelineObject &&other) noexcept = default;

    VulkanGraphicsPipelineObject::~VulkanGraphicsPipelineObject() {
        vkDestroyPipeline(device.get(), pipeline, nullptr);
        vkDestroyPipelineLayout(device.get(), pipelineLayout, nullptr);
    }

    VulkanGraphicsPipelineObject::Descriptor const &VulkanGraphicsPipelineObject::getDescriptor() const {
        return descriptor;
    }

    VkPipeline VulkanGraphicsPipelineObject::getPipeline() const {
        return pipeline;
    }

    VkPipelineLayout VulkanGraphicsPipelineObject::getLayout() const {
        return pipelineLayout;
    }
}
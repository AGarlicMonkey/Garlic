#pragma once

#include "Clove/Graphics/GhaGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Vulkan/DevicePointer.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    class VulkanGraphicsPipelineObject : public GhaGraphicsPipelineObject {
        //VARIABLES
    private:
        Descriptor descriptor{};

        DevicePointer device;

        VkPipeline pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

        //FUNCTIONS
    public:
        VulkanGraphicsPipelineObject() = delete;
        VulkanGraphicsPipelineObject(Descriptor descriptor, DevicePointer device, VkPipeline pipeline, VkPipelineLayout pipelineLayout);

        VulkanGraphicsPipelineObject(VulkanGraphicsPipelineObject const &other) = delete;
        VulkanGraphicsPipelineObject(VulkanGraphicsPipelineObject &&other) noexcept;

        VulkanGraphicsPipelineObject &operator=(VulkanGraphicsPipelineObject const &other) = delete;
        VulkanGraphicsPipelineObject &operator=(VulkanGraphicsPipelineObject &&other) noexcept;

        ~VulkanGraphicsPipelineObject();

        Descriptor const &getDescriptor() const override;

        VkPipeline getPipeline() const;
        VkPipelineLayout getLayout() const;
    };
}
#pragma once

#include "Clove/Graphics/GhaDescriptorSet.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    class VulkanDescriptorSet : public GhaDescriptorSet {
        //VARIABLES
    private:
        VkDevice device = VK_NULL_HANDLE;

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        //FUNCTIONS
    public:
        VulkanDescriptorSet() = delete;
        VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet);

        VulkanDescriptorSet(VulkanDescriptorSet const &other) = delete;
        VulkanDescriptorSet(VulkanDescriptorSet &&other) noexcept;

        VulkanDescriptorSet &operator=(VulkanDescriptorSet const &other) = delete;
        VulkanDescriptorSet &operator=(VulkanDescriptorSet &&other) noexcept;

        ~VulkanDescriptorSet();

        void map(GhaBuffer const &buffer, size_t const offset, size_t const range, DescriptorType const descriptorType, uint32_t const bindingSlot) override;

        void map(GhaImageView const &imageView, GhaImage::Layout const layout, uint32_t const bindingSlot) override;

        void map(GhaSampler const &sampler, uint32_t const bindingSlot) override;

        VkDescriptorSet getDescriptorSet() const;
    };
}
#pragma once

#include "Clove/Graphics/DescriptorSet.hpp"

#include <vulkan/vulkan.h>

namespace clv::gfx::vk {
    class VKDescriptorSet : public DescriptorSet {
        //VARIABLES
    private:
        VkDevice device = VK_NULL_HANDLE;

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        //FUNCTIONS
    public:
        VKDescriptorSet() = delete;
        VKDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet);

        VKDescriptorSet(const VKDescriptorSet& other) = delete;
        VKDescriptorSet(VKDescriptorSet&& other) noexcept;

        VKDescriptorSet& operator=(const VKDescriptorSet& other) = delete;
        VKDescriptorSet& operator=(VKDescriptorSet&& other) noexcept;

        ~VKDescriptorSet();

        void map(const GraphicsBuffer& buffer, const size_t offset, const size_t range, const uint32_t bindingSlot) override;

        void map(const GraphicsImageView& imageView, const Sampler& sampler, const GraphicsImage::Layout layout, const uint32_t bindingSlot) override;
        void map(std::span<std::shared_ptr<GraphicsImageView>> imageViews, const Sampler& sampler, const GraphicsImage::Layout layout, const uint32_t bindingSlot) override;

        VkDescriptorSet getDescriptorSet() const;
    };
}
#include "Clove/Graphics/Vulkan/VulkanDescriptorSet.hpp"

#include "Clove/Graphics/Vulkan/VulkanBuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanImage.hpp"
#include "Clove/Graphics/Vulkan/VulkanImageView.hpp"
#include "Clove/Graphics/Vulkan/VulkanSampler.hpp"
#include "Clove/Graphics/Vulkan/VulkanDescriptor.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
    VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet)
        : device(device)
        , descriptorSet(descriptorSet) {
    }

    VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet &&other) noexcept = default;

    VulkanDescriptorSet &VulkanDescriptorSet::operator=(VulkanDescriptorSet &&other) noexcept = default;

    VulkanDescriptorSet::~VulkanDescriptorSet() = default;

    void VulkanDescriptorSet::map(GhaBuffer const &buffer, size_t const offset, size_t const range, DescriptorType const descriptorType, uint32_t const bindingSlot) {
        auto const *vkBuffer = polyCast<VulkanBuffer const>(&buffer);

        VkDescriptorBufferInfo bufferInfo{
            .buffer = vkBuffer->getBuffer(),
            .offset = offset,
            .range  = range,
        };

        VkWriteDescriptorSet writeInfo{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptorSet,
            .dstBinding       = bindingSlot,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = getDescriptorType(descriptorType),
            .pImageInfo       = nullptr,
            .pBufferInfo      = &bufferInfo,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
    }

    void VulkanDescriptorSet::map(GhaImageView const &imageView, GhaSampler const &sampler, GhaImage::Layout const layout, uint32_t const bindingSlot) {
        auto const *vkSampler{ polyCast<VulkanSampler const>(&sampler) };
        auto const *vkImageView{ polyCast<VulkanImageView const>(&imageView) };

        VkDescriptorImageInfo imageInfo{
            .sampler     = vkSampler->getSampler(),
            .imageView   = vkImageView->getImageView(),
            .imageLayout = VulkanImage::convertLayout(layout),
        };

        VkWriteDescriptorSet writeInfo{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptorSet,
            .dstBinding       = bindingSlot,
            .dstArrayElement  = 0,
            .descriptorCount  = 1,
            .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo       = &imageInfo,
            .pBufferInfo      = nullptr,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
    }

    void VulkanDescriptorSet::map(std::span<std::shared_ptr<GhaImageView>> imageViews, GhaSampler const &sampler, GhaImage::Layout const layout, uint32_t const bindingSlot) {
        auto const *vkSampler{ polyCast<VulkanSampler const>(&sampler) };
        std::vector<VkDescriptorImageInfo> imageInfos(std::size(imageViews));

        for(size_t i = 0; i < std::size(imageInfos); ++i) {
            VkDescriptorImageInfo imageInfo{
                .sampler     = vkSampler->getSampler(),
                .imageView   = polyCast<const VulkanImageView>(imageViews[i].get())->getImageView(),
                .imageLayout = VulkanImage::convertLayout(layout),
            };
            imageInfos[i] = imageInfo;
        }

        VkWriteDescriptorSet writeInfo{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = descriptorSet,
            .dstBinding       = bindingSlot,
            .dstArrayElement  = 0,
            .descriptorCount  = static_cast<uint32_t>(std::size(imageInfos)),
            .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo       = std::data(imageInfos),
            .pBufferInfo      = nullptr,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(device, 1, &writeInfo, 0, nullptr);
    }

    VkDescriptorSet VulkanDescriptorSet::getDescriptorSet() const {
        return descriptorSet;
    }
}
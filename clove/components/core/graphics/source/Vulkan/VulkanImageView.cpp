#include "Clove/Graphics/Vulkan/VulkanImageView.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    VulkanImageView::VulkanImageView(VkDevice device, VkImageView imageView)
        : device(device)
        , imageView(imageView) {
    }

    VulkanImageView::VulkanImageView(VulkanImageView &&other) noexcept = default;

    VulkanImageView &VulkanImageView::operator=(VulkanImageView &&other) noexcept = default;

    VulkanImageView::~VulkanImageView() {
        vkDestroyImageView(device, imageView, nullptr);
    }

    VkImageView VulkanImageView::create(VkDevice device, VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t const baseLayer, uint32_t const layerCount) {
        VkImageViewCreateInfo viewInfo{
            .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext      = nullptr,
            .flags      = 0u,
            .image      = image,
            .viewType   = viewType,
            .format     = format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask     = aspectFlags,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = baseLayer,
                .layerCount     = layerCount,
            }
        };

        VkImageView imageView{ nullptr };
        if(vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Failed to create texture image view");
            return VK_NULL_HANDLE;
        }

        return imageView;
    }
}
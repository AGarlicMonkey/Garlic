#include "Clove/Graphics/Vulkan/VulkanImageView.hpp"

#include "Clove/Graphics/GhaImageView.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    VulkanImageView::VulkanImageView(GhaImage::Format viewedFormat, vec2ui viewedDimensions, VkDevice device, VkImageView imageView)
        : viewedFormat{ viewedFormat }
        , viewedDimensions{ std::move(viewedDimensions) }
        , device{ device }
        , imageView{ imageView } {
    }

    VulkanImageView::VulkanImageView(VulkanImageView &&other) noexcept = default;

    VulkanImageView &VulkanImageView::operator=(VulkanImageView &&other) noexcept = default;

    VulkanImageView::~VulkanImageView() {
        vkDestroyImageView(device, imageView, nullptr);
    }

    GhaImage::Format VulkanImageView::getImageFormat() const {
        return viewedFormat;
    }

    vec2ui const &VulkanImageView::getImageDimensions() const {
        return viewedDimensions;
    }

    VkImageViewType VulkanImageView::convertType(GhaImageView::Type garlicImageType) {
        switch(garlicImageType) {
            case GhaImageView::Type::_2D:
                return VK_IMAGE_VIEW_TYPE_2D;
            case GhaImageView::Type::_3D:
                return VK_IMAGE_VIEW_TYPE_3D;
            case GhaImageView::Type::Cube:
                return VK_IMAGE_VIEW_TYPE_CUBE;
            default:
                CLOVE_ASSERT(false, "{0}: Unhandled image type");
                return VK_IMAGE_VIEW_TYPE_2D;
        }
    }
}
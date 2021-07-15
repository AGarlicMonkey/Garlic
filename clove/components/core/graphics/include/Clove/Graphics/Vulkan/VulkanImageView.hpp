#pragma once

#include "Clove/Graphics/GhaImageView.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    class VulkanImageView : public GhaImageView {
        //VARIABLES
    private:
        GhaImage::Format viewedFormat{};
        vec2ui viewedDimensions{};

        VkDevice device{ VK_NULL_HANDLE };

        VkImageView imageView{ VK_NULL_HANDLE };

        //FUNCTIONS
    public:
        VulkanImageView() = delete;
        VulkanImageView(GhaImage::Format viewedFormat, vec2ui viewedDimensions, VkDevice device, VkImageView imageView);

        VulkanImageView(VulkanImageView const &other) = delete;
        VulkanImageView(VulkanImageView &&other) noexcept;

        VulkanImageView &operator=(VulkanImageView const &other) = delete;
        VulkanImageView &operator=(VulkanImageView &&other) noexcept;

        ~VulkanImageView();

        GhaImage::Format getImageFormat() const override;
        vec2ui const &getImageDimensions() const override;

        inline VkImageView getImageView() const;

        static VkImageViewType convertType(GhaImageView::Type garlicImageType);
    };
}

#include "VulkanImageView.inl"
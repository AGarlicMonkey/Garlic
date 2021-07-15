#include "Clove/Graphics/Vulkan/VulkanSwapchain.hpp"

#include "Clove/Graphics/Vulkan/VulkanImage.hpp"
#include "Clove/Graphics/Vulkan/VulkanImageView.hpp"
#include "Clove/Graphics/Vulkan/VulkanResult.hpp"
#include "Clove/Graphics/Vulkan/VulkanSemaphore.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
    VulkanSwapchain::VulkanSwapchain(DevicePointer device, VkSwapchainKHR swapchain, VkFormat swapChainImageFormat, VkExtent2D swapChainExtent, std::vector<std::shared_ptr<VulkanImageView>> imageViews)
        : device{ std::move(device) }
        , swapchain{ swapchain }
        , swapChainImageFormat{ swapChainImageFormat }
        , swapChainExtent{ swapChainExtent }
        , imageViews{ std::move(imageViews) } {
    }

    VulkanSwapchain::VulkanSwapchain(VulkanSwapchain &&other) noexcept = default;

    VulkanSwapchain &VulkanSwapchain::operator=(VulkanSwapchain &&other) noexcept = default;

    VulkanSwapchain::~VulkanSwapchain() {
        vkDestroySwapchainKHR(device.get(), swapchain, nullptr);
    }

    std::pair<uint32_t, Result> VulkanSwapchain::aquireNextImage(GhaSemaphore const *availableSemaphore) {
        uint32_t outImageIndex{ 0 };
        VkSemaphore vkSemaphore{ availableSemaphore != nullptr ? polyCast<const VulkanSemaphore>(availableSemaphore)->getSemaphore() : VK_NULL_HANDLE };
        VkResult const result{ vkAcquireNextImageKHR(device.get(), swapchain, UINT64_MAX, vkSemaphore, VK_NULL_HANDLE, &outImageIndex) };

        return { outImageIndex, convertResult(result) };
    }

    GhaImage::Format VulkanSwapchain::getImageFormat() const {
        return VulkanImage::convertFormat(swapChainImageFormat);
    }

    vec2ui VulkanSwapchain::getSize() const {
        return { swapChainExtent.width, swapChainExtent.height };
    }

    std::vector<std::shared_ptr<GhaImageView>> VulkanSwapchain::getImageViews() const {
        return { imageViews.begin(), imageViews.end() };
    }

    VkSwapchainKHR VulkanSwapchain::getSwapchain() const {
        return swapchain;
    }
}
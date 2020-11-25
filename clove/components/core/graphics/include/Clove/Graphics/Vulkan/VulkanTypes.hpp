#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace garlic::clove {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool isComplete(bool const requirePresentFamily) const {
            return graphicsFamily && (presentFamily || !requirePresentFamily) && transferFamily;
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
}
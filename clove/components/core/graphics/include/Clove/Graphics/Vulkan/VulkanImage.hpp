#pragma once

#include "Clove/Graphics/GhaImage.hpp"
#include "Clove/Graphics/Vulkan/DevicePointer.hpp"
#include "Clove/Graphics/Vulkan/MemoryAllocator.hpp"
#include "Clove/Graphics/Vulkan/VulkanTypes.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    struct QueueFamilyIndices;
}

namespace garlic::clove {
    class VulkanImage : public GhaImage {
        //VARIABLES
    private:
        DevicePointer device;

        VkImage image{ VK_NULL_HANDLE };

        Descriptor descriptor{};

        std::shared_ptr<MemoryAllocator> memoryAllocator;
        const MemoryAllocator::Chunk* allocatedBlock{ nullptr };

        //FUNCTIONS
    public:
        VulkanImage() = delete;
        VulkanImage(DevicePointer device, VkImage image, Descriptor descriptor, std::shared_ptr<MemoryAllocator> memoryAllocator);

        VulkanImage(VulkanImage const& other) = delete;
        VulkanImage(VulkanImage&& other) noexcept;

        VulkanImage& operator=(VulkanImage const& other) = delete;
        VulkanImage& operator=(VulkanImage&& other) noexcept;

        ~VulkanImage();

        Descriptor const &getDescriptor() const override;

        inline VkImage getImage() const;

        static Format convertFormat(VkFormat vulkanFormat);
        static VkFormat convertFormat(Format garlicFormat);

        static VkImageLayout convertLayout(Layout garlicLayout);
    };
}

#include "VulkanImage.inl"
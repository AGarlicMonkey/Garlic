#include "Clove/Graphics/Vulkan/VulkanImage.hpp"

#include "Clove/Graphics/Vulkan/VulkanImageView.hpp"
#include "Clove/Graphics/Vulkan/VulkanResource.hpp"

#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>
#include <array>

namespace garlic::clove {
    VulkanImage::VulkanImage(DevicePointer device, VkImage image, Descriptor descriptor, std::shared_ptr<MemoryAllocator> memoryAllocator)
        : device{ std::move(device) }
        , image{ image }
        , descriptor{ descriptor }
        , memoryAllocator{ std::move(memoryAllocator) } {
        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(this->device.get(), image, &memoryRequirements);

        allocatedBlock = this->memoryAllocator->allocate(memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkBindImageMemory(this->device.get(), image, allocatedBlock->memory, allocatedBlock->offset);
    }

    VulkanImage::VulkanImage(VulkanImage &&other) noexcept = default;

    VulkanImage &VulkanImage::operator=(VulkanImage &&other) noexcept = default;

    VulkanImage::~VulkanImage() {
        vkDestroyImage(device.get(), image, nullptr);
        memoryAllocator->free(allocatedBlock);
    }

    VulkanImage::Descriptor const &VulkanImage::getDescriptor() const {
        return descriptor;
    }

    GhaImage::Format VulkanImage::convertFormat(VkFormat vulkanFormat) {
        switch(vulkanFormat) {
            case VK_FORMAT_R8_UNORM:
                return Format::R8_UNORM;
            case VK_FORMAT_R8G8B8A8_SRGB:
                return Format::R8G8B8A8_SRGB;
            case VK_FORMAT_B8G8R8A8_SRGB:
                return Format::B8G8R8A8_SRGB;
            case VK_FORMAT_B8G8R8A8_UNORM:
                return Format::B8G8R8A8_UNORM;
            case VK_FORMAT_D32_SFLOAT:
                return Format::D32_SFLOAT;
            default:
                CLOVE_ASSERT(false, "{0}: Format not supported by garlic", CLOVE_FUNCTION_NAME);
                return Format::Unkown;
        }
    }

    VkFormat VulkanImage::convertFormat(Format garlicFormat) {
        switch(garlicFormat) {
            case Format::Unkown:
                return VK_FORMAT_UNDEFINED;
            case Format::R8_UNORM:
                return VK_FORMAT_R8_UNORM;
            case Format::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case Format::B8G8R8A8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::B8G8R8A8_UNORM:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case Format::D32_SFLOAT:
                return VK_FORMAT_D32_SFLOAT;
            default:
                CLOVE_ASSERT(false, "{0}: Unkown format", CLOVE_FUNCTION_NAME);
                return VK_FORMAT_UNDEFINED;
        }
    }

    VkImageLayout VulkanImage::convertLayout(Layout garlicLayout) {
        switch(garlicLayout) {
            case Layout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case Layout::General:
                return VK_IMAGE_LAYOUT_GENERAL;
            case Layout::Present:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case Layout::TransferSourceOptimal:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case Layout::TransferDestinationOptimal:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case Layout::ShaderReadOnlyOptimal:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case Layout::ColourAttachmentOptimal:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case Layout::DepthStencilAttachmentOptimal:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case Layout::DepthStencilReadOnlyOptimal:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            default:
                CLOVE_ASSERT(false, "{0}: Unkown image layout", CLOVE_FUNCTION_NAME);
                return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }
}
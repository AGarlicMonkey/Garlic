#include "Clove/Graphics/Vulkan/VulkanCommandBuffer.hpp"

#include "Clove/Graphics/Vulkan/VulkanBuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanImage.hpp"
#include "Clove/Graphics/Vulkan/VulkanResource.hpp"

#include <Clove/Cast.hpp>
#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    VkCommandBufferUsageFlags getCommandBufferUsageFlags(CommandBufferUsage garlicUsage) {
        switch(garlicUsage) {
            case garlic::clove::CommandBufferUsage::Default:
                return 0;
            case garlic::clove::CommandBufferUsage::OneTimeSubmit:
                return VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            default:
                CLOVE_ASSERT(false, "{0}: Unkown usage type", CLOVE_FUNCTION_NAME);
                return 0;
        }
    }

    void createBufferMemoryBarrier(VkCommandBuffer vkCommandBuffer, QueueFamilyIndices const &queueFamilyIndices, GhaBuffer &buffer, BufferMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
        uint32_t const sourceFamilyIndex{ getQueueFamilyIndex(barrierInfo.sourceQueue, queueFamilyIndices) };
        uint32_t const destinationFamilyIndex{ getQueueFamilyIndex(barrierInfo.destinationQueue, queueFamilyIndices) };

        VkBufferMemoryBarrier barrier{
            .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = convertAccessFlags(barrierInfo.currentAccess),
            .dstAccessMask       = convertAccessFlags(barrierInfo.newAccess),
            .srcQueueFamilyIndex = sourceFamilyIndex,
            .dstQueueFamilyIndex = destinationFamilyIndex,
            .buffer              = polyCast<VulkanBuffer>(&buffer)->getBuffer(),
            .offset              = 0,
            .size                = VK_WHOLE_SIZE,
        };

        VkPipelineStageFlags const vkSourceStage{ convertStage(sourceStage) };
        VkPipelineStageFlags const vkDestinationStage{ convertStage(destinationStage) };

        vkCmdPipelineBarrier(vkCommandBuffer, vkSourceStage, vkDestinationStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    }

    void createImageMemoryBarrier(VkCommandBuffer vkCommandBuffer, QueueFamilyIndices const &queueFamilyIndices, GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
        GhaImage::Descriptor const &imageDescriptor{ image.getDescriptor() };

        uint32_t const sourceFamilyIndex{ getQueueFamilyIndex(barrierInfo.sourceQueue, queueFamilyIndices) };
        uint32_t const destinationFamilyIndex{ getQueueFamilyIndex(barrierInfo.destinationQueue, queueFamilyIndices) };

        VkImageAspectFlags const aspectFlags{ static_cast<VkImageAspectFlags>(imageDescriptor.format == GhaImage::Format::D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT) };

        VkImageMemoryBarrier const barrier{
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = convertAccessFlags(barrierInfo.currentAccess),
            .dstAccessMask       = convertAccessFlags(barrierInfo.newAccess),
            .oldLayout           = VulkanImage::convertLayout(barrierInfo.currentImageLayout),
            .newLayout           = VulkanImage::convertLayout(barrierInfo.newImageLayout),
            .srcQueueFamilyIndex = sourceFamilyIndex,
            .dstQueueFamilyIndex = destinationFamilyIndex,
            .image               = polyCast<VulkanImage>(&image)->getImage(),
            .subresourceRange    = {
                .aspectMask     = aspectFlags,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = barrierInfo.baseArrayLayer,
                .layerCount     = barrierInfo.layerCount,
            },
        };

        VkPipelineStageFlags const vkSourceStage{ convertStage(sourceStage) };
        VkPipelineStageFlags const vkDestinationStage{ convertStage(destinationStage) };

        vkCmdPipelineBarrier(vkCommandBuffer, vkSourceStage, vkDestinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }
}
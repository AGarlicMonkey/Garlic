#include "Clove/Graphics/Vulkan/VulkanTransferCommandBuffer.hpp"

#include "Clove/Graphics/Vulkan/VulkanBuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanCommandBuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Vulkan/VulkanImage.hpp"

#include <Clove/Cast.hpp>
#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    VulkanTransferCommandBuffer::VulkanTransferCommandBuffer(VkCommandBuffer commandBuffer, QueueFamilyIndices queueFamilyIndices)
        : commandBuffer(commandBuffer)
        , queueFamilyIndices(queueFamilyIndices) {
    }

    VulkanTransferCommandBuffer::VulkanTransferCommandBuffer(VulkanTransferCommandBuffer &&other) noexcept = default;

    VulkanTransferCommandBuffer &VulkanTransferCommandBuffer::operator=(VulkanTransferCommandBuffer &&other) noexcept = default;

    VulkanTransferCommandBuffer::~VulkanTransferCommandBuffer() = default;

    void VulkanTransferCommandBuffer::beginRecording(CommandBufferUsage usageFlag) {
        VkCommandBufferBeginInfo beginInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags            = getCommandBufferUsageFlags(usageFlag),
            .pInheritanceInfo = nullptr,
        };

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Failed to begin recording command buffer");
        }
    }

    void VulkanTransferCommandBuffer::endRecording() {
        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Failed to end recording command buffer");
        }
    }

    void VulkanTransferCommandBuffer::copyBufferToBuffer(GhaBuffer &source, size_t const sourceOffset, GhaBuffer &destination, size_t const destinationOffset, size_t const sizeBytes) {
        VkBufferCopy copyRegion{
            .srcOffset = sourceOffset,
            .dstOffset = destinationOffset,
            .size      = sizeBytes,
        };

        vkCmdCopyBuffer(commandBuffer, polyCast<VulkanBuffer>(&source)->getBuffer(), polyCast<VulkanBuffer>(&destination)->getBuffer(), 1, &copyRegion);
    }

    void VulkanTransferCommandBuffer::copyBufferToImage(GhaBuffer &source, size_t const sourceOffset, GhaImage &destination, vec3i const &destinationOffset, vec3ui const &destinationExtent) {
        VkBufferImageCopy copyRegion{
            .bufferOffset      = sourceOffset,
            .bufferRowLength   = 0,//Tightly packed
            .bufferImageHeight = 0,//Tightly packed
            .imageSubresource  = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,//TODO: Handle other aspect masks
                .mipLevel       = 0,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
            .imageOffset = { destinationOffset.x, destinationOffset.y, destinationOffset.z },
            .imageExtent = { destinationExtent.x, destinationExtent.y, destinationExtent.z },
        };

        vkCmdCopyBufferToImage(commandBuffer, polyCast<VulkanBuffer>(&source)->getBuffer(), polyCast<VulkanImage>(&destination)->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    void VulkanTransferCommandBuffer::copyImageToBuffer(GhaImage &source, vec3i const &sourceOffset, vec3ui const &sourceExtent, GhaBuffer &destination, size_t const destinationOffset) {
        VkBufferImageCopy copyRegion{
            .bufferOffset      = destinationOffset,
            .bufferRowLength   = 0,//Tightly packed
            .bufferImageHeight = 0,//Tightly packed
            .imageSubresource  = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,//TODO: Handle other aspect masks
                .mipLevel       = 0,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
            .imageOffset = { sourceOffset.x, sourceOffset.y, sourceOffset.z },
            .imageExtent = { sourceExtent.x, sourceExtent.y, sourceExtent.z },
        };

        vkCmdCopyImageToBuffer(commandBuffer, polyCast<VulkanImage>(&source)->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, polyCast<VulkanBuffer>(&destination)->getBuffer(), 1, &copyRegion);
    }

    void VulkanTransferCommandBuffer::bufferMemoryBarrier(GhaBuffer &buffer, BufferMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
        createBufferMemoryBarrier(commandBuffer, queueFamilyIndices, buffer, barrierInfo, sourceStage, destinationStage);
    }

    void VulkanTransferCommandBuffer::imageMemoryBarrier(GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
        bool const isValidLayout =
            barrierInfo.newImageLayout != GhaImage::Layout::ShaderReadOnlyOptimal &&
            barrierInfo.newImageLayout != GhaImage::Layout::ColourAttachmentOptimal &&
            barrierInfo.newImageLayout != GhaImage::Layout::DepthStencilAttachmentOptimal &&
            barrierInfo.newImageLayout != GhaImage::Layout::DepthStencilReadOnlyOptimal;

        if(!isValidLayout) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "{0}: Invalid newLayout. This command buffer cannot handle transfering images to the following layouts:\n\tImageLayout::ShaderReadOnlyOptimal\n\tImageLayout::ColourAttachmentOptimal\n\tImageLayout::DepthStencilAttachmentOptimal\n\tImageLayout::DepthStencilReadOnlyOptimal", CLOVE_FUNCTION_NAME);
            return;
        }

        createImageMemoryBarrier(commandBuffer, queueFamilyIndices, image, barrierInfo, sourceStage, destinationStage);
    }

    VkCommandBuffer VulkanTransferCommandBuffer::getCommandBuffer() const {
        return commandBuffer;
    }
}
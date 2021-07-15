//There seems to be a bug with optional in msvc that stops it compiling. Having this header included here (above VulkanFactory) is a current work around.
#include "Clove/Graphics/Vulkan/VulkanRenderPass.hpp"
//
#include "Clove/Graphics/Vulkan/VulkanFactory.hpp"
//

#include "Clove/Graphics/ShaderCompiler.hpp"
#include "Clove/Graphics/Vulkan/MemoryAllocator.hpp"
#include "Clove/Graphics/Vulkan/VulkanBuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanComputePipelineObject.hpp"
#include "Clove/Graphics/Vulkan/VulkanComputeQueue.hpp"
#include "Clove/Graphics/Vulkan/VulkanDescriptor.hpp"
#include "Clove/Graphics/Vulkan/VulkanDescriptorPool.hpp"
#include "Clove/Graphics/Vulkan/VulkanDescriptorSetLayout.hpp"
#include "Clove/Graphics/Vulkan/VulkanFence.hpp"
#include "Clove/Graphics/Vulkan/VulkanFramebuffer.hpp"
#include "Clove/Graphics/Vulkan/VulkanGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Vulkan/VulkanGraphicsQueue.hpp"
#include "Clove/Graphics/Vulkan/VulkanImage.hpp"
#include "Clove/Graphics/Vulkan/VulkanImageView.hpp"
#include "Clove/Graphics/Vulkan/VulkanPipelineObject.hpp"
#include "Clove/Graphics/Vulkan/VulkanPresentQueue.hpp"
#include "Clove/Graphics/Vulkan/VulkanResource.hpp"
#include "Clove/Graphics/Vulkan/VulkanSampler.hpp"
#include "Clove/Graphics/Vulkan/VulkanSemaphore.hpp"
#include "Clove/Graphics/Vulkan/VulkanShader.hpp"
#include "Clove/Graphics/Vulkan/VulkanSwapchain.hpp"
#include "Clove/Graphics/Vulkan/VulkanTransferQueue.hpp"

#include <Clove/Cast.hpp>
#include <Clove/Log/Log.hpp>
#include <fstream>

namespace garlic::clove {
    namespace {
        VkCommandPoolCreateFlags convertCommandPoolCreateFlags(QueueFlags garlicFlags) {
            VkCommandPoolCreateFlags flags{ 0 };

            if((garlicFlags & QueueFlags::Transient) != 0) {
                flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            }
            if((garlicFlags & QueueFlags::ReuseBuffers) != 0) {
                flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            }

            return flags;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &availableFormats) {
            for(auto const &availableFormat : availableFormats) {
                if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }

            //Fall back to the first one if we can't find a surface format we want
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Warning, "GhaSwapchain could not find desired format. Using first available format from the surface");
            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const &availablePresentModes) {
            for(auto const &availablePresentMode : availablePresentModes) {
                if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const &capabilities, VkExtent2D const &windowExtent) {
            if(capabilities.currentExtent.width != UINT32_MAX) {//If width / height are max then the window manager is allowing us to differ in size
                return capabilities.currentExtent;
            } else {
                return VkExtent2D{
                    .width  = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, windowExtent.width)),
                    .height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, windowExtent.height)),
                };
            }
        }

        VkAttachmentLoadOp convertLoadOp(LoadOperation garlicOperation) {
            switch(garlicOperation) {
                case LoadOperation::Load:
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                case LoadOperation::Clear:
                    return VK_ATTACHMENT_LOAD_OP_CLEAR;
                case LoadOperation::DontCare:
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                default:
                    CLOVE_ASSERT(false, "{0}: Unkown operation", CLOVE_FUNCTION_NAME);
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
        }

        VkAttachmentStoreOp convertStoreOp(StoreOperation garlicOperation) {
            switch(garlicOperation) {
                case StoreOperation::Store:
                    return VK_ATTACHMENT_STORE_OP_STORE;
                case StoreOperation::DontCare:
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
                default:
                    CLOVE_ASSERT(false, "{0}: Unkown operation", CLOVE_FUNCTION_NAME);
                    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
        }

        VkFormat convertAttributeFormat(VertexAttributeFormat garlicFormat) {
            switch(garlicFormat) {
                case VertexAttributeFormat::R32_SFLOAT:
                    return VK_FORMAT_R32_SFLOAT;
                case VertexAttributeFormat::R32G32_SFLOAT:
                    return VK_FORMAT_R32G32_SFLOAT;
                case VertexAttributeFormat::R32G32B32_SFLOAT:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case VertexAttributeFormat::R32G32B32A32_SFLOAT:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                case VertexAttributeFormat::R32G32B32A32_SINT:
                    return VK_FORMAT_R32G32B32A32_SINT;
                default:
                    CLOVE_ASSERT(false, "{0}: Unkown format passed", CLOVE_FUNCTION_NAME_PRETTY);
                    return VK_FORMAT_UNDEFINED;
            }
        }

        VkDescriptorPoolCreateFlags getDescriptorPoolFlags(GhaDescriptorPool::Flag garlicFlag) {
            switch(garlicFlag) {
                case GhaDescriptorPool::Flag::None:
                    return 0;
                case GhaDescriptorPool::Flag::FreeDescriptorSet:
                    return VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                default:
                    CLOVE_ASSERT(false, "{0} Unknown flag", CLOVE_FUNCTION_NAME);
            }
        }

        VkBufferUsageFlags getUsageFlags(GhaBuffer::UsageMode garlicUsageFlags) {
            VkBufferUsageFlags flags = 0;

            if((garlicUsageFlags & GhaBuffer::UsageMode::TransferSource) != 0) {
                flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            if((garlicUsageFlags & GhaBuffer::UsageMode::TransferDestination) != 0) {
                flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            if((garlicUsageFlags & GhaBuffer::UsageMode::VertexBuffer) != 0) {
                flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
            if((garlicUsageFlags & GhaBuffer::UsageMode::IndexBuffer) != 0) {
                flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            if((garlicUsageFlags & GhaBuffer::UsageMode::UniformBuffer) != 0) {
                flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            }
            if((garlicUsageFlags & GhaBuffer::UsageMode::StorageBuffer) != 0) {
                flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            }

            return flags;
        }

        VkImageUsageFlags getUsageFlags(GhaImage::UsageMode garlicUsageFlags) {
            VkImageUsageFlags flags = 0;

            if((garlicUsageFlags & GhaImage::UsageMode::TransferSource) != 0) {
                flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            }
            if((garlicUsageFlags & GhaImage::UsageMode::TransferDestination) != 0) {
                flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            }
            if((garlicUsageFlags & GhaImage::UsageMode::Sampled) != 0) {
                flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
            }
            if((garlicUsageFlags & GhaImage::UsageMode::ColourAttachment) != 0) {
                flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            if((garlicUsageFlags & GhaImage::UsageMode::DepthStencilAttachment) != 0) {
                flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }

            return flags;
        }

        VkImageType getImageType(GhaImage::Type garlicImageType) {
            switch(garlicImageType) {
                case GhaImage::Type::_2D:
                case GhaImage::Type::Cube:
                    return VK_IMAGE_TYPE_2D;
                case GhaImage::Type::_3D:
                    return VK_IMAGE_TYPE_3D;
                default:
                    CLOVE_ASSERT(false, "{0}: Unhandled image type");
                    return VK_IMAGE_TYPE_2D;
            }
        }

        VkFilter getFilter(GhaSampler::Filter garlicfilter) {
            switch(garlicfilter) {
                case GhaSampler::Filter::Nearest:
                    return VK_FILTER_NEAREST;
                case GhaSampler::Filter::Linear:
                    return VK_FILTER_LINEAR;
                default:
                    CLOVE_ASSERT(false, "{0}: Unkown type", CLOVE_FUNCTION_NAME);
                    return VK_FILTER_NEAREST;
            }
        }

        VkSamplerAddressMode getAddressMode(GhaSampler::AddressMode garlicMode) {
            switch(garlicMode) {
                case GhaSampler::AddressMode::Repeat:
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case GhaSampler::AddressMode::MirroredRepeat:
                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case GhaSampler::AddressMode::ClampToEdge:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                case GhaSampler::AddressMode::ClampToBorder:
                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                default:
                    CLOVE_ASSERT(false, "{0}: Unkown type", CLOVE_FUNCTION_NAME);
                    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
        }

        Expected<VkImageView, std::runtime_error> createVkImageView(VkDevice device, VkImage image, GhaImageView::Descriptor const &viewdescriptor, GhaImage::Format const imageFormat) {
            VkImageAspectFlags const aspectFlags{ static_cast<VkImageAspectFlags>(imageFormat == GhaImage::Format::D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT) };

            VkImageViewCreateInfo const viewInfo{
                .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext      = nullptr,
                .flags      = 0,
                .image      = image,
                .viewType   = VulkanImageView::convertType(viewdescriptor.type),
                .format     = VulkanImage::convertFormat(imageFormat),
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
                    .baseArrayLayer = viewdescriptor.layer,
                    .layerCount     = viewdescriptor.layerCount,
                }
            };

            VkImageView imageView{ nullptr };
            if(VkResult const result{ vkCreateImageView(device, &viewInfo, nullptr, &imageView) }; result != VK_SUCCESS) {
                switch(result) {
                    case VK_ERROR_OUT_OF_HOST_MEMORY:
                        return Unexpected{ std::runtime_error{ "Failed to create GhaImageView. Out of host memory" } };
                    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                        return Unexpected{ std::runtime_error{ "Failed to create GhaImageView. Out of device memory" } };
                    default:
                        return Unexpected{ std::runtime_error{ "Failed to create GhaImageView. Reason unkown." } };
                }
            }

            return imageView;
        }
    }

    VulkanFactory::VulkanFactory(DevicePointer devicePtr, QueueFamilyIndices queueFamilyIndices)
        : devicePtr{ std::move(devicePtr) }
        , queueFamilyIndices{ queueFamilyIndices } {
        memoryAllocator = std::make_shared<MemoryAllocator>(this->devicePtr);
    }

    VulkanFactory::VulkanFactory(VulkanFactory &&other) noexcept = default;

    VulkanFactory &VulkanFactory::operator=(VulkanFactory &&other) noexcept = default;

    VulkanFactory::~VulkanFactory() = default;

    Expected<std::unique_ptr<GhaGraphicsQueue>, std::runtime_error> VulkanFactory::createGraphicsQueue(CommandQueueDescriptor descriptor) {
        uint32_t const familyIndex{ *queueFamilyIndices.graphicsFamily };

        VkCommandPoolCreateInfo poolInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = convertCommandPoolCreateFlags(descriptor.flags),
            .queueFamilyIndex = familyIndex,
        };

        VkCommandPool commandPool{ nullptr };
        if(VkResult const result{ vkCreateCommandPool(devicePtr.get(), &poolInfo, nullptr, &commandPool) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaGraphicsQueue. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaGraphicsQueue. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaGraphicsQueue. Reason unkown." } };
            }
        }

        VkQueue queue{ nullptr };
        vkGetDeviceQueue(devicePtr.get(), familyIndex, 0, &queue);

        return std::unique_ptr<GhaGraphicsQueue>{ std::make_unique<VulkanGraphicsQueue>(devicePtr, queue, commandPool, queueFamilyIndices) };
    }

    Expected<std::unique_ptr<GhaPresentQueue>, std::runtime_error> VulkanFactory::createPresentQueue() {
        if(!queueFamilyIndices.presentFamily.has_value()) {
            return Unexpected{ std::runtime_error{ "Presentation queue not available. GhaDevice is likely headless" } };
        }

        VkQueue queue{ nullptr };
        vkGetDeviceQueue(devicePtr.get(), *queueFamilyIndices.presentFamily, 0, &queue);

        return std::unique_ptr<GhaPresentQueue>{ std::make_unique<VulkanPresentQueue>(devicePtr, queue) };
    }

    Expected<std::unique_ptr<GhaTransferQueue>, std::runtime_error> VulkanFactory::createTransferQueue(CommandQueueDescriptor descriptor) {
        uint32_t const familyIndex{ *queueFamilyIndices.transferFamily };

        VkCommandPoolCreateInfo const poolInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = convertCommandPoolCreateFlags(descriptor.flags),
            .queueFamilyIndex = familyIndex,
        };

        VkCommandPool commandPool{ nullptr };
        if(VkResult const result{ vkCreateCommandPool(devicePtr.get(), &poolInfo, nullptr, &commandPool) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaTransferQueue. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaTransferQueue. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaTransferQueue. Reason unkown." } };
            }
        }

        VkQueue queue{ nullptr };
        vkGetDeviceQueue(devicePtr.get(), familyIndex, 0, &queue);

        return std::unique_ptr<GhaTransferQueue>{ std::make_unique<VulkanTransferQueue>(devicePtr, queue, commandPool, queueFamilyIndices) };
    }

    Expected<std::unique_ptr<GhaComputeQueue>, std::runtime_error> VulkanFactory::createComputeQueue(CommandQueueDescriptor descriptor) {
        uint32_t const familyIndex{ *queueFamilyIndices.computeFamily };

        VkCommandPoolCreateInfo const poolInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = convertCommandPoolCreateFlags(descriptor.flags),
            .queueFamilyIndex = familyIndex,
        };

        VkCommandPool commandPool{ nullptr };
        if(VkResult const result{ vkCreateCommandPool(devicePtr.get(), &poolInfo, nullptr, &commandPool) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaComputeQueue. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaComputeQueue. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaComputeQueue. Reason unkown." } };
            }
        }

        VkQueue queue{ nullptr };
        vkGetDeviceQueue(devicePtr.get(), familyIndex, 0, &queue);

        return std::unique_ptr<GhaComputeQueue>{ std::make_unique<VulkanComputeQueue>(devicePtr, queue, commandPool, queueFamilyIndices) };
    }

    Expected<std::unique_ptr<GhaSwapchain>, std::runtime_error> VulkanFactory::createSwapChain(GhaSwapchain::Descriptor descriptor) {
        if(devicePtr.getSurface() == VK_NULL_HANDLE) {
            return Unexpected{ std::runtime_error{ "GhaSwapchain is not available. GhaDevice is likely headless" } };
        }

        VkExtent2D const windowExtent{ descriptor.extent.x, descriptor.extent.y };

        SurfaceSupportDetails const surfaceSupport{ SurfaceSupportDetails::query(devicePtr.getPhysical(), devicePtr.getSurface()) };

        VkSurfaceFormatKHR const surfaceFormat{ chooseSwapSurfaceFormat(surfaceSupport.formats) };
        VkPresentModeKHR const presentMode{ chooseSwapPresentMode(surfaceSupport.presentModes) };
        VkExtent2D const swapchainExtent{ chooseSwapExtent(surfaceSupport.capabilities, windowExtent) };

        //Request one more than the minimum images the swap chain can support because sometimes we might need to wait for the driver
        uint32_t imageCount{ surfaceSupport.capabilities.minImageCount + 1 };
        if(surfaceSupport.capabilities.maxImageCount > 0 && imageCount > surfaceSupport.capabilities.maxImageCount) {
            imageCount = surfaceSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR const createInfo{
            .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface               = devicePtr.getSurface(),
            .minImageCount         = imageCount,
            .imageFormat           = surfaceFormat.format,
            .imageColorSpace       = surfaceFormat.colorSpace,
            .imageExtent           = swapchainExtent,
            .imageArrayLayers      = 1,
            .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices   = nullptr,
            .preTransform          = surfaceSupport.capabilities.currentTransform,
            .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode           = presentMode,
            .clipped               = VK_TRUE,
            .oldSwapchain          = VK_NULL_HANDLE,
        };

        VkSwapchainKHR swapchain{ nullptr };
        if(VkResult const result{ vkCreateSwapchainKHR(devicePtr.get(), &createInfo, nullptr, &swapchain) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Out of device memory" } };
                case VK_ERROR_DEVICE_LOST:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Device lost." } };
                case VK_ERROR_SURFACE_LOST_KHR:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Surface lost." } };
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Native window is in use." } };
                case VK_ERROR_INITIALIZATION_FAILED:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Initialisation has failed." } };
                case VK_ERROR_VALIDATION_FAILED_EXT:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Validation has failed." } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaSwapchain. Reason unkown." } };
            }
        }

        std::vector<VkImage> images{};
        std::vector<std::shared_ptr<VulkanImageView>> imageViews{};

        GhaImage::Format const imageFormat{ VulkanImage::convertFormat(surfaceFormat.format) };
        vec2ui const swapchainSize{ swapchainExtent.width, swapchainExtent.height };

        GhaImageView::Descriptor const viewDescriptor{
            .type = GhaImageView::Type::_2D,
        };

        uint32_t createdImageCount{ 0 };
        vkGetSwapchainImagesKHR(devicePtr.get(), swapchain, &createdImageCount, nullptr);
        images.resize(createdImageCount);
        vkGetSwapchainImagesKHR(devicePtr.get(), swapchain, &createdImageCount, images.data());

        imageViews.resize(images.size());
        for(size_t i{ 0 }; i < images.size(); ++i) {
            auto result{ createVkImageView(devicePtr.get(), images[i], viewDescriptor, imageFormat) };
            if(!result.hasValue()) {
                vkDestroySwapchainKHR(devicePtr.get(), swapchain, nullptr);
                return Unexpected{ result.getError() };
            }

            imageViews[i] = std::make_shared<VulkanImageView>(imageFormat, swapchainSize, devicePtr.get(), result.getValue());
        }

        return std::unique_ptr<GhaSwapchain>{ std::make_unique<VulkanSwapchain>(devicePtr, swapchain, surfaceFormat.format, swapchainExtent, std::move(imageViews)) };
    }

    Expected<std::unique_ptr<GhaShader>, std::runtime_error> VulkanFactory::createShaderFromFile(std::filesystem::path const &file, GhaShader::Stage shaderStage) {
        Expected<std::vector<uint32_t>, std::runtime_error> compilationResult{ ShaderCompiler::compileFromFile(file, shaderStage) };
        if(compilationResult.hasValue()) {
            std::vector<uint32_t> spirvSource{ std::move(compilationResult.getValue()) };
            return createShaderObject({ spirvSource.begin(), spirvSource.end() });
        } else {
            return Unexpected{ std::move(compilationResult.getError()) };
        }
    }

    Expected<std::unique_ptr<GhaShader>, std::runtime_error> VulkanFactory::createShaderFromSource(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage) {
        Expected<std::vector<uint32_t>, std::runtime_error> compilationResult{ ShaderCompiler::compileFromSource(source, std::move(includeSources), shaderName, shaderStage) };
        if(compilationResult.hasValue()) {
            std::vector<uint32_t> spirvSource{ std::move(compilationResult.getValue()) };
            return createShaderObject({ spirvSource.begin(), spirvSource.end() });
        } else {
            return Unexpected{ std::move(compilationResult.getError()) };
        }
    }

    Expected<std::unique_ptr<GhaRenderPass>, std::runtime_error> VulkanFactory::createRenderPass(GhaRenderPass::Descriptor descriptor) {
        //Attachments
        size_t const colourAttachmentSize{ descriptor.colourAttachments.size() };
        std::vector<VkAttachmentDescription> attachments(colourAttachmentSize);
        for(size_t i{ 0 }; i < colourAttachmentSize; ++i) {
            attachments[i] = VkAttachmentDescription{
                .format         = VulkanImage::convertFormat(descriptor.colourAttachments[i].format),
                .samples        = VK_SAMPLE_COUNT_1_BIT,
                .loadOp         = convertLoadOp(descriptor.colourAttachments[i].loadOperation),
                .storeOp        = convertStoreOp(descriptor.colourAttachments[i].storeOperation),
                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout  = VulkanImage::convertLayout(descriptor.colourAttachments[i].initialLayout),
                .finalLayout    = VulkanImage::convertLayout(descriptor.colourAttachments[i].finalLayout),
            };
        }
        //Push back the depth attachment so we can reference it in the subpass.
        attachments.emplace_back(VkAttachmentDescription{
            .format         = VulkanImage::convertFormat(descriptor.depthAttachment.format),
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = convertLoadOp(descriptor.depthAttachment.loadOperation),
            .storeOp        = convertStoreOp(descriptor.depthAttachment.storeOperation),
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VulkanImage::convertLayout(descriptor.depthAttachment.initialLayout),
            .finalLayout    = VulkanImage::convertLayout(descriptor.depthAttachment.finalLayout),
        });

        //Build attachment references
        std::vector<VkAttachmentReference> colourAttachmentReferences(colourAttachmentSize);
        for(size_t i{ 0 }; i < colourAttachmentSize; ++i) {
            colourAttachmentReferences[i] = VkAttachmentReference{
                .attachment = static_cast<uint32_t>(i),
                .layout     = VulkanImage::convertLayout(descriptor.colourAttachments[i].usedLayout),
            };
        }
        VkAttachmentReference const depthStencilAttachment{
            .attachment = static_cast<uint32_t>(attachments.size() - 1),
            .layout     = VulkanImage::convertLayout(descriptor.depthAttachment.usedLayout),
        };

        //Subpass
        VkSubpassDescription const subpass{
            .flags                   = 0,
            .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount    = 0,
            .pInputAttachments       = nullptr,
            .colorAttachmentCount    = static_cast<uint32_t>(colourAttachmentSize),
            .pColorAttachments       = colourAttachmentReferences.data(),
            .pResolveAttachments     = nullptr,
            .pDepthStencilAttachment = &depthStencilAttachment,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments    = nullptr,
        };

        //Dependency
        VkSubpassDependency constexpr dependecy{
            .srcSubpass    = VK_SUBPASS_EXTERNAL,
            .dstSubpass    = 0,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        //Renderpass
        VkRenderPassCreateInfo const renderPassInfo{
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments    = attachments.data(),
            .subpassCount    = 1,
            .pSubpasses      = &subpass,
            .dependencyCount = 1,
            .pDependencies   = &dependecy,
        };

        VkRenderPass renderPass{ nullptr };
        if(VkResult const result{ vkCreateRenderPass(devicePtr.get(), &renderPassInfo, nullptr, &renderPass) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create RenderPass. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create RenderPass. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create RenderPass. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaRenderPass>{ std::make_unique<VulkanRenderPass>(std::move(descriptor), devicePtr, renderPass) };
    }

    Expected<std::unique_ptr<GhaDescriptorSetLayout>, std::runtime_error> VulkanFactory::createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor descriptor) {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(std::size(descriptor.bindings));
        for(size_t i = 0; i < std::size(layoutBindings); ++i) {
            auto const &bindingDescriptor{ descriptor.bindings[i] };
            layoutBindings[i] = VkDescriptorSetLayoutBinding{
                .binding            = bindingDescriptor.binding,
                .descriptorType     = getDescriptorType(bindingDescriptor.type),
                .descriptorCount    = static_cast<uint32_t>(bindingDescriptor.arraySize),
                .stageFlags         = VulkanShader::convertStage(bindingDescriptor.stage),
                .pImmutableSamplers = nullptr,
            };
        }

        VkDescriptorSetLayoutCreateInfo const createInfo{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = nullptr,
            .flags        = 0,
            .bindingCount = static_cast<uint32_t>(std::size(layoutBindings)),
            .pBindings    = std::data(layoutBindings),
        };

        VkDescriptorSetLayout layout{ nullptr };
        if(VkResult const result{ vkCreateDescriptorSetLayout(devicePtr.get(), &createInfo, nullptr, &layout) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create DescriptorSetLayout. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create DescriptorSetLayout. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create DescriptorSetLayout. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaDescriptorSetLayout>{ std::make_unique<VulkanDescriptorSetLayout>(devicePtr, layout, std::move(descriptor)) };
    }

    Expected<std::unique_ptr<GhaGraphicsPipelineObject>, std::runtime_error> VulkanFactory::createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor descriptor) {
        //Descriptor set layouts
        size_t const descriptorLayoutCount{ std::size(descriptor.descriptorSetLayouts) };
        std::vector<VkDescriptorSetLayout> descriptorLayouts(descriptorLayoutCount);
        for(size_t i = 0; i < descriptorLayoutCount; ++i) {
            descriptorLayouts[i] = polyCast<VulkanDescriptorSetLayout>(descriptor.descriptorSetLayouts[i].get())->getLayout();
        }

        //Push constants
        std::vector<VkPushConstantRange> vkPushConstantRanges(std::size(descriptor.pushConstants));
        for(size_t i = 0; i < std::size(vkPushConstantRanges); ++i) {
            vkPushConstantRanges[i] = VkPushConstantRange{
                .stageFlags = VulkanShader::convertStage(descriptor.pushConstants[i].stage),
                .offset     = static_cast<uint32_t>(descriptor.pushConstants[i].offset),
                .size       = static_cast<uint32_t>(descriptor.pushConstants[i].size),
            };
        }

        //Pipeline Layout
        VkPipelineLayoutCreateInfo const pipelineLayoutInfo{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = static_cast<uint32_t>(descriptorLayouts.size()),
            .pSetLayouts            = descriptorLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size()),
            .pPushConstantRanges    = vkPushConstantRanges.data(),
        };

        VkPipelineLayout pipelineLayout{ nullptr };
        if(VkResult const result{ vkCreatePipelineLayout(devicePtr.get(), &pipelineLayoutInfo, nullptr, &pipelineLayout) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject's layout. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject's layout. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject's layout. Reason unkown." } };
            }
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

        //Vertex shader
        shaderStages[0] = VkPipelineShaderStageCreateInfo{
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = polyCast<VulkanShader>(descriptor.vertexShader.get())->getModule(),
            .pName  = "main",
        };

        //Fragment shader
        shaderStages[1] = VkPipelineShaderStageCreateInfo{
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = polyCast<VulkanShader>(descriptor.pixelShader.get())->getModule(),
            .pName  = "main",
        };

        //Vertex input
        VkVertexInputBindingDescription const inputBindingDescription{
            .binding   = 0,
            .stride    = descriptor.vertexInput.stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        size_t const vertexAttributeCount{ std::size(descriptor.vertexAttributes) };
        std::vector<VkVertexInputAttributeDescription> vertexAttributes(vertexAttributeCount);
        for(size_t i = 0; i < vertexAttributeCount; ++i) {
            auto const &attribute{ descriptor.vertexAttributes[i] };
            vertexAttributes[i] = VkVertexInputAttributeDescription{
                .location = static_cast<uint32_t>(i),
                .binding  = 0,
                .format   = convertAttributeFormat(attribute.format),
                .offset   = attribute.offset,
            };
        }

        VkPipelineVertexInputStateCreateInfo const vertexInputInfo{
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = 1,
            .pVertexBindingDescriptions      = &inputBindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(std::size(vertexAttributes)),
            .pVertexAttributeDescriptions    = std::data(vertexAttributes),
        };

        //Input assembly
        VkPipelineInputAssemblyStateCreateInfo const inputAssembly{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        //Viewport + Scissor
        std::vector<VkViewport> viewportStates{};
        std::vector<VkRect2D> scissorStates{};
        std::vector<VkDynamicState> dynamicStates{};

        if(descriptor.viewportDescriptor.state == ElementState::Static) {
            viewportStates.emplace_back(VkViewport{
                .x        = static_cast<float>(descriptor.viewportDescriptor.position.x),
                .y        = static_cast<float>(descriptor.viewportDescriptor.position.y),
                .width    = static_cast<float>(descriptor.viewportDescriptor.size.x),
                .height   = static_cast<float>(descriptor.viewportDescriptor.size.y),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            });
        } else {
            dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        }

        if(descriptor.scissorDescriptor.state == ElementState::Static) {
            scissorStates.emplace_back(VkRect2D{
                .offset = { descriptor.scissorDescriptor.position.x, descriptor.scissorDescriptor.position.y },
                .extent = { descriptor.scissorDescriptor.size.x, descriptor.scissorDescriptor.size.y },
            });
        } else {
            dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        }

        VkPipelineViewportStateCreateInfo const viewportState{
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,//Dynamic or not vulkan expects 1 viewport to be bound
            .pViewports    = viewportStates.data(),
            .scissorCount  = 1,//Dynamic or not vulkan expects 1 scissor rect to be bound
            .pScissors     = scissorStates.data(),
        };

        VkPipelineDynamicStateCreateInfo const dynamicViewportState{
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates    = dynamicStates.data(),
        };

        //Rasteriser
        VkPipelineRasterizationStateCreateInfo const rasterizer{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = VK_POLYGON_MODE_FILL,
            .cullMode                = VK_CULL_MODE_BACK_BIT,
            .frontFace               = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = 1.0f,//Any thicker requires the 'wideLines' feature
        };

        //Multisampling
        VkPipelineMultisampleStateCreateInfo const multisampling{
            .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable  = VK_FALSE,
        };

        //Depth / Stencil
        VkPipelineDepthStencilStateCreateInfo const depthStencil{
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .depthTestEnable       = static_cast<VkBool32>(descriptor.depthState.depthTest ? VK_TRUE : VK_FALSE),
            .depthWriteEnable      = static_cast<VkBool32>(descriptor.depthState.depthWrite ? VK_TRUE : VK_FALSE),
            .depthCompareOp        = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = VK_FALSE,
            .front                 = {},
            .back                  = {},
            .minDepthBounds        = 0.0f,
            .maxDepthBounds        = 1.0f,
        };

        //Colour blending
        VkPipelineColorBlendAttachmentState const colourBlendAttachment{
            .blendEnable         = static_cast<VkBool32>(descriptor.enableBlending ? VK_TRUE : VK_FALSE),
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo const colorBlending{
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable   = VK_FALSE,
            .logicOp         = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments    = &colourBlendAttachment,
            .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
        };

        //Pipeline
        VkGraphicsPipelineCreateInfo const pipelineInfo{
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = nullptr,
            .flags               = 0,
            .stageCount          = shaderStages.size(),
            .pStages             = shaderStages.data(),
            .pVertexInputState   = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pTessellationState  = nullptr,
            .pViewportState      = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState   = &multisampling,
            .pDepthStencilState  = &depthStencil,
            .pColorBlendState    = &colorBlending,
            .pDynamicState       = &dynamicViewportState,
            .layout              = pipelineLayout,
            .renderPass          = polyCast<VulkanRenderPass>(descriptor.renderPass.get())->getRenderPass(),
            .subpass             = 0,//The subpass of the renderpass that'll use this pipeline
            .basePipelineHandle  = VK_NULL_HANDLE,
            .basePipelineIndex   = -1,
        };

        VkPipeline pipeline{ nullptr };
        if(VkResult const result{ vkCreateGraphicsPipelines(devicePtr.get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaGraphicsPipelineObject>{ std::make_unique<VulkanGraphicsPipelineObject>(std::move(descriptor), devicePtr, pipeline, pipelineLayout) };
    }

    Expected<std::unique_ptr<GhaComputePipelineObject>, std::runtime_error> VulkanFactory::createComputePipelineObject(GhaComputePipelineObject::Descriptor descriptor) {
        //Descriptor set layouts
        size_t const descriptorLayoutCount{ std::size(descriptor.descriptorSetLayouts) };
        std::vector<VkDescriptorSetLayout> descriptorLayouts(descriptorLayoutCount);
        for(size_t i = 0; i < descriptorLayoutCount; ++i) {
            descriptorLayouts[i] = polyCast<VulkanDescriptorSetLayout>(descriptor.descriptorSetLayouts[i].get())->getLayout();
        }

        //Push constants
        std::vector<VkPushConstantRange> vkPushConstantRanges(std::size(descriptor.pushConstants));
        for(size_t i = 0; i < std::size(vkPushConstantRanges); ++i) {
            vkPushConstantRanges[i] = VkPushConstantRange{
                .stageFlags = VulkanShader::convertStage(descriptor.pushConstants[i].stage),
                .offset     = static_cast<uint32_t>(descriptor.pushConstants[i].offset),
                .size       = static_cast<uint32_t>(descriptor.pushConstants[i].size),
            };
        }

        //Pipeline Layout
        VkPipelineLayoutCreateInfo const pipelineLayoutInfo{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = static_cast<uint32_t>(descriptorLayouts.size()),
            .pSetLayouts            = descriptorLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(vkPushConstantRanges.size()),
            .pPushConstantRanges    = vkPushConstantRanges.data(),
        };

        VkPipelineLayout pipelineLayout{ nullptr };
        if(VkResult const result{ vkCreatePipelineLayout(devicePtr.get(), &pipelineLayoutInfo, nullptr, &pipelineLayout) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create ComputePipelineObject's layout. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create ComputePipelineObject's layout. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create ComputePipelineObject's layout. Reason unkown." } };
            }
        }

        //Shader
        VkPipelineShaderStageCreateInfo const shaderStage{
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = polyCast<VulkanShader>(descriptor.shader.get())->getModule(),
            .pName  = "main",
        };

        //Pipeline
        VkComputePipelineCreateInfo const pipelineInfo{
            .sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext  = nullptr,
            .flags  = 0,
            .stage  = shaderStage,
            .layout = pipelineLayout,
        };

        VkPipeline pipeline{ nullptr };
        if(VkResult const result{ vkCreateComputePipelines(devicePtr.get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create ComputePipelineObject. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create ComputePipelineObject. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create ComputePipelineObject. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaComputePipelineObject>{ std::make_unique<VulkanComputePipelineObject>(devicePtr, pipeline, pipelineLayout) };
    }

    Expected<std::unique_ptr<GhaFramebuffer>, std::runtime_error> VulkanFactory::createFramebuffer(GhaFramebuffer::Descriptor descriptor) {
        std::vector<VkImageView> attachments;
        attachments.reserve(std::size(descriptor.attachments));
        for(auto &attachment : descriptor.attachments) {
            attachments.push_back(polyCast<VulkanImageView>(attachment.get())->getImageView());
        }

        VkFramebufferCreateInfo const framebufferInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .renderPass      = polyCast<VulkanRenderPass>(descriptor.renderPass.get())->getRenderPass(),
            .attachmentCount = static_cast<uint32_t>(std::size(attachments)),
            .pAttachments    = std::data(attachments),
            .width           = descriptor.width,
            .height          = descriptor.height,
            .layers          = 1,
        };

        VkFramebuffer framebuffer{ nullptr };
        if(VkResult const result{ vkCreateFramebuffer(devicePtr.get(), &framebufferInfo, nullptr, &framebuffer) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaFramebuffer. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaFramebuffer. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaFramebuffer. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaFramebuffer>{ std::make_unique<VulkanFramebuffer>(devicePtr, framebuffer) };
    }

    Expected<std::unique_ptr<GhaDescriptorPool>, std::runtime_error> VulkanFactory::createDescriptorPool(GhaDescriptorPool::Descriptor descriptor) {
        size_t const numDescriptorTypes = std::size(descriptor.poolTypes);
        std::vector<VkDescriptorPoolSize> poolSizes(numDescriptorTypes);
        for(size_t i = 0; i < numDescriptorTypes; ++i) {
            poolSizes[i] = VkDescriptorPoolSize{
                .type            = getDescriptorType(descriptor.poolTypes[i].type),
                .descriptorCount = descriptor.poolTypes[i].count,
            };
        }

        VkDescriptorPoolCreateInfo const createInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = getDescriptorPoolFlags(descriptor.flag),
            .maxSets       = descriptor.maxSets,
            .poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
            .pPoolSizes    = std::data(poolSizes),
        };

        VkDescriptorPool pool{ nullptr };
        if(VkResult const result{ vkCreateDescriptorPool(devicePtr.get(), &createInfo, nullptr, &pool) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaDescriptorPool. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaDescriptorPool. Out of device memory" } };
                case VK_ERROR_FRAGMENTATION_EXT:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaDescriptorPool. Memory is too fragmented" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaDescriptorPool. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaDescriptorPool>{ std::make_unique<VulkanDescriptorPool>(devicePtr, pool, std::move(descriptor)) };
    }

    Expected<std::unique_ptr<GhaSemaphore>, std::runtime_error> VulkanFactory::createSemaphore() {
        VkSemaphoreCreateInfo const createInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkSemaphore semaphore{ nullptr };
        if(VkResult const result{ vkCreateSemaphore(devicePtr.get(), &createInfo, nullptr, &semaphore) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create Semaphore. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create Semaphore. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create Semaphore. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaSemaphore>{ std::make_unique<VulkanSemaphore>(devicePtr, semaphore) };
    }

    Expected<std::unique_ptr<GhaFence>, std::runtime_error> VulkanFactory::createFence(GhaFence::Descriptor descriptor) {
        VkFenceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = descriptor.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFenceCreateFlags>(0),
        };

        VkFence fence{ nullptr };
        if(VkResult const result{ vkCreateFence(devicePtr.get(), &createInfo, nullptr, &fence) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaFence. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaFence. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaFence. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaFence>{ std::make_unique<VulkanFence>(devicePtr, fence) };
    }

    Expected<std::unique_ptr<GhaBuffer>, std::runtime_error> VulkanFactory::createBuffer(GhaBuffer::Descriptor descriptor) {
        std::array const sharedQueueIndices{ *queueFamilyIndices.graphicsFamily, *queueFamilyIndices.transferFamily, *queueFamilyIndices.computeFamily };
        bool const isExclusive{ descriptor.sharingMode == SharingMode::Exclusive };

        VkBufferCreateInfo const createInfo{
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = 0,
            .size                  = descriptor.size,
            .usage                 = getUsageFlags(descriptor.usageFlags),
            .sharingMode           = isExclusive ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = isExclusive ? 0 : static_cast<uint32_t>(std::size(sharedQueueIndices)),
            .pQueueFamilyIndices   = isExclusive ? nullptr : std::data(sharedQueueIndices),
        };

        VkBuffer buffer{ nullptr };
        if(VkResult const result{ vkCreateBuffer(devicePtr.get(), &createInfo, nullptr, &buffer) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaBuffer. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaBuffer. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaBuffer. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaBuffer>{ std::make_unique<VulkanBuffer>(devicePtr, buffer, descriptor, memoryAllocator) };
    }

    Expected<std::unique_ptr<GhaImage>, std::runtime_error> VulkanFactory::createImage(GhaImage::Descriptor descriptor) {
        std::array const sharedQueueIndices{ *queueFamilyIndices.graphicsFamily, *queueFamilyIndices.transferFamily, *queueFamilyIndices.computeFamily };
        bool const isExclusive{ descriptor.sharingMode == SharingMode::Exclusive };
        bool const isCube{ descriptor.type == GhaImage::Type::Cube };

        VkImageCreateInfo const createInfo{
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = nullptr,
            .flags                 = isCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
            .imageType             = getImageType(descriptor.type),
            .format                = VulkanImage::convertFormat(descriptor.format),
            .extent                = { descriptor.dimensions.x, descriptor.dimensions.y, 1 },
            .mipLevels             = 1,
            .arrayLayers           = isCube ? 6u : 1u,
            .samples               = VK_SAMPLE_COUNT_1_BIT,
            .tiling                = VK_IMAGE_TILING_OPTIMAL,
            .usage                 = getUsageFlags(descriptor.usageFlags),
            .sharingMode           = isExclusive ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = isExclusive ? 0 : static_cast<uint32_t>(std::size(sharedQueueIndices)),
            .pQueueFamilyIndices   = isExclusive ? nullptr : std::data(sharedQueueIndices),
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VkImage image{ nullptr };
        if(VkResult const result{ vkCreateImage(devicePtr.get(), &createInfo, nullptr, &image) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaImage. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaImage. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create GhaImage. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaImage>{ std::make_unique<VulkanImage>(devicePtr, image, descriptor, memoryAllocator) };
    }

    Expected<std::unique_ptr<GhaImageView>, std::runtime_error> VulkanFactory::createImageView(GhaImage const &image, GhaImageView::Descriptor descriptor) {
        GhaImage::Descriptor const imageDescriptor{ image.getDescriptor() };

        auto result{ createVkImageView(devicePtr.get(), polyCast<VulkanImage const>(&image)->getImage(), descriptor, imageDescriptor.format) };
        if(!result.hasValue()) {
            return Unexpected{ result.getError() };
        }

        return std::unique_ptr<GhaImageView>{ std::make_unique<VulkanImageView>(imageDescriptor.format, imageDescriptor.dimensions, devicePtr.get(), result.getValue()) };
    }

    Expected<std::unique_ptr<GhaSampler>, std::runtime_error> VulkanFactory::createSampler(GhaSampler::Descriptor descriptor) {
        VkSamplerCreateInfo const createInfo{
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext                   = nullptr,
            .flags                   = 0,
            .magFilter               = getFilter(descriptor.magFilter),
            .minFilter               = getFilter(descriptor.minFilter),
            .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU            = getAddressMode(descriptor.addressModeU),
            .addressModeV            = getAddressMode(descriptor.addressModeV),
            .addressModeW            = getAddressMode(descriptor.addressModeW),
            .mipLodBias              = 0.0f,
            .anisotropyEnable        = static_cast<VkBool32>(descriptor.enableAnisotropy ? VK_TRUE : VK_FALSE),
            .maxAnisotropy           = descriptor.maxAnisotropy,
            .compareEnable           = VK_FALSE,
            .compareOp               = VK_COMPARE_OP_ALWAYS,
            .minLod                  = 0.0f,
            .maxLod                  = 0.0f,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkSampler sampler{ nullptr };
        if(VkResult const result{ vkCreateSampler(devicePtr.get(), &createInfo, nullptr, &sampler) }; result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create Sampler. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create Sampler. Out of device memory" } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create Sampler. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaSampler>{ std::make_unique<VulkanSampler>(devicePtr, sampler) };
    }

    Expected<std::unique_ptr<GhaShader>, std::runtime_error> VulkanFactory::createShaderObject(std::span<uint32_t> spirvSource) {
        VkShaderModuleCreateInfo const createInfo{
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext    = nullptr,
            .flags    = 0,
            .codeSize = spirvSource.size_bytes(),
            .pCode    = std::data(spirvSource),
        };

        VkShaderModule module{ nullptr };
        if(VkResult const result = vkCreateShaderModule(devicePtr.get(), &createInfo, nullptr, &module); result != VK_SUCCESS) {
            switch(result) {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create Shader. Out of host memory" } };
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    return Unexpected{ std::runtime_error{ "Failed to create Shader. Out of device memory" } };
                case VK_ERROR_INVALID_SHADER_NV:
                    return Unexpected{ std::runtime_error{ "Failed to create Shader. Shader failed to compile." } };
                default:
                    return Unexpected{ std::runtime_error{ "Failed to create Shader. Reason unkown." } };
            }
        }

        return std::unique_ptr<GhaShader>{ std::make_unique<VulkanShader>(devicePtr, module) };
    }
}
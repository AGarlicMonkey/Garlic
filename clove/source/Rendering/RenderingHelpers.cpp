#include "Clove/Rendering/RenderingHelpers.hpp"

#include "Clove/Rendering/RenderingConstants.hpp"

#include <Clove/Graphics/GhaDescriptorSetLayout.hpp>
#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaGraphicsQueue.hpp>
#include <Clove/Graphics/GhaShader.hpp>
#include <Clove/Graphics/GhaTransferQueue.hpp>
#include <Clove/TextureLoader.hpp>

using namespace garlic::clove;

namespace garlic::clove {
    std::unique_ptr<GhaDescriptorSetLayout> createMeshDescriptorSetLayout(GhaFactory &factory) {
        DescriptorSetBindingInfo const diffuseTextureBinding{
            .binding   = 0,
            .type      = DescriptorType::CombinedImageSampler,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Pixel,
        };

        DescriptorSetBindingInfo const specularTextureBinding{
            .binding   = 1,
            .type      = DescriptorType::CombinedImageSampler,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Pixel,
        };

        DescriptorSetBindingInfo const modelBinding{
            .binding   = 2,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Vertex,
        };

        DescriptorSetBindingInfo const skeletonBinding{
            .binding   = 3,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Vertex,
        };

        DescriptorSetBindingInfo const colourBinding{
            .binding   = 4,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Pixel,
        };

        return *factory.createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor{
            .bindings = {
                std::move(diffuseTextureBinding),
                std::move(specularTextureBinding),
                std::move(modelBinding),
                std::move(skeletonBinding),
                std::move(colourBinding),
            },
        });
    }

    std::unique_ptr<GhaDescriptorSetLayout> createViewDescriptorSetLayout(GhaFactory &factory) {
        DescriptorSetBindingInfo const viewDataBinding{
            .binding   = 0,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Vertex,
        };

        DescriptorSetBindingInfo const viewPosBinding{
            .binding   = 1,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Pixel,
        };

        return *factory.createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor{
            .bindings = {
                std::move(viewDataBinding),
                std::move(viewPosBinding),
            },
        });
    }

    std::unique_ptr<GhaDescriptorSetLayout> createLightingDescriptorSetLayout(GhaFactory &factory) {
        DescriptorSetBindingInfo const lightDataBinding{
            .binding   = 0,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Pixel,
        };

        DescriptorSetBindingInfo const numLightBinding{
            .binding   = 1,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Vertex | GhaShader::Stage::Pixel,
        };

        DescriptorSetBindingInfo const directionalShadowTransformBinding{
            .binding   = 2,
            .type      = DescriptorType::UniformBuffer,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Vertex,
        };

        DescriptorSetBindingInfo const directionalShadowMapBinding{
            .binding   = 3,
            .type      = DescriptorType::CombinedImageSampler,
            .arraySize = MAX_LIGHTS,
            .stage     = GhaShader::Stage::Pixel,
        };

        DescriptorSetBindingInfo const pointShadowMapBinding{
            .binding   = 4,
            .type      = DescriptorType::CombinedImageSampler,
            .arraySize = MAX_LIGHTS,
            .stage     = GhaShader::Stage::Pixel,
        };

        return *factory.createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor{
            .bindings = {
                std::move(lightDataBinding),
                std::move(numLightBinding),
                std::move(directionalShadowTransformBinding),
                std::move(directionalShadowMapBinding),
                std::move(pointShadowMapBinding),
            },
        });
    }

    std::unique_ptr<GhaDescriptorSetLayout> createUiDescriptorSetLayout(GhaFactory &factory) {
        DescriptorSetBindingInfo const textureBinding{
            .binding   = 0,
            .type      = DescriptorType::CombinedImageSampler,
            .arraySize = 1,
            .stage     = GhaShader::Stage::Pixel,
        };

        return *factory.createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor{
            .bindings = {
                std::move(textureBinding),
            },
        });
    }

    std::unordered_map<DescriptorType, uint32_t> countDescriptorBindingTypes(GhaDescriptorSetLayout const &descriptorSetLayout) {
        std::unordered_map<DescriptorType, uint32_t> counts;
        auto const &descriptor{ descriptorSetLayout.getDescriptor() };
        for(auto &binding : descriptor.bindings) {
            counts[binding.type] += binding.arraySize;
        }

        return counts;
    }

    std::unique_ptr<GhaImage> createImageWithData(GhaFactory &factory, GhaImage::Descriptor imageDescriptor, void const *data, size_t const dataSize) {
        //Make sure the image is set up to be transfered to.
        imageDescriptor.usageFlags |= GhaImage::UsageMode::TransferDestination;

        ImageMemoryBarrierInfo constexpr layoutTransferInfo{
            .currentAccess      = AccessFlags::None,
            .newAccess          = AccessFlags::TransferWrite,
            .currentImageLayout = GhaImage::Layout::Undefined,
            .newImageLayout     = GhaImage::Layout::TransferDestinationOptimal,
            .sourceQueue        = QueueType::None,
            .destinationQueue   = QueueType::None,
        };

        ImageMemoryBarrierInfo constexpr transferQueueReleaseInfo{
            .currentAccess      = AccessFlags::TransferWrite,
            .newAccess          = AccessFlags::None,
            .currentImageLayout = GhaImage::Layout::TransferDestinationOptimal,
            .newImageLayout     = GhaImage::Layout::TransferDestinationOptimal,
            .sourceQueue        = QueueType::Transfer,
            .destinationQueue   = QueueType::Graphics,
        };

        ImageMemoryBarrierInfo constexpr graphicsQueueAcquireInfo{
            .currentAccess      = AccessFlags::TransferWrite,
            .newAccess          = AccessFlags::ShaderRead,
            .currentImageLayout = GhaImage::Layout::TransferDestinationOptimal,
            .newImageLayout     = GhaImage::Layout::ShaderReadOnlyOptimal,
            .sourceQueue        = QueueType::Transfer,
            .destinationQueue   = QueueType::Graphics,
        };

        size_t constexpr bufferOffset{ 0 };
        vec3i constexpr imageOffset{ 0, 0, 0 };
        vec3ui const imageExtent{ imageDescriptor.dimensions.x, imageDescriptor.dimensions.y, 1 };

        auto transferQueue = *factory.createTransferQueue({ QueueFlags::Transient });
        auto graphicsQueue = *factory.createGraphicsQueue({ QueueFlags::Transient });

        std::shared_ptr<GhaTransferCommandBuffer> transferCommandBuffer{ transferQueue->allocateCommandBuffer() };
        std::shared_ptr<GhaGraphicsCommandBuffer> graphicsCommandBuffer{ graphicsQueue->allocateCommandBuffer() };

        auto image{ *factory.createImage(std::move(imageDescriptor)) };

        auto transferBuffer = *factory.createBuffer(GhaBuffer::Descriptor{
            .size        = dataSize,
            .usageFlags  = GhaBuffer::UsageMode::TransferSource,
            .sharingMode = SharingMode::Exclusive,
            .memoryType  = MemoryType::SystemMemory,
        });
        transferBuffer->write(data, bufferOffset, dataSize);

        //Change the layout of the image, write the buffer into it and then release the queue ownership
        transferCommandBuffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
        transferCommandBuffer->imageMemoryBarrier(*image, std::move(layoutTransferInfo), GhaPipelineObject::Stage::Top, GhaPipelineObject::Stage::Transfer);
        transferCommandBuffer->copyBufferToImage(*transferBuffer, bufferOffset, *image, imageOffset, imageExtent);
        transferCommandBuffer->imageMemoryBarrier(*image, std::move(transferQueueReleaseInfo), GhaPipelineObject::Stage::Transfer, GhaPipelineObject::Stage::Transfer);
        transferCommandBuffer->endRecording();

        //Acquire ownership of the image to a graphics queue
        graphicsCommandBuffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
        graphicsCommandBuffer->imageMemoryBarrier(*image, std::move(graphicsQueueAcquireInfo), GhaPipelineObject::Stage::Transfer, GhaPipelineObject::Stage::PixelShader);
        graphicsCommandBuffer->endRecording();

        auto transferQueueFinishedFence{ *factory.createFence({ false }) };
        auto graphicsQueueFinishedFence{ *factory.createFence({ false }) };

        transferQueue->submit({ TransferSubmitInfo{ .commandBuffers = { transferCommandBuffer } } }, transferQueueFinishedFence.get());
        graphicsQueue->submit({ GraphicsSubmitInfo{ .commandBuffers = { graphicsCommandBuffer } } }, graphicsQueueFinishedFence.get());

        transferQueueFinishedFence->wait();
        transferQueue->freeCommandBuffer(*transferCommandBuffer);

        graphicsQueueFinishedFence->wait();
        graphicsQueue->freeCommandBuffer(*graphicsCommandBuffer);

        return image;
    }

    std::unique_ptr<GhaImage> createImageFromPath(GhaFactory &factory, std::filesystem::path const &path) {
        TextureLoader::LoadedTextureData const textureData{ TextureLoader::loadTexture(path).getValue() };

        GhaImage::Descriptor const textureDesc{
            .type        = GhaImage::Type::_2D,
            .usageFlags  = GhaImage::UsageMode::Sampled,
            .dimensions  = textureData.dimensions,
            .format      = GhaImage::Format::R8G8B8A8_SRGB,
            .sharingMode = SharingMode::Exclusive,
        };

        size_t const size{ textureData.dimensions.x * textureData.dimensions.y * textureData.channels };

        return createImageWithData(factory, std::move(textureDesc), textureData.buffer.get(), size);
    }
}
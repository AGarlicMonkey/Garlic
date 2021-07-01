#include "Clove/Rendering/RenderGraph/RenderGraph.hpp"

#include "Clove/Rendering/RenderGraph/RgFrameCache.hpp"
#include "Clove/Rendering/RenderGraph/RgGlobalCache.hpp"
#include "Clove/Rendering/Vertex.hpp"

#include <Clove/Graphics/GhaDescriptorSet.hpp>
#include <Clove/Graphics/GhaDescriptorSetLayout.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    RenderGraph::RenderGraph(RgFrameCache &frameCache, RgGlobalCache &globalCache)
        : frameCache{ frameCache }
        , globalCache{ globalCache } {
        frameCache.reset();
    }

    RenderGraph::~RenderGraph() = default;

    ResourceIdType RenderGraph::createBuffer(size_t const bufferSize) {
        ResourceIdType const bufferId{ nextResourceId++ };
        buffers[bufferId] = std::make_unique<RgBuffer>(bufferId, bufferSize);

        return bufferId;
    }

    ResourceIdType RenderGraph::createBuffer(std::shared_ptr<GhaBuffer> buffer, size_t const offset, size_t const size) {
        ResourceIdType const bufferId{ nextResourceId++ };
        buffers[bufferId] = std::make_unique<RgBuffer>(bufferId, std::move(buffer), size, offset);

        return bufferId;
    }

    void RenderGraph::writeToBuffer(ResourceIdType buffer, void const *data, size_t const offset, size_t const size) {
        //TODO
    }

    ResourceIdType RenderGraph::createImage(GhaImage::Type imageType, GhaImage::Format format, vec2ui dimensions) {
        ResourceIdType const imageId{ nextResourceId++ };
        images[imageId] = std::make_unique<RgImage>(imageId, imageType, format, dimensions);

        return imageId;
    }

    ResourceIdType RenderGraph::createImage(std::shared_ptr<GhaImageView> ghaImageView) {
        ResourceIdType const imageId{ nextResourceId++ };
        images[imageId] = std::make_unique<RgImage>(imageId, std::move(ghaImageView));

        return imageId;
    }

    void RenderGraph::registerGraphOutput(ResourceIdType resource) {
        outputResource = resource;
    }

    RgShader RenderGraph::createShader(std::filesystem::path const &file, GhaShader::Stage shaderStage) {
        RgShader const shader{ nextResourceId++ };
        allocatedShaders[shader] = globalCache.createShader(file, shaderStage);//Allocate straight away as it's usage does not define it's properties

        return shader;
    }

    RgShader RenderGraph::createShader(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage) {
        RgShader const shader{ nextResourceId++ };
        allocatedShaders[shader] = globalCache.createShader(source, std::move(includeSources), shaderName, shaderStage);//Allocate straight away as it's usage does not define it's properties

        return shader;
    }

    PassIdType RenderGraph::addRenderPass(RgRenderPass::Descriptor passDescriptor) {
        PassIdType const renderPassId{ nextPassId };
        auto pass{ std::make_unique<RgRenderPass>(renderPassId) };

        for(auto &renderTarget : passDescriptor.renderTargets) {
            ResourceIdType const imageId{ renderTarget.target };

            auto &image{ images.at(imageId) };
            image->addImageUsage(GhaImage::UsageMode::ColourAttachment);
            image->addWritePass(renderPassId);

            pass->addOutputResource(imageId);
        }

        {
            ResourceIdType const imageId{ passDescriptor.depthStencil.target };

            auto &image{ images.at(imageId) };
            image->addImageUsage(GhaImage::UsageMode::DepthStencilAttachment);
            image->addWritePass(renderPassId);

            pass->addOutputResource(imageId);
        }

        renderPasses[renderPassId] = std::move(pass);
    }

    void RenderGraph::addRenderSubmission(PassIdType const renderPass, RgRenderPass::Submission submission) {
        auto &pass{ renderPasses.at(renderPass) };

        {
            ResourceIdType const bufferId{ submission.vertexBuffer };

            auto &buffer{ buffers.at(bufferId) };
            buffer->addBufferUsage(GhaBuffer::UsageMode::VertexBuffer);
            buffer->addReadPass(renderPass);

            pass->addInputResource(bufferId);
        }

        {
            ResourceIdType const bufferId{ submission.indexBuffer };

            auto &buffer{ buffers.at(bufferId) };
            buffer->addBufferUsage(GhaBuffer::UsageMode::IndexBuffer);
            buffer->addReadPass(renderPass);

            pass->addInputResource(bufferId);
        }

        for(auto const &ubo : submission.shaderUbos) {
            ResourceIdType const bufferId{ ubo.buffer };

            auto &buffer{ buffers.at(bufferId) };
            buffer->addBufferUsage(GhaBuffer::UsageMode::UniformBuffer);
            buffer->addReadPass(renderPass);

            pass->addInputResource(bufferId);
        }

        for(auto const &imageSampler : submission.shaderCombinedImageSamplers) {
            ResourceIdType const imageId{ imageSampler.image };

            auto &image{ images.at(imageId) };
            image->addImageUsage(GhaImage::UsageMode::Sampled);
            image->addReadPass(renderPass);

            pass->addInputResource(imageId);
        }
    }

    GraphicsSubmitInfo RenderGraph::execute() {
        //TODO
        return {};
    }
}
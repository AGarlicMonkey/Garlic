#include "Clove/Rendering/RenderGraph/RenderGraph.hpp"

#include "Clove/Rendering/RenderGraph/RgFrameCache.hpp"
#include "Clove/Rendering/RenderGraph/RgGlobalCache.hpp"
#include "Clove/Rendering/Vertex.hpp"

#include <Clove/Graphics/GhaDescriptorSet.hpp>
#include <Clove/Graphics/GhaDescriptorSetLayout.hpp>
#include <Clove/Graphics/GhaGraphicsCommandBuffer.hpp>
#include <Clove/Graphics/GhaGraphicsPipelineObject.hpp>
#include <Clove/Graphics/GhaRenderPass.hpp>
#include <Clove/Graphics/GhaSampler.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    RenderGraph::RenderGraph(RgFrameCache &frameCache, RgGlobalCache &globalCache)
        : frameCache{ frameCache }
        , globalCache{ globalCache } {
        frameCache.reset();
    }

    RenderGraph::~RenderGraph() = default;

    RgResourceIdType RenderGraph::createBuffer(size_t const bufferSize) {
        RgResourceIdType const bufferId{ nextResourceId++ };
        buffers[bufferId] = std::make_unique<RgBuffer>(bufferId, bufferSize);

        return bufferId;
    }

    RgResourceIdType RenderGraph::createBuffer(std::shared_ptr<GhaBuffer> buffer, size_t const offset, size_t const size) {
        RgResourceIdType const bufferId{ nextResourceId++ };
        buffers[bufferId] = std::make_unique<RgBuffer>(bufferId, std::move(buffer), size, offset);

        return bufferId;
    }

    void RenderGraph::writeToBuffer(RgResourceIdType buffer, void const *data, size_t const offset, size_t const size) {
        RgPassIdType const transferPassId{ nextPassId++ };

        auto &rgBuffer{ buffers.at(buffer) };
        rgBuffer->makeCpuAccessable();
        rgBuffer->addWritePass(transferPassId);

        RgTransferPass::BufferWrite write{
            .bufferId = buffer,
            .data     = std::vector<std::byte>(size),
            .offset   = offset,
            .size     = size,
        };
        memcpy(write.data.data(), data, size);

        transferPasses[transferPassId] = std::make_unique<RgTransferPass>(transferPassId, std::move(write));
    }

    RgResourceIdType RenderGraph::createImage(GhaImage::Type imageType, GhaImage::Format format, vec2ui dimensions) {
        RgResourceIdType const imageId{ nextResourceId++ };
        images[imageId] = std::make_unique<RgImage>(imageId, imageType, format, dimensions);

        return imageId;
    }

    RgResourceIdType RenderGraph::createImage(std::shared_ptr<GhaImageView> ghaImageView) {
        RgResourceIdType const imageId{ nextResourceId++ };
        images[imageId] = std::make_unique<RgImage>(imageId, std::move(ghaImageView));

        return imageId;
    }

    void RenderGraph::registerGraphOutput(RgResourceIdType resource) {
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

    RgPassIdType RenderGraph::addRenderPass(RgRenderPass::Descriptor passDescriptor) {
        RgPassIdType const renderPassId{ nextPassId++ };

        for(auto &renderTarget : passDescriptor.renderTargets) {
            RgResourceIdType const imageId{ renderTarget.target };

            auto &image{ images.at(imageId) };
            if(!image->isExternalImage()) {
                image->addImageUsage(GhaImage::UsageMode::ColourAttachment);
            }
            image->addWritePass(renderPassId);
        }

        {
            RgResourceIdType const imageId{ passDescriptor.depthStencil.target };

            auto &image{ images.at(imageId) };
            if(!image->isExternalImage()) {
                image->addImageUsage(GhaImage::UsageMode::DepthStencilAttachment);
            }
            image->addWritePass(renderPassId);
        }

        renderPasses[renderPassId] = std::make_unique<RgRenderPass>(renderPassId, std::move(passDescriptor));

        return renderPassId;
    }

    void RenderGraph::addRenderSubmission(RgPassIdType const renderPass, RgRenderPass::Submission submission) {
        auto &pass{ renderPasses.at(renderPass) };

        {
            RgResourceIdType const bufferId{ submission.vertexBuffer };

            auto &buffer{ buffers.at(bufferId) };
            if(!buffer->isExternalBuffer()) {
                buffer->addBufferUsage(GhaBuffer::UsageMode::VertexBuffer);
            }
            buffer->addReadPass(renderPass);
        }

        {
            RgResourceIdType const bufferId{ submission.indexBuffer };

            auto &buffer{ buffers.at(bufferId) };
            if(!buffer->isExternalBuffer()) {
                buffer->addBufferUsage(GhaBuffer::UsageMode::IndexBuffer);
            }
            buffer->addReadPass(renderPass);
        }

        for(auto const &ubo : submission.shaderUbos) {
            RgResourceIdType const bufferId{ ubo.buffer };

            auto &buffer{ buffers.at(bufferId) };
            if(!buffer->isExternalBuffer()) {
                buffer->addBufferUsage(GhaBuffer::UsageMode::UniformBuffer);
            }
            buffer->addReadPass(renderPass);
        }

        for(auto const &imageSampler : submission.shaderCombinedImageSamplers) {
            RgResourceIdType const imageId{ imageSampler.image };

            auto &image{ images.at(imageId) };
            if(!image->isExternalImage()) {
                image->addImageUsage(GhaImage::UsageMode::Sampled);
            }
            image->addReadPass(renderPass);
        }

        pass->addSubmission(std::move(submission));
    }

    GraphicsSubmitInfo RenderGraph::execute() {
        CLOVE_ASSERT(outputResource != INVALID_RESOURCE_ID, "No output resource has been specified");

        //Build an array of passes. This will be the execution order.
        std::vector<RgPassIdType> passes{};
        buildExecutionPasses(passes, outputResource);

        //Filter out any duplicates and then reverse order so the output pass is at the end
        {
            std::unordered_set<RgPassIdType> seenPasses{};
            for(auto iter{ passes.begin() }; iter != passes.end();) {
                if(!seenPasses.contains(*iter)) {
                    seenPasses.emplace(*iter);
                    ++iter;
                } else {
                    iter = passes.erase(iter);
                }
            }
        }
        std::reverse(passes.begin(), passes.end());

        //Allocate all of the GHA objects required for each pass
        std::unordered_map<RgPassIdType, std::shared_ptr<GhaRenderPass>> allocatedRenderPasses{};
        std::unordered_map<RgPassIdType, std::shared_ptr<GhaFramebuffer>> allocatedFramebuffers{};
        std::unordered_map<RgPassIdType, std::shared_ptr<GhaGraphicsPipelineObject>> allocatedPipelines{};
        std::unordered_map<RgPassIdType, std::shared_ptr<GhaDescriptorSetLayout>> descriptorSetLayouts{};
        std::unordered_map<RgPassIdType, std::vector<std::shared_ptr<GhaDescriptorSet>>> allocatedDescriptorSets{};
        std::unordered_map<RgResourceIdType, std::shared_ptr<GhaSampler>> allocatedSamplers{};

        std::unordered_map<DescriptorType, uint32_t> totalDescriptorBindingCount{};
        uint32_t totalDescriptorSets{ 0 };

        for(int32_t i{ 0 }; i < passes.size(); ++i) {
            RgPassIdType const passId{ passes[i] };
            if(!renderPasses.contains(passId)) {
                continue;
            }

            RgRenderPass::Descriptor const &passDescriptor{ renderPasses.at(passId)->getDescriptor() };
            std::vector<RgRenderPass::Submission> const &passSubmissions{ renderPasses.at(passId)->getSubmissions() };

            //Allocate image samplers
            for(auto const &submission : passSubmissions) {
                for(auto const &imageSampler : submission.shaderCombinedImageSamplers) {
                    allocatedSamplers[passId] = globalCache.createSampler(imageSampler.samplerState);
                }
            }

            //Build and allocate the render pass
            std::vector<AttachmentDescriptor> colourAttachments{};
            for(auto &renderTarget : passDescriptor.renderTargets) {
                colourAttachments.emplace_back(AttachmentDescriptor{
                    .format         = images[renderTarget.target]->getGhaImageView(frameCache)->getImageFormat(),
                    .loadOperation  = renderTarget.loadOp,
                    .storeOperation = renderTarget.storeOp,
                    .initialLayout  = getPreviousLayout(passes, i, renderTarget.target),
                    .usedLayout     = GhaImage::Layout::ColourAttachmentOptimal,
                    .finalLayout    = renderTarget.target == outputResource ? GhaImage::Layout::Present : GhaImage::Layout::ColourAttachmentOptimal,
                });
            }

            RgDepthStencilBinding const &depthStencil{ passDescriptor.depthStencil };
            AttachmentDescriptor depthStencilAttachment{
                .format         = images[depthStencil.target]->getGhaImageView(frameCache)->getImageFormat(),
                .loadOperation  = depthStencil.loadOp,
                .storeOperation = depthStencil.storeOp,
                .initialLayout  = getPreviousLayout(passes, i, depthStencil.target),
                .usedLayout     = GhaImage::Layout::DepthStencilAttachmentOptimal,
                .finalLayout    = GhaImage::Layout::DepthStencilAttachmentOptimal,
            };

            allocatedRenderPasses[passId] = globalCache.createRenderPass(GhaRenderPass::Descriptor{ .colourAttachments = std::move(colourAttachments), .depthAttachment = std::move(depthStencilAttachment) });

            //Build descriptor layouts using the first pass.
            //TODO: Get this infomation from shader reflection
            std::vector<DescriptorSetBindingInfo> descriptorBindings{};
            for(auto &ubo : passSubmissions[0].shaderUbos) {
                descriptorBindings.emplace_back(DescriptorSetBindingInfo{
                    .binding   = ubo.slot,
                    .type      = DescriptorType::UniformBuffer,
                    .arraySize = 1,
                    .stage     = GhaShader::Stage::Vertex,//TODO: provided by pass or by shader reflection
                });
            }
            for(auto &sampler : passSubmissions[0].shaderCombinedImageSamplers) {
                descriptorBindings.emplace_back(DescriptorSetBindingInfo{
                    .binding   = sampler.slot,
                    .type      = DescriptorType::CombinedImageSampler,
                    .arraySize = 1,
                    .stage     = GhaShader::Stage::Pixel,//TODO: provided by pass or by shader reflection
                });
            }

            descriptorSetLayouts[passId] = globalCache.createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor{ .bindings = std::move(descriptorBindings) });

            //Build graphics pipeline
            std::vector<VertexAttributeDescriptor> const vertexAttributes{
                VertexAttributeDescriptor{
                    .format = VertexAttributeFormat::R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex, position),
                },
            };

            //TEMP: Using dynamic for now
            AreaDescriptor const viewScissorArea{
                .state = ElementState::Dynamic,
            };

            allocatedPipelines[passId] = globalCache.createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor{
                .vertexShader       = allocatedShaders.at(passDescriptor.vertexShader),
                .pixelShader        = allocatedShaders.at(passDescriptor.pixelShader),
                .vertexInput        = Vertex::getInputBindingDescriptor(),
                .vertexAttributes   = vertexAttributes,
                .viewportDescriptor = viewScissorArea,
                .scissorDescriptor  = viewScissorArea,
                .depthState         = {
                    .depthTest  = passDescriptor.depthTest,
                    .depthWrite = passDescriptor.depthWrite,
                },
                .enableBlending       = passDescriptor.enableBlending,
                .renderPass           = allocatedRenderPasses.at(passId),
                .descriptorSetLayouts = { descriptorSetLayouts.at(passId) },
                .pushConstants        = {},
            });

            //Allocate the frame buffer
            std::vector<std::shared_ptr<GhaImageView>> attachments{};
            for(auto &renderTarget : passDescriptor.renderTargets) {
                attachments.push_back(images.at(renderTarget.target)->getGhaImageView(frameCache));
            }
            attachments.push_back(images.at(passDescriptor.depthStencil.target)->getGhaImageView(frameCache));

            allocatedFramebuffers[passId] = frameCache.allocateFramebuffer(GhaFramebuffer::Descriptor{
                .renderPass  = allocatedRenderPasses[passId],
                .attachments = std::move(attachments),
                .width       = images[passDescriptor.renderTargets[0].target]->getGhaImageView(frameCache)->getImageDimensions().x,//TEMP: Just using the first target as the size. This will need to be validated
                .height      = images[passDescriptor.renderTargets[0].target]->getGhaImageView(frameCache)->getImageDimensions().y,
            });

            //Count descriptor sets required for the entire pass
            for(auto const &submission : passSubmissions) {
                bool const hasUbo{ !submission.shaderUbos.empty() };
                bool const hasImageSampler{ !submission.shaderCombinedImageSamplers.empty() };

                if(hasUbo) {
                    totalDescriptorBindingCount[DescriptorType::UniformBuffer] += submission.shaderUbos.size();
                }
                if(hasImageSampler) {
                    totalDescriptorBindingCount[DescriptorType::CombinedImageSampler] += submission.shaderCombinedImageSamplers.size();
                }

                if(hasUbo || hasImageSampler) {
                    ++totalDescriptorSets;//Allocating a single set per submission
                }
            }
        }

        //Allocate descriptor sets
        //Create a single pool to allocate from
        std::vector<DescriptorInfo> descriptorTypes{};
        descriptorTypes.reserve(totalDescriptorBindingCount.size());

        for(auto &&[type, count] : totalDescriptorBindingCount) {
            descriptorTypes.emplace_back(DescriptorInfo{
                .type  = type,
                .count = count,
            });
        }

        std::shared_ptr<GhaDescriptorPool> descriptorPool{ frameCache.allocateDescriptorPool(GhaDescriptorPool::Descriptor{
            .poolTypes = std::move(descriptorTypes),
            .flag      = GhaDescriptorPool::Flag::None,
            .maxSets   = totalDescriptorSets,
        }) };
        descriptorPool->reset();//Make sure we have a fresh pool. TODO: do this inside frame cache? Or have a function to reset states

        //Create a descriptor set for each pipeline
        for(auto &&[id, pipeline] : allocatedPipelines) {
            std::vector<std::shared_ptr<GhaDescriptorSetLayout>> layouts(renderPasses.at(id)->getSubmissions().size(), pipeline->getDescriptor().descriptorSetLayouts[0]);//TEMP: Using first index as we know pipelines always have a single descriptor for now
            allocatedDescriptorSets[id] = descriptorPool->allocateDescriptorSets(layouts);
        }

        //Record all of the commands of each individual pass
        std::shared_ptr<GhaGraphicsCommandBuffer> graphicsCommandBufffer{ frameCache.getGraphicsCommandBuffer() };
        //std::shared_ptr<GhaComputeCommandBuffer> computeCommandBufffer{ frameCache.getComputeCommandBuffer() };
        //std::shared_ptr<GhaTransferCommandBuffer> transferCommandBufffer{ frameCache.getTransferCommandBuffer() };

        graphicsCommandBufffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
        for(RgPassIdType passId : passes) {
            if(transferPasses.contains(passId)) {
                RgTransferPass::BufferWrite const &writeOp{ transferPasses.at(passId)->getWriteOperation() };
                std::unique_ptr<RgBuffer> const &buffer{ buffers.at(writeOp.bufferId) };
                buffer->getGhaBuffer(frameCache)->write(writeOp.data.data(), buffer->getBufferOffset() + writeOp.offset, writeOp.size);
                continue;//Move to next pass as the rest of this loop assume it's a render pass
            }

            RgRenderPass::Descriptor const &passDescriptor{ renderPasses.at(passId)->getDescriptor() };
            std::vector<RgRenderPass::Submission> const &passSubmissions{ renderPasses.at(passId)->getSubmissions() };

            //TODO: Batch operations by render pass and start the pass outside of the operation
            RenderArea renderArea{
                .origin = passDescriptor.viewportPosition,
                .size   = passDescriptor.viewportSize,
            };

            std::vector<ClearValue> clearValues{};
            for(auto &target : passDescriptor.renderTargets) {
                clearValues.push_back(target.clearColour);
            }
            clearValues.push_back(passDescriptor.depthStencil.clearValue);

            graphicsCommandBufffer->beginRenderPass(*allocatedRenderPasses.at(passId), *allocatedFramebuffers.at(passId), renderArea, clearValues);

            //TODO: Only do this if viewport is dynamic
            graphicsCommandBufffer->setViewport(passDescriptor.viewportPosition, passDescriptor.viewportSize);
            graphicsCommandBufffer->setScissor({ 0, 0 }, passDescriptor.viewportSize);

            graphicsCommandBufffer->bindPipelineObject(*allocatedPipelines.at(passId));

            for(size_t index{ 0 }; auto const &submission : passSubmissions) {
                std::shared_ptr<GhaDescriptorSet> const &descriptorSet{ allocatedDescriptorSets.at(passId)[index] };

                for(auto const &ubo : submission.shaderUbos) {
                    //TODO: Handle different allocations within the same buffer
                    std::unique_ptr<RgBuffer> const &buffer{ buffers.at(ubo.buffer) };
                    descriptorSet->map(*buffer->getGhaBuffer(frameCache), 0, buffer->getBufferSize(), DescriptorType::UniformBuffer, ubo.slot);
                }
                for(auto const &imageSampler : submission.shaderCombinedImageSamplers) {
                    descriptorSet->map(*images.at(imageSampler.image)->getGhaImageView(frameCache), *allocatedSamplers.at(imageSampler.image), GhaImage::Layout::ShaderReadOnlyOptimal, imageSampler.slot);
                }

                graphicsCommandBufffer->bindDescriptorSet(*descriptorSet, 0);//TODO: Multiple sets / only set sets for a whole pass (i.e. view)

                std::unique_ptr<RgBuffer> const &vertexBuffer{ buffers.at(submission.vertexBuffer) };
                std::unique_ptr<RgBuffer> const &indexBuffer{ buffers.at(submission.indexBuffer) };
                graphicsCommandBufffer->bindVertexBuffer(*vertexBuffer->getGhaBuffer(frameCache), vertexBuffer->getBufferOffset());
                graphicsCommandBufffer->bindIndexBuffer(*indexBuffer->getGhaBuffer(frameCache), indexBuffer->getBufferOffset(), IndexType::Uint16);

                graphicsCommandBufffer->drawIndexed(submission.indexCount);

                ++index;
            }

            graphicsCommandBufffer->endRenderPass();
        }
        graphicsCommandBufffer->endRecording();

        //TODO: Execute this internally
        return GraphicsSubmitInfo{
            .commandBuffers = { graphicsCommandBufffer },
        };
    }

    void RenderGraph::buildExecutionPasses(std::vector<RgPassIdType> &passes, RgResourceIdType resourceId) {
        RgResource *resource{ getResourceFromId(resourceId) };

        std::vector<RgPassIdType> resourceWritePasses{};
        for(RgPassIdType passId : resource->getWritePasses()) {
            resourceWritePasses.push_back(passId);
        }

        passes.insert(passes.end(), resourceWritePasses.begin(), resourceWritePasses.end());

        for(RgPassIdType passId : resourceWritePasses) {
            RgPass *pass{ getPassFromId(passId) };
            for(RgResourceIdType resource : pass->getInputResources()) {
                buildExecutionPasses(passes, resource);
            }
        }
    }

    GhaImage::Layout RenderGraph::getPreviousLayout(std::vector<RgPassIdType> const &passes, int32_t const currentPassIndex, RgResourceIdType const imageId) {
        for(int32_t i = currentPassIndex - 1; i >= 0; --i) {
            if(!renderPasses.contains(passes[i])) {
                continue;//Only evaluate renderpasses for now
            }
            std::unique_ptr<RgRenderPass> const &renderPass{ renderPasses.at(passes[i]) };

            if(renderPass->getInputResources().contains(imageId)) {
                for(auto const &submission : renderPass->getSubmissions()) {
                    for(auto const &imageSamplers : submission.shaderCombinedImageSamplers) {
                        if(imageSamplers.image == imageId) {
                            return GhaImage::Layout::ShaderReadOnlyOptimal;
                        }
                    }
                }

                CLOVE_ASSERT(false, "ImageId is not in any render target's submissions even though it's marked as an input resource");
            } else if(renderPass->getOutputResources().contains(imageId)) {
                for(auto const &renderTarget : renderPass->getDescriptor().renderTargets) {
                    if(renderTarget.target == imageId) {
                        return GhaImage::Layout::ColourAttachmentOptimal;
                    }
                }
                if(renderPass->getDescriptor().depthStencil.target == imageId) {
                    return GhaImage::Layout::ColourAttachmentOptimal;
                }

                CLOVE_ASSERT(false, "ImageId is not in any render target's output even though it's marked as an output resource");
            }
        }

        return GhaImage::Layout::Undefined;
    }

    RgResource *RenderGraph::getResourceFromId(RgResourceIdType resourceId) {
        if(images.contains(resourceId)) {
            return images.at(resourceId).get();
        } else if(buffers.contains(resourceId)) {
            return buffers.at(resourceId).get();
        } else {
            return nullptr;
        }
    }

    RgPass *RenderGraph::getPassFromId(RgPassIdType passId) {
        if(renderPasses.contains(passId)) {
            return renderPasses.at(passId).get();
        } else if(transferPasses.contains(passId)) {
            return transferPasses.at(passId).get();
        } else {
            CLOVE_ASSERT(false, "Could not find pass ID");
            return nullptr;
        }
    }
}
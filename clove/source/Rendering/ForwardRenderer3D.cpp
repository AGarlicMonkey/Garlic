#include "Clove/Rendering/ForwardRenderer3D.hpp"

#include "Clove/Application.hpp"
#include "Clove/Rendering/Camera.hpp"
#include "Clove/Rendering/Material.hpp"
#include "Clove/Rendering/RenderPasses/DirectionalLightPass.hpp"
#include "Clove/Rendering/RenderPasses/ForwardColourPass.hpp"
#include "Clove/Rendering/RenderPasses/PointLightPass.hpp"
#include "Clove/Rendering/RenderPasses/SkinningPass.hpp"
#include "Clove/Rendering/RenderTarget.hpp"
#include "Clove/Rendering/Renderables/Mesh.hpp"
#include "Clove/Rendering/RenderingHelpers.hpp"
#include "Clove/Rendering/Vertex.hpp"

#include <Clove/Graphics/GhaDescriptorSet.hpp>
#include <Clove/Graphics/GhaImageView.hpp>
#include <Clove/Graphics/Graphics.hpp>
#include <Clove/Log/Log.hpp>
#include <Clove/Platform/Window.hpp>

extern "C" const char constants[];
extern "C" const size_t constantsLength;

extern "C" const char ui_v[];
extern "C" const size_t ui_vLength;
extern "C" const char widget_p[];
extern "C" const size_t widget_pLength;
extern "C" const char font_p[];
extern "C" const size_t font_pLength;

namespace garlic::clove {
    ForwardRenderer3D::ForwardRenderer3D(GhaDevice *graphicsDevice, std::unique_ptr<RenderTarget> renderTarget)
        : ghaDevice{ graphicsDevice }
        , renderTarget{ std::move(renderTarget) } {
        shaderIncludes["Constants.glsl"] = { constants, constantsLength };

        renderTargetPropertyChangedBeginHandle = this->renderTarget->onPropertiesChangedBegin.bind(&ForwardRenderer3D::cleanupRenderTargetResources, this);
        renderTargetPropertyChangedEndHandle   = this->renderTarget->onPropertiesChangedEnd.bind(&ForwardRenderer3D::createRenderTargetResources, this);

        ghaFactory = graphicsDevice->getGraphicsFactory();

        //Object initialisation
        graphicsQueue = *ghaFactory->createGraphicsQueue({ QueueFlags::Transient | QueueFlags::ReuseBuffers });
        computeQueue  = *ghaFactory->createComputeQueue({ QueueFlags::Transient | QueueFlags::ReuseBuffers });

        descriptorSetLayouts[DescriptorSetSlots::Mesh]     = createMeshDescriptorSetLayout(*ghaFactory);
        descriptorSetLayouts[DescriptorSetSlots::View]     = createViewDescriptorSetLayout(*ghaFactory);
        descriptorSetLayouts[DescriptorSetSlots::Lighting] = createLightingDescriptorSetLayout(*ghaFactory);
        descriptorSetLayouts[DescriptorSetSlots::UI]       = createUiDescriptorSetLayout(*ghaFactory);

        skinningSetLayout = createSkinningDescriptorSetLayout(*ghaFactory);

        createRenderpass();
        createShadowMapRenderpass();

        //Create the geometry passes this renderer supports
        geometryPasses[GeometryPass::getId<ForwardColourPass>()]    = std::make_unique<ForwardColourPass>(*ghaFactory, renderPass);
        geometryPasses[GeometryPass::getId<DirectionalLightPass>()] = std::make_unique<DirectionalLightPass>(*ghaFactory, shadowMapRenderPass);
        geometryPasses[GeometryPass::getId<PointLightPass>()]       = std::make_unique<PointLightPass>(*ghaFactory, shadowMapRenderPass);
        geometryPasses[GeometryPass::getId<SkinningPass>()]         = std::make_unique<SkinningPass>(*ghaFactory);

        createUiPipeline();

        createRenderTargetResources();

        //Create semaphores for frame synchronisation
        for(auto &skinningFinishedSemaphore : skinningFinishedSemaphores) {
            skinningFinishedSemaphore = *ghaFactory->createSemaphore();
        }

        float constexpr anisotropy{ 16.0f };
        textureSampler = *ghaFactory->createSampler(GhaSampler::Descriptor{
            .minFilter        = GhaSampler::Filter::Linear,
            .magFilter        = GhaSampler::Filter::Linear,
            .addressModeU     = GhaSampler::AddressMode::Repeat,
            .addressModeV     = GhaSampler::AddressMode::Repeat,
            .addressModeW     = GhaSampler::AddressMode::Repeat,
            .enableAnisotropy = true,
            .maxAnisotropy    = anisotropy,
        });

        uiSampler = *ghaFactory->createSampler(GhaSampler::Descriptor{
            .minFilter        = GhaSampler::Filter::Nearest,
            .magFilter        = GhaSampler::Filter::Nearest,
            .addressModeU     = GhaSampler::AddressMode::ClampToBorder,
            .addressModeV     = GhaSampler::AddressMode::ClampToBorder,
            .addressModeW     = GhaSampler::AddressMode::ClampToBorder,
            .enableAnisotropy = false,
        });

        shadowSampler = *ghaFactory->createSampler(GhaSampler::Descriptor{
            .minFilter        = GhaSampler::Filter::Linear,
            .magFilter        = GhaSampler::Filter::Linear,
            .addressModeU     = GhaSampler::AddressMode::ClampToBorder,
            .addressModeV     = GhaSampler::AddressMode::ClampToBorder,
            .addressModeW     = GhaSampler::AddressMode::ClampToBorder,
            .enableAnisotropy = false,
        });

        std::vector<Vertex> const uiVertices{
            Vertex{
                .position = { 0.0f, 1.0f, 0.0f },
                .texCoord = { 0.0f, 0.0f },
            },
            Vertex{
                .position = { 1.0f, 1.0f, 0.0f },
                .texCoord = { 1.0f, 0.0f },
            },
            Vertex{
                .position = { 0.0f, 0.0f, 0.0f },
                .texCoord = { 0.0f, 1.0f },
            },
            Vertex{
                .position = { 1.0f, 0.0f, 0.0f },
                .texCoord = { 1.0f, 1.0f },
            },
        };

        std::vector<uint16_t> const uiIndices{
            0,
            2,
            3,
            0,
            3,
            1,
        };

        uiMesh = std::make_unique<Mesh>(uiVertices, uiIndices);
    }

    //ForwardRenderer3D::ForwardRenderer3D(ForwardRenderer3D&& other) noexcept = default;

    ForwardRenderer3D &ForwardRenderer3D::operator=(ForwardRenderer3D &&other) noexcept = default;

    ForwardRenderer3D::~ForwardRenderer3D() {
        //Wait for an idle device before shutting down so resources aren't freed while in use
        ghaDevice->waitForIdleDevice();

        //Reset these manually as they would fail after the device has been destroyed (how to solve this?)
        textureSampler.reset();
        for(auto &imageData : inFlightImageData) {
            imageData.frameDataBuffer.reset();

            graphicsQueue->freeCommandBuffer(*imageData.commandBuffer);
            graphicsQueue->freeCommandBuffer(*imageData.shadowMapCommandBuffer);
            graphicsQueue->freeCommandBuffer(*imageData.cubeShadowMapCommandBuffer);
        }
    }

    void ForwardRenderer3D::begin() {
        currentFrameData.meshes.clear();
        currentFrameData.widgets.clear();
        currentFrameData.text.clear();

        currentFrameData.bufferData.numLights.numDirectional = 0;
        currentFrameData.bufferData.numLights.numPoint       = 0;
    }

    void ForwardRenderer3D::submitMesh(MeshInfo meshInfo, std::set<GeometryPass::Id> const &geometryPassIds) {
        currentFrameData.meshes.push_back(std::move(meshInfo));
        for(auto id : geometryPassIds) {
            geometryPasses[id]->addJob({ currentFrameData.meshes.size() - 1, currentFrameData.meshes.back().mesh });
        }
    }

    void ForwardRenderer3D::submitCamera(Camera const &camera, vec3f position) {
        currentFrameData.bufferData.viewData.view       = camera.getView();
        currentFrameData.bufferData.viewData.projection = camera.getProjection();

        currentFrameData.bufferData.viewPosition = position;
    }

    void ForwardRenderer3D::submitLight(DirectionalLight const &light) {
        uint32_t const lightIndex = currentFrameData.bufferData.numLights.numDirectional++;

        currentFrameData.bufferData.lights.directionalLights[lightIndex]    = light.data;
        currentFrameData.bufferData.directionalShadowTransforms[lightIndex] = light.shadowTransform;
    }

    void ForwardRenderer3D::submitLight(PointLight const &light) {
        uint32_t const lightIndex = currentFrameData.bufferData.numLights.numPoint++;

        currentFrameData.bufferData.lights.pointLights[lightIndex] = light.data;
        currentFrameData.pointShadowTransforms[lightIndex]         = light.shadowTransforms;
    }

    void ForwardRenderer3D::submitWidget(std::shared_ptr<GhaImageView> const &widget, mat4f const modelProjection) {
        currentFrameData.widgets.emplace_back(widget, modelProjection);
    }

    void ForwardRenderer3D::submitText(std::shared_ptr<GhaImageView> const &text, mat4f const modelProjection) {
        currentFrameData.text.emplace_back(text, modelProjection);
    }

    void ForwardRenderer3D::end() {
        //Aquire the next available image from the render target
        Expected<uint32_t, std::string> const result{ renderTarget->aquireNextImage(currentFrame) };
        if(!result.hasValue()) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Debug, result.getError());
            return;
        }

        size_t const imageIndex{ result.getValue() };

        ImageData &currentImageData{ inFlightImageData[imageIndex] };

        //Rendering constants / globals
        RenderArea renderArea{
            .origin = { 0, 0 },
            .size   = renderTarget->getSize(),
        };
        RenderArea shadowArea{
            .origin = { 0, 0 },
            .size   = { shadowMapSize, shadowMapSize }
        };

        vec4f clearColour{ 0.0f, 0.0f, 0.0f, 1.0f };
        DepthStencilValue depthStencilClearValue{ 1.0f, 0 };

        std::array<ClearValue, 2> outputClearValues{
            clearColour,
            depthStencilClearValue
        };
        std::array<ClearValue, 1> shadowMapClearValues{
            depthStencilClearValue
        };

        size_t const meshCount{ currentFrameData.meshes.size() };

        //We can just write the struct straight in as all the mappings are based off of it's layout
        currentImageData.frameDataBuffer->write(&currentFrameData, 0, sizeof(currentFrameData));

        //Map any non-UBO pieces of data (such as textures / shadow maps)
        currentImageData.lightingDescriptorSet->map(currentImageData.shadowMapViews, *shadowSampler, GhaImage::Layout::ShaderReadOnlyOptimal, 3);
        currentImageData.lightingDescriptorSet->map(currentImageData.cubeShadowMapViews, *shadowSampler, GhaImage::Layout::ShaderReadOnlyOptimal, 4);

        //Allocate a descriptor set for each mesh to be drawn
        std::vector<std::shared_ptr<GhaDescriptorSet>> meshSets;
        if(meshCount > 0) {
            if(currentImageData.meshDescriptorPool == nullptr || currentImageData.meshDescriptorPool->getDescriptor().maxSets < meshCount) {
                auto meshSetBindingCount = countDescriptorBindingTypes(*descriptorSetLayouts[DescriptorSetSlots::Mesh]);
                for(auto &[key, val] : meshSetBindingCount) {
                    val *= meshCount;
                }
                currentImageData.meshDescriptorPool = createDescriptorPool(meshSetBindingCount, meshCount);
            }

            currentImageData.meshDescriptorPool->reset();
            std::vector<std::shared_ptr<GhaDescriptorSetLayout>> layouts(meshCount, descriptorSetLayouts[DescriptorSetSlots::Mesh]);
            meshSets = currentImageData.meshDescriptorPool->allocateDescriptorSets(layouts);
        }

        struct MeshUBOLayout {
            alignas(256) ModelData model;                           //NOLINT
            alignas(256) vec4f colour;                              //NOLINT
            alignas(256) std::array<mat4f, MAX_JOINTS> matrixPallet;//NOLINT
        };

        auto const writeObjectBuffer = [&graphicsFactory = ghaFactory](std::unique_ptr<GhaBuffer> &buffer, MeshUBOLayout const &layout) {
            if(buffer == nullptr) {
                buffer = *graphicsFactory->createBuffer(GhaBuffer::Descriptor{
                    .size        = sizeof(layout),
                    .usageFlags  = GhaBuffer::UsageMode::UniformBuffer,
                    .sharingMode = SharingMode::Exclusive,
                    .memoryType  = MemoryType::SystemMemory,
                });
            }
            buffer->write(&layout, 0, sizeof(layout));
        };

        //Populate and map the UBOs for each mesh
        if(std::size(currentImageData.objectBuffers) < meshCount) {
            currentImageData.objectBuffers.resize(meshCount);
        }
        for(size_t index = 0; auto &meshInfo : currentFrameData.meshes) {
            MeshUBOLayout layout{
                .model{
                    meshInfo.transform,
                    inverse(transpose(meshInfo.transform)),
                },
                .colour       = meshInfo.material->getColour(),
                .matrixPallet = meshInfo.matrixPalet,
            };

            writeObjectBuffer(currentImageData.objectBuffers[index], layout);

            std::shared_ptr<GhaDescriptorSet> &meshDescriptorSet = meshSets[index];
            meshDescriptorSet->map(*meshInfo.material->getDiffuseView(), *textureSampler, GhaImage::Layout::ShaderReadOnlyOptimal, 0);
            meshDescriptorSet->map(*meshInfo.material->getSpecularView(), *textureSampler, GhaImage::Layout::ShaderReadOnlyOptimal, 1);
            meshDescriptorSet->map(*currentImageData.objectBuffers[index], offsetof(MeshUBOLayout, model), sizeof(ModelData), DescriptorType::UniformBuffer, 2);
            //meshDescriptorSet->map(*currentImageData.objectBuffers[index], offsetof(MeshUBOLayout, matrixPallet), sizeof(mat4f) * MAX_JOINTS, DescriptorType::UniformBuffer, 3);
            meshDescriptorSet->map(*currentImageData.objectBuffers[index], offsetof(MeshUBOLayout, colour), sizeof(vec4f), DescriptorType::UniformBuffer, 4);

            ++index;
        }

        //Frame data for the geometry passes
        GeometryPass::FrameData geometryPassData{
            .meshDescriptorSets    = meshSets,
            .viewDescriptorSet     = currentImageData.viewDescriptorSet,
            .lightingDescriptorSet = currentImageData.lightingDescriptorSet,
        };

        //SKINNING
        //TEMP: Create the desciptor sets for the skinning pass
        std::vector<std::shared_ptr<GhaDescriptorSet>> skinningSets;
        if(meshCount > 0) {
            if(currentImageData.skinningDescriptorPool == nullptr || currentImageData.skinningDescriptorPool->getDescriptor().maxSets < meshCount) {
                auto setBindingCount{ countDescriptorBindingTypes(*skinningSetLayout) };
                for(auto &[key, val] : setBindingCount) {
                    val *= meshCount;
                }
                currentImageData.skinningDescriptorPool = createDescriptorPool(setBindingCount, meshCount);
            }

            currentImageData.skinningDescriptorPool->reset();
            std::vector<std::shared_ptr<GhaDescriptorSetLayout>> layouts(meshCount, skinningSetLayout);
            skinningSets = currentImageData.skinningDescriptorPool->allocateDescriptorSets(layouts);
        }
        //Map the descriptor sets
        for(size_t index = 0; auto &meshInfo : currentFrameData.meshes) {
            std::shared_ptr<GhaDescriptorSet> &skinningDescriptorSet{ skinningSets[index] };
            //Use the previously created buffer
            skinningDescriptorSet->map(*currentImageData.objectBuffers[index], offsetof(MeshUBOLayout, matrixPallet), sizeof(mat4f) * MAX_JOINTS, DescriptorType::UniformBuffer, 0);
            skinningDescriptorSet->map(*meshInfo.mesh->getVertexBuffer(), 0, meshInfo.mesh->getVertexBufferSize(), DescriptorType::StorageBuffer, 1);
            skinningDescriptorSet->map(*meshInfo.mesh->getCombinedBuffer(), meshInfo.mesh->getVertexOffset(), meshInfo.mesh->getVertexBufferSize(), DescriptorType::StorageBuffer, 2);

            ++index;
        }

        //Dispatch all the commands
        geometryPassData.skinningMeshSets = skinningSets;
        currentImageData.skinningCommandBuffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
        geometryPasses[GeometryPass::getId<SkinningPass>()]->execute(*currentImageData.skinningCommandBuffer, geometryPassData);
        currentImageData.skinningCommandBuffer->endRecording();
        geometryPasses[GeometryPass::getId<SkinningPass>()]->flushJobs();

        //Submit the command buffer for the skinning
        ComputeSubmitInfo skinningSubmitInfo{
            .commandBuffers   = { currentImageData.skinningCommandBuffer },
            .signalSemaphores = { skinningFinishedSemaphores[currentFrame] },
        };
        computeQueue->submit({ std::move(skinningSubmitInfo) }, nullptr);

        //DIRECTIONAL LIGHT SHADOWS
        currentImageData.shadowMapCommandBuffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
        for(size_t i = 0; i < MAX_LIGHTS; ++i) {
            if(i < currentFrameData.bufferData.numLights.numDirectional) {
                currentImageData.shadowMapCommandBuffer->beginRenderPass(*shadowMapRenderPass, *currentImageData.shadowMapFrameBuffers[i], shadowArea, shadowMapClearValues);

                geometryPassData.currentDirLightTransform = &currentFrameData.bufferData.directionalShadowTransforms[i];
                geometryPasses[GeometryPass::getId<DirectionalLightPass>()]->execute(*currentImageData.shadowMapCommandBuffer, geometryPassData);

                currentImageData.shadowMapCommandBuffer->endRenderPass();
            } else {
                //Make sure transition the layout of the images we don't render to as these get sent to colour pixel shader anyway
                ImageMemoryBarrierInfo const memoryBarrier{
                    .currentImageLayout = GhaImage::Layout::Undefined,
                    .newImageLayout     = GhaImage::Layout::ShaderReadOnlyOptimal,
                };
                currentImageData.shadowMapCommandBuffer->imageMemoryBarrier(*currentImageData.shadowMaps[i], memoryBarrier, PipelineStage::Top, PipelineStage::Top);
            }
        }
        currentImageData.shadowMapCommandBuffer->endRecording();
        geometryPasses[GeometryPass::getId<DirectionalLightPass>()]->flushJobs();

        //POINT LIGHT SHADOWS
        currentImageData.cubeShadowMapCommandBuffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
        for(size_t i{ 0 }; i < MAX_LIGHTS; ++i) {
            for(size_t j{ 0 }; j < cubeMapLayerCount; ++j) {
                if(i < currentFrameData.bufferData.numLights.numPoint) {
                    currentImageData.cubeShadowMapCommandBuffer->beginRenderPass(*shadowMapRenderPass, *currentImageData.cubeShadowMapFrameBuffers[i][j], shadowArea, shadowMapClearValues);

                    geometryPassData.currentPointLightTransform = &currentFrameData.pointShadowTransforms[i][j];
                    geometryPassData.currentPointLightPosition  = currentFrameData.bufferData.lights.pointLights[i].position;
                    geometryPassData.currentPointLightFarPlane  = currentFrameData.bufferData.lights.pointLights[i].farPlane;
                    geometryPasses[GeometryPass::getId<PointLightPass>()]->execute(*currentImageData.cubeShadowMapCommandBuffer, geometryPassData);

                    currentImageData.cubeShadowMapCommandBuffer->endRenderPass();
                } else {
                    //Make sure transition the layout of the images we don't render to as these get sent to colour pixel shader anyway
                    ImageMemoryBarrierInfo const memoryBarrier{
                        .currentImageLayout = GhaImage::Layout::Undefined,
                        .newImageLayout     = GhaImage::Layout::ShaderReadOnlyOptimal,
                    };
                    currentImageData.cubeShadowMapCommandBuffer->imageMemoryBarrier(*currentImageData.cubeShadowMaps[i], memoryBarrier, PipelineStage::Top, PipelineStage::Top);
                }
            }
        }
        currentImageData.cubeShadowMapCommandBuffer->endRecording();
        geometryPasses[GeometryPass::getId<PointLightPass>()]->flushJobs();

        //Submit the command buffers for the shadow maps.
        GraphicsSubmitInfo cubeShadowSubmitInfo{
            .waitSemaphores   = { { skinningFinishedSemaphores[currentFrame], PipelineStage::VertexShader } },
            .commandBuffers   = { currentImageData.shadowMapCommandBuffer, currentImageData.cubeShadowMapCommandBuffer }
        };
        graphicsQueue->submit({ std::move(cubeShadowSubmitInfo) }, nullptr);

        //Allocate a descriptor set for each ui element to be drawn
        size_t const widgetCount{ std::size(currentFrameData.widgets) };
        size_t const textCount{ std::size(currentFrameData.text) };
        size_t const uiElementCount{ widgetCount + textCount };

        std::vector<std::shared_ptr<GhaDescriptorSet>> uiSets;
        if(uiElementCount > 0) {
            if(currentImageData.uiDescriptorPool == nullptr || currentImageData.uiDescriptorPool->getDescriptor().maxSets < uiElementCount) {
                auto uiSetBindingCount = countDescriptorBindingTypes(*descriptorSetLayouts[DescriptorSetSlots::UI]);
                for(auto &[key, val] : uiSetBindingCount) {
                    val *= uiElementCount;
                }
                currentImageData.uiDescriptorPool = createDescriptorPool(uiSetBindingCount, uiElementCount);
            }

            currentImageData.uiDescriptorPool->reset();
            std::vector<std::shared_ptr<GhaDescriptorSetLayout>> uiLayouts(uiElementCount, descriptorSetLayouts[DescriptorSetSlots::UI]);
            uiSets = currentImageData.uiDescriptorPool->allocateDescriptorSets(uiLayouts);
        }

        //Lambda used to draw a mesh
        auto const drawMesh = [](GhaGraphicsCommandBuffer &commandBuffer, Mesh const &mesh) {
            commandBuffer.bindVertexBuffer(*mesh.getCombinedBuffer(), mesh.getVertexOffset());
            commandBuffer.bindIndexBuffer(*mesh.getCombinedBuffer(), mesh.getIndexOffset(), IndexType::Uint16);

            commandBuffer.drawIndexed(mesh.getIndexCount());
        };

        //FINAL COLOUR
        {
            currentImageData.commandBuffer->beginRecording(CommandBufferUsage::OneTimeSubmit);
            currentImageData.commandBuffer->beginRenderPass(*renderPass, *frameBuffers[imageIndex], renderArea, outputClearValues);

            currentImageData.commandBuffer->setViewport(vec2i{ 0 }, renderTarget->getSize());
            currentImageData.commandBuffer->setScissor(vec2i{ 0 }, renderTarget->getSize());

            geometryPasses[GeometryPass::getId<ForwardColourPass>()]->execute(*currentImageData.commandBuffer, geometryPassData);
            geometryPasses[GeometryPass::getId<ForwardColourPass>()]->flushJobs();

            //Widgets
            currentImageData.commandBuffer->bindPipelineObject(*widgetPipelineObject);
            for(size_t index{ 0 }; auto &&[texture, modelProj] : currentFrameData.widgets) {
                vec4f constexpr colour{ 1.0f };//Temp colour

                uiSets[index]->map(*texture, *uiSampler, GhaImage::Layout::ShaderReadOnlyOptimal, 0);

                currentImageData.commandBuffer->bindDescriptorSet(*uiSets[index], 0);
                currentImageData.commandBuffer->pushConstant(GhaShader::Stage::Vertex, 0, sizeof(modelProj), &modelProj);
                currentImageData.commandBuffer->pushConstant(GhaShader::Stage::Pixel, sizeof(modelProj), sizeof(colour), &colour);

                drawMesh(*currentImageData.commandBuffer, *uiMesh);

                ++index;
            }

            //Text
            currentImageData.commandBuffer->bindPipelineObject(*textPipelineObject);
            for(size_t index{ widgetCount }; auto &&[texture, modelProj] : currentFrameData.text) {
                vec4f constexpr colour{ 1.0f };//Temp colour

                uiSets[index]->map(*texture, *uiSampler, GhaImage::Layout::ShaderReadOnlyOptimal, 0);

                currentImageData.commandBuffer->bindDescriptorSet(*uiSets[index], 0);
                currentImageData.commandBuffer->pushConstant(GhaShader::Stage::Vertex, 0, sizeof(modelProj), &modelProj);
                currentImageData.commandBuffer->pushConstant(GhaShader::Stage::Pixel, sizeof(modelProj), sizeof(colour), &colour);

                drawMesh(*currentImageData.commandBuffer, *uiMesh);

                ++index;
            }

            currentImageData.commandBuffer->endRenderPass();
            currentImageData.commandBuffer->endRecording();
        }

        //Submit the colour output to the render target
        GraphicsSubmitInfo submitInfo{
            .commandBuffers = { currentImageData.commandBuffer },
        };
        renderTarget->submit(imageIndex, currentFrame, std::move(submitInfo));

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

    void ForwardRenderer3D::cleanupRenderTargetResources() {
        ghaDevice->waitForIdleDevice();

        frameBuffers.clear();

        for(auto &imageData : inFlightImageData) {
            graphicsQueue->freeCommandBuffer(*imageData.commandBuffer);
            graphicsQueue->freeCommandBuffer(*imageData.shadowMapCommandBuffer);
            graphicsQueue->freeCommandBuffer(*imageData.cubeShadowMapCommandBuffer);
            computeQueue->freeCommandBuffer(*imageData.skinningCommandBuffer);
        }
    }

    void ForwardRenderer3D::createRenderTargetResources() {
        createDepthBuffer();
        createRenderTargetFrameBuffers();

        size_t const imageCount{ std::size(frameBuffers) };

        inFlightImageData.resize(imageCount);

        //Allocate frame scope descriptor pools
        auto viewSetBindingCount{ countDescriptorBindingTypes(*descriptorSetLayouts[DescriptorSetSlots::View]) };
        auto lightingSetBindingCount{ countDescriptorBindingTypes(*descriptorSetLayouts[DescriptorSetSlots::Lighting]) };

        uint32_t constexpr totalSets{ 2 };//Only 2 sets will be allocated from these pools (view + lighting)
        auto bindingCounts = viewSetBindingCount;
        for(auto &&[type, count] : lightingSetBindingCount) {
            bindingCounts[type] += count;
        }

        for(auto &imageData : inFlightImageData) {
            //Create command buffers
            imageData.commandBuffer              = graphicsQueue->allocateCommandBuffer();
            imageData.shadowMapCommandBuffer     = graphicsQueue->allocateCommandBuffer();
            imageData.cubeShadowMapCommandBuffer = graphicsQueue->allocateCommandBuffer();
            imageData.skinningCommandBuffer      = computeQueue->allocateCommandBuffer();

            //Create uniform buffers
            imageData.frameDataBuffer = *ghaFactory->createBuffer(GhaBuffer::Descriptor{
                .size        = sizeof(FrameData),
                .usageFlags  = GhaBuffer::UsageMode::UniformBuffer,
                .sharingMode = SharingMode::Exclusive,
                .memoryType  = MemoryType::SystemMemory,
            });

            //Allocate frame scope descriptor Sets
            imageData.frameDescriptorPool = createDescriptorPool(bindingCounts, totalSets);

            imageData.viewDescriptorSet     = imageData.frameDescriptorPool->allocateDescriptorSets(descriptorSetLayouts[DescriptorSetSlots::View]);
            imageData.lightingDescriptorSet = imageData.frameDescriptorPool->allocateDescriptorSets(descriptorSetLayouts[DescriptorSetSlots::Lighting]);

            //As we only have one UBO per frame for every GhaDescriptorSet we can map the buffer into them straight away
            imageData.viewDescriptorSet->map(*imageData.frameDataBuffer, offsetof(FrameData::BufferData, viewData), sizeof(currentFrameData.bufferData.viewData), DescriptorType::UniformBuffer, 0);
            imageData.viewDescriptorSet->map(*imageData.frameDataBuffer, offsetof(FrameData::BufferData, viewPosition), sizeof(currentFrameData.bufferData.viewPosition), DescriptorType::UniformBuffer, 1);

            imageData.lightingDescriptorSet->map(*imageData.frameDataBuffer, offsetof(FrameData::BufferData, lights), sizeof(currentFrameData.bufferData.lights), DescriptorType::UniformBuffer, 0);
            imageData.lightingDescriptorSet->map(*imageData.frameDataBuffer, offsetof(FrameData::BufferData, numLights), sizeof(currentFrameData.bufferData.numLights), DescriptorType::UniformBuffer, 1);
            imageData.lightingDescriptorSet->map(*imageData.frameDataBuffer, offsetof(FrameData::BufferData, directionalShadowTransforms), sizeof(currentFrameData.bufferData.directionalShadowTransforms), DescriptorType::UniformBuffer, 2);

            //Create the shadow maps for each frame
            for(size_t i = 0; i < MAX_LIGHTS; ++i) {
                //Directional
                imageData.shadowMaps[i]     = *ghaFactory->createImage(GhaImage::Descriptor{
                    .type        = GhaImage::Type::_2D,
                    .usageFlags  = GhaImage::UsageMode::Sampled | GhaImage::UsageMode::DepthStencilAttachment,
                    .dimensions  = { shadowMapSize, shadowMapSize },
                    .format      = GhaImage::Format::D32_SFLOAT,
                    .sharingMode = SharingMode::Exclusive,
                });
                imageData.shadowMapViews[i] = imageData.shadowMaps[i]->createView(GhaImageView::Descriptor{
                    .type       = GhaImageView::Type::_2D,
                    .layer      = 0,
                    .layerCount = 1,
                });

                imageData.shadowMapFrameBuffers[i] = *ghaFactory->createFramebuffer(GhaFramebuffer::Descriptor{
                    .renderPass  = shadowMapRenderPass,
                    .attachments = { imageData.shadowMapViews[i] },
                    .width       = shadowMapSize,
                    .height      = shadowMapSize,
                });

                //Point
                imageData.cubeShadowMaps[i]     = *ghaFactory->createImage(GhaImage::Descriptor{
                    .type        = GhaImage::Type::Cube,
                    .usageFlags  = GhaImage::UsageMode::Sampled | GhaImage::UsageMode::DepthStencilAttachment,
                    .dimensions  = { shadowMapSize, shadowMapSize },
                    .format      = GhaImage::Format::D32_SFLOAT,
                    .sharingMode = SharingMode::Exclusive,
                });
                imageData.cubeShadowMapViews[i] = imageData.cubeShadowMaps[i]->createView(GhaImageView::Descriptor{
                    .type       = GhaImageView::Type::Cube,
                    .layer      = 0,
                    .layerCount = cubeMapLayerCount,
                });

                for(size_t j = 0; j < cubeMapLayerCount; ++j) {
                    imageData.cubeShadowMapFaceViews[i][j] = imageData.cubeShadowMaps[i]->createView(GhaImageView::Descriptor{
                        .type       = GhaImageView::Type::_2D,
                        .layer      = static_cast<uint32_t>(j),
                        .layerCount = 1,
                    });

                    imageData.cubeShadowMapFrameBuffers[i][j] = *ghaFactory->createFramebuffer(GhaFramebuffer::Descriptor{
                        .renderPass  = shadowMapRenderPass,
                        .attachments = { imageData.cubeShadowMapFaceViews[i][j] },
                        .width       = shadowMapSize,
                        .height      = shadowMapSize,
                    });
                }
            }
        }
    }

    void ForwardRenderer3D::createRenderpass() {
        //Define what attachments we have
        AttachmentDescriptor colourAttachment{
            .format         = renderTarget->getImageFormat(),
            .loadOperation  = LoadOperation::Clear,
            .storeOperation = StoreOperation::Store,
            .initialLayout  = GhaImage::Layout::Undefined,
            .finalLayout    = GhaImage::Layout::Present,
        };

        AttachmentDescriptor depthAttachment{
            .format         = GhaImage::Format::D32_SFLOAT,
            .loadOperation  = LoadOperation::Clear,
            .storeOperation = StoreOperation::DontCare,
            .initialLayout  = GhaImage::Layout::Undefined,
            .finalLayout    = GhaImage::Layout::DepthStencilAttachmentOptimal,
        };

        //Define attachment references so the subpass knows which slot each attachment will be in
        AttachmentReference colourReference{
            .attachmentIndex = 0,
            .layout          = GhaImage::Layout::ColourAttachmentOptimal,
        };

        AttachmentReference depthReference{
            .attachmentIndex = 1,
            .layout          = GhaImage::Layout::DepthStencilAttachmentOptimal,
        };

        SubpassDescriptor subpass{
            .colourAttachments = { colourReference },
            .depthAttachment   = depthReference,
        };

        //Wait on the implicit subpass at the start. This is so the subpass can transition the layout at the right time
        SubpassDependency dependency{
            .sourceSubpass      = SUBPASS_EXTERNAL,
            .destinationSubpass = 0,
            .sourceStage        = PipelineStage::ColourAttachmentOutput,
            .destinationStage   = PipelineStage::ColourAttachmentOutput,
            .currentAccess      = AccessFlags::None,
            .newAccess          = AccessFlags::ColourAttachmentWrite,
        };

        //Create render pass
        GhaRenderPass::Descriptor renderPassDescriptor{
            .attachments  = { colourAttachment, depthAttachment },
            .subpasses    = { std::move(subpass) },
            .dependencies = { dependency },
        };

        renderPass = *ghaFactory->createRenderPass(std::move(renderPassDescriptor));
    }

    void ForwardRenderer3D::createShadowMapRenderpass() {
        AttachmentDescriptor depthAttachment{
            .format         = GhaImage::Format::D32_SFLOAT,
            .loadOperation  = LoadOperation::Clear,
            .storeOperation = StoreOperation::Store,
            .initialLayout  = GhaImage::Layout::Undefined,
            .finalLayout    = GhaImage::Layout::ShaderReadOnlyOptimal,
        };

        AttachmentReference depthReference{
            .attachmentIndex = 0,
            .layout          = GhaImage::Layout::DepthStencilAttachmentOptimal,
        };

        SubpassDescriptor subpass{
            .colourAttachments = {},
            .depthAttachment   = depthReference,
        };

        GhaRenderPass::Descriptor renderPassDescriptor{
            .attachments  = { depthAttachment },
            .subpasses    = { std::move(subpass) },
            .dependencies = {},
        };

        shadowMapRenderPass = *ghaFactory->createRenderPass(std::move(renderPassDescriptor));
    }

    void ForwardRenderer3D::createDepthBuffer() {
        depthImage     = *ghaFactory->createImage(GhaImage::Descriptor{
            .type        = GhaImage::Type::_2D,
            .usageFlags  = GhaImage::UsageMode::DepthStencilAttachment,
            .dimensions  = renderTarget->getSize(),
            .format      = GhaImage::Format::D32_SFLOAT,
            .sharingMode = SharingMode::Exclusive,
        });
        depthImageView = depthImage->createView(GhaImageView::Descriptor{
            .type       = GhaImageView::Type::_2D,
            .layer      = 0,
            .layerCount = 1,
        });
    }

    void ForwardRenderer3D::createUiPipeline() {
        size_t constexpr totalAttributes{ 2 };
        std::vector<VertexAttributeDescriptor> vertexAttributes{};
        vertexAttributes.reserve(totalAttributes);

        vertexAttributes.emplace_back(VertexAttributeDescriptor{
            .location = 0,
            .format   = VertexAttributeFormat::R32G32B32_SFLOAT,
            .offset   = offsetof(Vertex, position),
        });
        vertexAttributes.emplace_back(VertexAttributeDescriptor{
            .location = 1,
            .format   = VertexAttributeFormat::R32G32B32_SFLOAT,
            .offset   = offsetof(Vertex, texCoord),
        });

        AreaDescriptor viewScissorArea{
            .state = ElementState::Dynamic,
        };

        DepthStateDescriptor depthState{
            .depthTest  = false,
            .depthWrite = false,
        };

        PushConstantDescriptor vertexPushConstant{
            .stage  = GhaShader::Stage::Vertex,
            .offset = 0,
            .size   = sizeof(mat4f),
        };
        PushConstantDescriptor pixelPushConstant{
            .stage  = GhaShader::Stage::Pixel,
            .offset = vertexPushConstant.size,
            .size   = sizeof(vec4f),
        };

        GhaGraphicsPipelineObject::Descriptor pipelineDescriptor{
            .vertexShader         = *ghaFactory->createShaderFromSource({ ui_v, ui_vLength }, shaderIncludes, "UI (vertex)", GhaShader::Stage::Vertex),
            .pixelShader          = *ghaFactory->createShaderFromSource({ widget_p, widget_pLength }, shaderIncludes, "Widget (pixel)", GhaShader::Stage::Pixel),
            .vertexInput          = Vertex::getInputBindingDescriptor(),
            .vertexAttributes     = vertexAttributes,
            .viewportDescriptor   = viewScissorArea,
            .scissorDescriptor    = viewScissorArea,
            .depthState           = depthState,
            .renderPass           = renderPass,
            .descriptorSetLayouts = { descriptorSetLayouts[DescriptorSetSlots::UI] },
            .pushConstants        = { vertexPushConstant, pixelPushConstant },
        };

        widgetPipelineObject = *ghaFactory->createGraphicsPipelineObject(pipelineDescriptor);

        pipelineDescriptor.pixelShader = *ghaFactory->createShaderFromSource({ font_p, font_pLength }, shaderIncludes, "Font (pixel)", GhaShader::Stage::Pixel);

        textPipelineObject = *ghaFactory->createGraphicsPipelineObject(std::move(pipelineDescriptor));
    }

    void ForwardRenderer3D::createRenderTargetFrameBuffers() {
        for(auto &imageView : renderTarget->getImageViews()) {
            frameBuffers.emplace_back(*ghaFactory->createFramebuffer(GhaFramebuffer::Descriptor{
                .renderPass  = renderPass,
                .attachments = { imageView, depthImageView },
                .width       = renderTarget->getSize().x,
                .height      = renderTarget->getSize().y,
            }));
        }
    }

    std::shared_ptr<GhaDescriptorPool> ForwardRenderer3D::createDescriptorPool(std::unordered_map<DescriptorType, uint32_t> const &bindingCount, uint32_t const setCount) {
        std::vector<DescriptorInfo> poolTypes;
        poolTypes.reserve(bindingCount.size());

        for(auto &&[type, count] : bindingCount) {
            poolTypes.emplace_back(DescriptorInfo{
                .type  = type,
                .count = count,
            });
        }

        GhaDescriptorPool::Descriptor poolDescriptor{
            .poolTypes = std::move(poolTypes),
            .flag      = GhaDescriptorPool::Flag::None,
            .maxSets   = setCount,
        };

        return *ghaFactory->createDescriptorPool(std::move(poolDescriptor));
    }
}

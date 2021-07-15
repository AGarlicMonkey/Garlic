#pragma once

#include "Clove/Rendering/RenderGraph/RgId.hpp"

#include <Clove/Graphics/GhaBuffer.hpp>
#include <Clove/Graphics/GhaComputeCommandBuffer.hpp>
#include <Clove/Graphics/GhaDescriptorPool.hpp>
#include <Clove/Graphics/GhaFactory.hpp>
#include <Clove/Graphics/GhaFramebuffer.hpp>
#include <Clove/Graphics/GhaGraphicsCommandBuffer.hpp>
#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Graphics/GhaImageView.hpp>
#include <Clove/Graphics/GhaTransferCommandBuffer.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace garlic::clove {
    /**
     * @brief Stores objects associated with a single frame (images, buffers, etc.) for a RenderGraph
     */
    class RgFrameCache {
        //TYPES
    private:
        using PoolId = uint64_t;

        template<typename T>
        struct ResourcePool {
            std::unordered_map<PoolId, std::vector<std::shared_ptr<T>>> free{};
            std::unordered_map<PoolId, std::vector<std::shared_ptr<T>>> allocated{};

            /**
             * @brief Retrieve an element from the pool. Allocating a new one if necessary.
             * @param id Unique id of an element to retrieve.
             * @param createItem Functor that will be called if a new item needs to be created.
             * @return 
             */
            std::shared_ptr<T> retrieve(PoolId const id, std::function<std::shared_ptr<T>()> createItem) {
                std::shared_ptr<T> item{};

                bool const containsFreeElement{ free.contains(id) && !free.at(id).empty() };
                if(containsFreeElement) {
                    auto &freeVec{ free.at(id) };

                    item = std::move(freeVec.back());
                    freeVec.pop_back();
                } else {
                    item = createItem();
                }
                allocated[id].push_back(item);

                return item;
            }

            /**
             * @brief Resets the pool marking all allocated elements as free.
             */
            void reset() {
                for(auto&&[key, vec] : allocated){
                    free[key].insert(free[key].end(), vec.begin(), vec.end());
                }
                allocated.clear();
            }
        };

        //VARIABLES
    private:
        std::shared_ptr<GhaFactory> ghaFactory{ nullptr };

        ResourcePool<GhaBuffer> bufferPool{};
        ResourcePool<GhaImage> imagePool{};
        ResourcePool<GhaImageView> imageViewPool{};
        ResourcePool<GhaFramebuffer> framebufferPool{};
        ResourcePool<GhaDescriptorPool> descriptorPoolPool{};

        std::shared_ptr<GhaGraphicsCommandBuffer> graphicsCommandBuffer{ nullptr };
        std::shared_ptr<GhaComputeCommandBuffer> computeCommandBuffer{ nullptr };
        std::shared_ptr<GhaTransferCommandBuffer> transferCommandBuffer{ nullptr };

        //FUNCTIONS
    public:
        RgFrameCache() = delete;
        RgFrameCache(std::shared_ptr<GhaFactory> ghaFactory, GhaGraphicsQueue &graphicsQueue, GhaComputeQueue &computeQueue, GhaTransferQueue &transferQueue);

        RgFrameCache(RgFrameCache const &other) = delete;
        RgFrameCache(RgFrameCache &&other) noexcept;

        RgFrameCache &operator=(RgFrameCache const &other) = delete;
        RgFrameCache &operator=(RgFrameCache &&other) noexcept;

        ~RgFrameCache();

        /**
         * @brief Marks all allocate objects as free to use.
         */
        void reset();

        std::shared_ptr<GhaBuffer> allocateBuffer(GhaBuffer::Descriptor descriptor);

        std::shared_ptr<GhaImage> allocateImage(GhaImage::Descriptor descriptor);
        std::shared_ptr<GhaImageView> allocateImageView(GhaImage const *const image, GhaImageView::Descriptor descriptor);

        std::shared_ptr<GhaFramebuffer> allocateFramebuffer(GhaFramebuffer::Descriptor descriptor);

        std::shared_ptr<GhaDescriptorPool> allocateDescriptorPool(GhaDescriptorPool::Descriptor descriptor);

        std::shared_ptr<GhaGraphicsCommandBuffer> getGraphicsCommandBuffer();
        std::shared_ptr<GhaComputeCommandBuffer> getComputeCommandBuffer();
        std::shared_ptr<GhaTransferCommandBuffer> getTransferCommandBuffer();
    };
}
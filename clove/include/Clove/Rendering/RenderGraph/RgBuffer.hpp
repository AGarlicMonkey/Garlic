#pragma once

#include "Clove/Rendering/RenderGraph/RgResource.hpp"

#include <Clove/Graphics/GhaBuffer.hpp>
#include <memory>

namespace garlic::clove {
    class RgFrameCache;
}

namespace garlic::clove {
    /**
     * @brief Tracks the usage and dependencies of a buffer in the RenderGraph.
     * @details Can be used to later create a GhaBuffer when executing the graph
     * or track an exisiting GhaBuffer.
     */
    class RgBuffer : public RgResource {
        //VARIABLES
    private:
        std::shared_ptr<GhaBuffer> ghaBuffer{ nullptr };
        GhaBuffer::Descriptor ghaBufferDescriptor{};
        bool externalBuffer{ false }; /**< Will be true if this was registered with an external buffer. Preventing the descriptor being built. */

        size_t offset{ 0 }; /**< Offset into the GhaBuffer this buffer represents. */
        size_t size{ 0 };   /**< Size of the region in the GhaBuffer this buffer represents. */

        //FUNCTIONS
    public:
        RgBuffer() = delete;
        RgBuffer(RgResourceIdType id, size_t size);
        RgBuffer(RgResourceIdType id, std::shared_ptr<GhaBuffer> ghaBuffer, size_t offset, size_t size);

        RgBuffer(RgBuffer const &other) = delete;
        RgBuffer(RgBuffer &&other) noexcept;

        RgBuffer &operator=(RgBuffer const &other) = delete;
        RgBuffer &operator=(RgBuffer &&other) noexcept;

        ~RgBuffer();

        /**
         * @brief Get the GhaBuffer for this object. Will create one if possible.
         * @param ghaFactory 
         * @return 
         */
        std::shared_ptr<GhaBuffer> getGhaBuffer(RgFrameCache &cache);

        inline bool isExternalBuffer() const;

        inline size_t getBufferOffset() const;
        inline size_t getBufferSize() const;

        void addBufferUsage(GhaBuffer::UsageMode usage);
        /**
         * @brief Allows the CPU to access the buffer's memory.
         * @details This will change the memory type from VideoMemory to SystemMemory.
         */
        void makeCpuAccessable();
    };
}

#include "RgBuffer.inl"
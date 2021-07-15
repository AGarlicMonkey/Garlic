#pragma once

#include "Clove/Rendering/RenderGraph/RgResource.hpp"

#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Graphics/GhaImageView.hpp>
#include <memory>

namespace garlic::clove {
    class RgFrameCache;
}

namespace garlic::clove {
    /**
     * @brief Tracks the usage and dependencies of an image in the RenderGraph.
     * @details Can be used to later create a GhaImage when executing the graph.
     */
    class RgImage : public RgResource {
        //VARIABLES
    private:
        std::shared_ptr<GhaImage> ghaImage{ nullptr }; /**< The GHA image this object views. */
        std::shared_ptr<GhaImageView> ghaImageView{ nullptr }; /**< The GHA image view this object represents. */
        GhaImage::Descriptor ghaImageDescriptor{};
        bool externalImage{ false }; /**< Will be true if this was registered with an external image. Preventing the descriptor being built. */

        //FUNCTIONS
    public:
        RgImage() = delete;
        RgImage(RgResourceIdType id, GhaImage::Type imageType, GhaImage::Format format, vec2ui dimensions);
        RgImage(RgResourceIdType id, std::shared_ptr<GhaImageView> ghaImageView);

        RgImage(RgImage const &other) = delete;
        RgImage(RgImage &&other) noexcept;

        RgImage &operator=(RgImage const &other) = delete;
        RgImage &operator=(RgImage &&other) noexcept;

        ~RgImage();

        /**
         * @brief Get the GhaImageView for this object. Will create any resources required.
         * @param ghaFactory 
         * @return 
         */
        std::shared_ptr<GhaImageView> getGhaImageView(RgFrameCache &cache);

        inline bool isExternalImage() const;

        void addImageUsage(GhaImage::UsageMode usage);
    };
}

#include "RgImage.inl"
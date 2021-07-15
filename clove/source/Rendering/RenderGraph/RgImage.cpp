#include "Clove/Rendering/RenderGraph/RgImage.hpp"

#include "Clove/Rendering/RenderGraph/RgFrameCache.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    namespace {
        GhaImageView::Type getViewType(GhaImage::Type imageType) {
            switch(imageType) {
                case GhaImage::Type::_2D:
                    return GhaImageView::Type::_2D;
                case GhaImage::Type::_3D:
                    return GhaImageView::Type::_3D;
                case GhaImage::Type::Cube:
                    return GhaImageView::Type::Cube;
            }
        }
    }

    RgImage::RgImage(RgResourceIdType id, GhaImage::Type imageType, GhaImage::Format format, vec2ui dimensions)
        : RgResource{ id } {
        ghaImageDescriptor = GhaImage::Descriptor{
            .type        = imageType,
            .usageFlags  = static_cast<GhaImage::UsageMode>(0),//Will be built when executing the graph
            .dimensions  = dimensions,
            .format      = format,
            .sharingMode = SharingMode::Exclusive,//Assume exclusive to beigin with
        };
    }

    RgImage::RgImage(RgResourceIdType id, std::shared_ptr<GhaImageView> ghaImageView)
        : RgResource{ id }
        , ghaImageView{ std::move(ghaImageView) } {
        externalImage = true;
    }

    RgImage::RgImage(RgImage &&other) noexcept = default;

    RgImage &RgImage::operator=(RgImage &&other) noexcept = default;

    RgImage::~RgImage() = default;

    std::shared_ptr<GhaImageView> RgImage::getGhaImageView(RgFrameCache &cache) {
        if(ghaImageView == nullptr) {
            CLOVE_ASSERT(!externalImage, "RgImage is registered as an external image but does not have a valid GhaImageView.");
            ghaImage     = cache.allocateImage(ghaImageDescriptor);
            ghaImageView = cache.allocateImageView(ghaImage.get(), GhaImageView::Descriptor{ .type = getViewType(ghaImageDescriptor.type) });
        }

        return ghaImageView;
    }

    void RgImage::addImageUsage(GhaImage::UsageMode usage) {
        CLOVE_ASSERT(!externalImage, "Cannot change usage mode. RgImage is registered as an external image.");
        ghaImageDescriptor.usageFlags |= usage;
    }
}
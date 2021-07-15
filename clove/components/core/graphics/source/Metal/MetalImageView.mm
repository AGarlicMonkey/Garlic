#include "Clove/Graphics/Metal/MetalImageView.hpp"

#include "Clove/Graphics/GhaImageView.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
	MetalImageView::MetalImageView(GhaImage::Format viewedFormat, vec2ui viewedDimensions, id<MTLTexture> texture)
		: viewedFormat{ viewedFormat }
        , viewedDimensions{ std::move(viewedDimensions) }
		, texture{ [texture retain] } {
	}
	
	MetalImageView::MetalImageView(MetalImageView &&other) noexcept = default;
	
	MetalImageView& MetalImageView::operator=(MetalImageView &&other) noexcept = default;
	
	MetalImageView::~MetalImageView() {
		[texture release];
	}

	GhaImage::Format MetalImageView::getImageFormat() const {
		return viewedFormat;
	}

    vec2ui const &MetalImageView::getImageDimensions() const {
		return viewedDimensions;
	}
	
	id<MTLTexture> MetalImageView::getTexture() const {
		return texture;
	}

	MTLTextureType MetalImageView::convertType(GhaImageView::Type type) {
		switch (type) {
			case GhaImageView::Type::_2D:
				return MTLTextureType2D;
			case GhaImageView::Type::_3D:
				return MTLTextureType3D;
			case GhaImageView::Type::Cube:
				return MTLTextureTypeCube;
			default:
				CLOVE_ASSERT(false, "{0}: Unkown type passed", CLOVE_FUNCTION_NAME_PRETTY);
				return MTLTextureType2D;
		}
	}
}

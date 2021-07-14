#include "Clove/Graphics/Metal/MetalImageView.hpp"

namespace garlic::clove {
	MetalImageView::MetalImageView(id<MTLTexture> texture)
		: texture{ texture }{
	}
	
	MetalImageView::MetalImageView(MetalImageView &&other) noexcept = default;
	
	MetalImageView& MetalImageView::operator=(MetalImageView &&other) noexcept = default;
	
	MetalImageView::~MetalImageView() {
		[texture release];
	}
	
	id<MTLTexture> MetalImageView::getTexture() const {
		return texture;
	}
}

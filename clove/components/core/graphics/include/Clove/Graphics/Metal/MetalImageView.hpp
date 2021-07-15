#pragma once

#include "Clove/Graphics/GhaImageView.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalImageView : public GhaImageView {
		//VARIABLES
	private:
        GhaImage::Format viewedFormat{};
        vec2ui viewedDimensions{};

        //Metal doesn't have an image view type. MTLTextures can however share the same storage space or a subset of that space.
		id<MTLTexture> texture{ nullptr };
		
		//FUNCTIONS
	public:
		MetalImageView() = delete;
        MetalImageView(GhaImage::Format viewedFormat, vec2ui viewedDimensions, id<MTLTexture> texture);

        MetalImageView(MetalImageView const &other) = delete;
		MetalImageView(MetalImageView &&other) noexcept;
		
		MetalImageView& operator=(MetalImageView const &other) = delete;
		MetalImageView& operator=(MetalImageView &&other) noexcept;
		
		~MetalImageView();

        GhaImage::Format getImageFormat() const override;
        vec2ui const &getImageDimensions() const override;

        id<MTLTexture> getTexture() const;

        static MTLTextureType convertType(GhaImageView::Type type);
    };
}

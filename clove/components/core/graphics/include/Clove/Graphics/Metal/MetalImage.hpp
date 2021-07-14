#pragma once

#include "Clove/Graphics/GhaImage.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalImage : public GhaImage {
		//VARIABLES
	private:
		Descriptor descriptor{};
		
		id<MTLTexture> texture{ nullptr };
		
		//FUNCTIONS
	public:
		MetalImage() = delete;
		MetalImage(id<MTLTexture> texture, Descriptor descriptor);
		
		MetalImage(MetalImage const &other) = delete;
		MetalImage(MetalImage &&other) noexcept;
		
		MetalImage& operator=(MetalImage const &other) = delete;
		MetalImage& operator=(MetalImage &&other) noexcept;
		
		~MetalImage();
		
		Descriptor const &getDescriptor() const override;

		std::unique_ptr<GhaImageView> createView(GhaImageView::Descriptor viewDescriptor) const override;
		
		id<MTLTexture> getTexture() const;
		
		static MTLPixelFormat convertFormat(Format format);
		static Format convertFormat(MTLPixelFormat format);
	};
}

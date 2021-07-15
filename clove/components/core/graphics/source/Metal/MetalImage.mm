#include "Clove/Graphics/Metal/MetalImage.hpp"

#include "Clove/Graphics/Metal/MetalImageView.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
	MetalImage::MetalImage(id<MTLTexture> texture, Descriptor descriptor)
		: texture{ [texture retain] }
		, descriptor{ descriptor } {
	}
	
	MetalImage::MetalImage(MetalImage &&other) noexcept = default;
	
	MetalImage& MetalImage::operator=(MetalImage &&other) noexcept = default;
	
	MetalImage::~MetalImage() {
		[texture release];
	}
	
	GhaImage::Descriptor const &MetalImage::getDescriptor() const {
		return descriptor;
	}

	id<MTLTexture> MetalImage::getTexture() const {
		return texture;
	}
	
	MTLPixelFormat MetalImage::convertFormat(Format format) {
		switch(format) {
			case Format::Unkown:
				return MTLPixelFormatInvalid;
			case Format::R8_UNORM:
				return MTLPixelFormatR8Unorm;
			case Format::R8G8B8A8_SRGB:
				return MTLPixelFormatRGBA8Unorm_sRGB;
			case Format::B8G8R8A8_SRGB:
				return MTLPixelFormatBGRA8Unorm_sRGB;
			case Format::B8G8R8A8_UNORM:
				return MTLPixelFormatBGRA8Unorm;
			case Format::D32_SFLOAT:
				return MTLPixelFormatDepth32Float;
			default:
				CLOVE_ASSERT(false, "{0}: Unkown format", CLOVE_FUNCTION_NAME);
				return MTLPixelFormatInvalid;
		}
	}
	
	GhaImage::Format MetalImage::convertFormat(MTLPixelFormat format) {
		switch(format) {
			case MTLPixelFormatInvalid:
				return Format::Unkown;
			case MTLPixelFormatR8Unorm:
				return Format::R8_UNORM;
			case MTLPixelFormatRGBA8Unorm_sRGB:
				return Format::R8G8B8A8_SRGB;
			case MTLPixelFormatBGRA8Unorm_sRGB:
				return Format::B8G8R8A8_SRGB;
			case MTLPixelFormatBGRA8Unorm:
				return Format::B8G8R8A8_UNORM;
			case MTLPixelFormatDepth32Float:
				return Format::D32_SFLOAT;
			default:
				CLOVE_ASSERT(false, "{0}: Unkown format", CLOVE_FUNCTION_NAME);
				return Format::Unkown;
		}
	}
}

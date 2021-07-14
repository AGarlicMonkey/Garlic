#include "Clove/Graphics/Metal/MetalDescriptorSetLayout.hpp"

namespace garlic::clove {
	MetalDescriptorSetLayout::MetalDescriptorSetLayout(Descriptor descriptor, NSArray<MTLArgumentDescriptor *> *vertexDescriptors, NSArray<MTLArgumentDescriptor *> *pixelDescriptors, NSArray<MTLArgumentDescriptor *> *computeDescriptors)
		: descriptor{ std::move(descriptor) }
		, vertexDescriptors{ [vertexDescriptors retain] }
		, pixelDescriptors{ [pixelDescriptors retain] }
		, computeDescriptors{ [computeDescriptors retain] } {
	}
	
	MetalDescriptorSetLayout::MetalDescriptorSetLayout(MetalDescriptorSetLayout &&other) noexcept = default;
	
	MetalDescriptorSetLayout& MetalDescriptorSetLayout::operator=(MetalDescriptorSetLayout &&other) noexcept = default;
	
	MetalDescriptorSetLayout::~MetalDescriptorSetLayout() {
		[vertexDescriptors release];
		[pixelDescriptors release];
		[computeDescriptors release];
	}
	
	GhaDescriptorSetLayout::Descriptor const &MetalDescriptorSetLayout::getDescriptor() const {
		return descriptor;
	}
}

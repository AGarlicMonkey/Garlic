#include "Clove/Graphics/Metal/MetalDescriptorSetLayout.hpp"

namespace garlic::clove {
	MetalDescriptorSetLayout::MetalDescriptorSetLayout(Descriptor descriptor)
		: descriptor{ std::move(descriptor) } {
	}
	
	MetalDescriptorSetLayout::MetalDescriptorSetLayout(MetalDescriptorSetLayout &&other) noexcept = default;
	
	MetalDescriptorSetLayout& MetalDescriptorSetLayout::operator=(MetalDescriptorSetLayout &&other) noexcept = default;
	
	MetalDescriptorSetLayout::~MetalDescriptorSetLayout() = default;
	
	GhaDescriptorSetLayout::Descriptor const &MetalDescriptorSetLayout::getDescriptor() const {
		return descriptor;
	}
}

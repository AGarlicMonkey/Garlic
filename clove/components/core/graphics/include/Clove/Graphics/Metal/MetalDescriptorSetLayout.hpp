#pragma once

#include "Clove/Graphics/GhaDescriptorSetLayout.hpp"

namespace garlic::clove {
	class MetalDescriptorSetLayout : public GhaDescriptorSetLayout {
		//VARIABLES
	private:
		Descriptor descriptor{};
		
		//FUNCTIONS
	public:
		MetalDescriptorSetLayout() = delete;
		MetalDescriptorSetLayout(Descriptor descriptor);
		
		MetalDescriptorSetLayout(MetalDescriptorSetLayout const &other) = delete;
		MetalDescriptorSetLayout(MetalDescriptorSetLayout &&other) noexcept;
		
		MetalDescriptorSetLayout& operator=(MetalDescriptorSetLayout const &other) = delete;
		MetalDescriptorSetLayout& operator=(MetalDescriptorSetLayout &&other) noexcept;
		
		~MetalDescriptorSetLayout();
		
		Descriptor const &getDescriptor() const override;
	};
}

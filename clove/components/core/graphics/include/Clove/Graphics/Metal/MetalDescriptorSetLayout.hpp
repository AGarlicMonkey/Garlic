#pragma once

#include "Clove/Graphics/GhaDescriptorSetLayout.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalDescriptorSetLayout : public GhaDescriptorSetLayout {
		//VARIABLES
	private:
		Descriptor descriptor{};
		
		NSArray<MTLArgumentDescriptor *> *vertexDescriptors{ nullptr };
		NSArray<MTLArgumentDescriptor *> *pixelDescriptors{ nullptr };
		NSArray<MTLArgumentDescriptor *> *computeDescriptors{ nullptr };
		
		//FUNCTIONS
	public:
		MetalDescriptorSetLayout() = delete;
		MetalDescriptorSetLayout(Descriptor descriptor, NSArray<MTLArgumentDescriptor *> *vertexDescriptors, NSArray<MTLArgumentDescriptor *> *pixelDescriptors, NSArray<MTLArgumentDescriptor *> *computeDescriptors);
		
		MetalDescriptorSetLayout(MetalDescriptorSetLayout const &other) = delete;
		MetalDescriptorSetLayout(MetalDescriptorSetLayout &&other) noexcept;
		
		MetalDescriptorSetLayout& operator=(MetalDescriptorSetLayout const &other) = delete;
		MetalDescriptorSetLayout& operator=(MetalDescriptorSetLayout &&other) noexcept;
		
		~MetalDescriptorSetLayout();
		
		Descriptor const &getDescriptor() const override;
		
		inline NSArray<MTLArgumentDescriptor *> *getVertexDescriptors() const;
		inline NSArray<MTLArgumentDescriptor *> *getPixelDescriptors() const;
		inline NSArray<MTLArgumentDescriptor *> *getComputeDescriptors() const;
	};
}

#include "MetalDescriptorSetLayout.inl"

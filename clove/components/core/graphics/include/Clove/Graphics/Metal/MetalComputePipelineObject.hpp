#pragma once

#include "Clove/Graphics/GhaComputePipelineObject.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalComputePipelineObject : public GhaComputePipelineObject {
		//VARIABLES
	private:
		id<MTLComputePipelineState> pipelineState{ nullptr };
		
		//FUNCTIONS
	public:
		MetalComputePipelineObject() = delete;
		MetalComputePipelineObject(id<MTLComputePipelineState> pipelineState);
		
		MetalComputePipelineObject(MetalComputePipelineObject const &other) = delete;
		MetalComputePipelineObject(MetalComputePipelineObject &&other) noexcept;
		
		MetalComputePipelineObject &operator=(MetalComputePipelineObject const &other) = delete;
		MetalComputePipelineObject &operator=(MetalComputePipelineObject &&other) noexcept;
		
		~MetalComputePipelineObject();
		
		inline id<MTLComputePipelineState> getPipelineState() const;
	};
}

#include "Clove/Graphics/Metal/MetalComputePipelineObject.inl"

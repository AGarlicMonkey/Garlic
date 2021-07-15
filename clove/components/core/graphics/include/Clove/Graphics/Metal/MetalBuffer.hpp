#pragma once

#include "Clove/Graphics/GhaBuffer.hpp"

#import <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalBuffer : public GhaBuffer {
		//VARIABLES
	private:
		id<MTLBuffer> buffer{ nullptr };
		
		Descriptor descriptor;
		
		//FUNCTIONS
	public:
		MetalBuffer() = delete;
		MetalBuffer(id<MTLBuffer> buffer, Descriptor descriptor);

		MetalBuffer(MetalBuffer const &other) = delete;
		MetalBuffer(MetalBuffer &&other) noexcept;

		MetalBuffer &operator=(MetalBuffer const &other) = delete;
		MetalBuffer &operator=(MetalBuffer &&other) noexcept;

		~MetalBuffer();

		void write(void const *data, size_t const offset, size_t const size) override;

		void read(void *data, size_t const offset, size_t const size) override;

		id<MTLBuffer> getBuffer() const;
	};
}

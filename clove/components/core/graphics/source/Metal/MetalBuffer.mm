#include "Clove/Graphics/Metal/MetalBuffer.hpp"

#include <Clove/Log/Log.hpp>

namespace garlic::clove {
	MetalBuffer::MetalBuffer(id<MTLBuffer> buffer, Descriptor descriptor)
		: buffer{ [buffer retain] }
		, descriptor{ descriptor }{
	}

	MetalBuffer::MetalBuffer(MetalBuffer &&other) noexcept = default;

	MetalBuffer &MetalBuffer::operator=(MetalBuffer &&other) noexcept = default;

	MetalBuffer::~MetalBuffer() {
		[buffer release];
	}

	void MetalBuffer::write(void const *data, size_t const offset, size_t const size) {
		CLOVE_ASSERT(descriptor.memoryType == MemoryType::SystemMemory, "{0}: Can only write to SystemMemory buffers", CLOVE_FUNCTION_NAME_PRETTY);
		
		uint8_t *cpuAccessibleMemory{ reinterpret_cast<uint8_t *>([buffer contents]) };
		memcpy((cpuAccessibleMemory + offset), data, size);
	}

	void MetalBuffer::read(void *data, size_t const offset, size_t const size) {
		CLOVE_ASSERT(descriptor.memoryType == MemoryType::SystemMemory, "{0}: Can only read from SystemMemory buffers", CLOVE_FUNCTION_NAME_PRETTY);
		
		uint8_t *cpuAccessibleMemory{ reinterpret_cast<uint8_t *>([buffer contents]) };
		memcpy(data, (cpuAccessibleMemory + offset), size);
	}

	id<MTLBuffer> MetalBuffer::getBuffer() const {
		return buffer;
	}
}

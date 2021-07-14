#pragma once

#include <cinttypes>

namespace garlic::clove {
    /**
     * @brief Due to SPIR-V Cross we don't have much control at what index push constants
     * and UBOs etc. start in metal as with MSL the vertex buffer + data for the vertex
     * stage share the same index. So we rebind the vertex buffer's index to an arbitrary
     * number.
     */
    static int32_t constexpr vertexBufferBindingIndex{ 30 };
	
	/**
	 * @brief Push constants in metal work like regular buffer bindings so this
	 * slot is reserved for push constant uploads.
	 */
	static int32_t constexpr pushConstantSlot{ 5 };
}

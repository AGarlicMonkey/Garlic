#include "Clove/Graphics/Metal/MetalFence.hpp"

namespace garlic::clove {
	MetalFence::MetalFence(bool signaled)
		: signaled{ signaled } {
	}
	
	MetalFence::~MetalFence() = default;
	
	void MetalFence::wait() {
		std::unique_lock lock{ signaledMutex };
		conditionVariable.wait(lock, [this](){ return signaled; });
	}
	
	void MetalFence::reset() {
		signaled = false;
	}
	
	void MetalFence::signal() {
		std::scoped_lock lock{ signaledMutex };
		signaled = true;
		conditionVariable.notify_all();
	}
}

#pragma once

#include "Clove/Graphics/GhaFence.hpp"

#include <MetalKit/MetalKit.h>
#include <condition_variable>
#include <mutex>

namespace garlic::clove {
	class MetalFence : public GhaFence {
		//VARIABLES
	private:
		//dispatch_sempahore_t is a counting semaphore. We want to simulate vulkan's fence which acts as a bool so we manually handle it ourselves.
		bool signaled{ false };
		std::mutex signaledMutex{};
		std::condition_variable conditionVariable{};
		
		//FUNCTIONS
	public:
		MetalFence() = delete;
		MetalFence(bool signaled);
		
		MetalFence(MetalFence const &other) = delete;
		MetalFence(MetalFence &&other) noexcept = delete;
		
		MetalFence &operator=(MetalFence const &other) = delete;
		MetalFence &operator=(MetalFence &&other) noexcept = delete;
		
		~MetalFence();
		
		void wait() override;
		void reset() override;
		
		void signal();
	};
}

#pragma once

#include "Clove/Graphics/GhaComputeCommandBuffer.hpp"

#include <MetalKit/MetalKit.h>
#include <vector>
#include <functional>

namespace garlic::clove {
	class MetalComputeCommandBuffer : public GhaComputeCommandBuffer {
		//VARIABLES
	private:
		std::vector<std::function<void(id<MTLComputeCommandEncoder>)>> commands{};
		
		//Validation
		CommandBufferUsage currentUsage{ CommandBufferUsage::Default };
		bool hasBeenUsed{ false }; /**< Will be true if this buffer has been used before being rerecorded. */
		bool allowReuse{ false }; /**< Will be true if this can be reused (recorded to multiple times without beeing freed) */
		bool endRecordingCalled{ true };
		
		//FUNCTIONS
	public:
		MetalComputeCommandBuffer() = delete;
		MetalComputeCommandBuffer(bool allowReuse);
		
		MetalComputeCommandBuffer(MetalComputeCommandBuffer const &other) = delete;
		MetalComputeCommandBuffer(MetalComputeCommandBuffer &&other) noexcept;
		
		MetalComputeCommandBuffer &operator=(MetalComputeCommandBuffer const &other) = delete;
		MetalComputeCommandBuffer &operator=(MetalComputeCommandBuffer &&other) noexcept;
		
		~MetalComputeCommandBuffer();

		void beginRecording(CommandBufferUsage usageFlag) override;
		void endRecording() override;

		void bindPipelineObject(GhaComputePipelineObject &pipelineObject) override;

		void bindDescriptorSet(GhaDescriptorSet &descriptorSet, uint32_t const setNum) override;
		void pushConstant(size_t const offset, size_t const size, void const *data) override;

		void disptach(vec3ui groupCount) override;

		void bufferMemoryBarrier(GhaBuffer &buffer, BufferMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) override;
		void imageMemoryBarrier(GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) override;
		
		inline std::vector<std::function<void(id<MTLComputeCommandEncoder>)>> const &getCommands() const;
		
		inline CommandBufferUsage getCommandBufferUsage() const;
		inline void markAsUsed();
		inline bool bufferHasBeenUsed() const;
	};
}

#include "MetalComputeCommandBuffer.inl"

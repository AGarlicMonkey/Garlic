namespace garlic::clove {
	std::vector<std::function<void(id<MTLComputeCommandEncoder>)>> const &MetalComputeCommandBuffer::getCommands() const {
		return commands;
	}
	
	CommandBufferUsage MetalComputeCommandBuffer::getCommandBufferUsage() const {
		return currentUsage;
	}
	
	void MetalComputeCommandBuffer::markAsUsed() {
		hasBeenUsed = true;
	}
	
	bool MetalComputeCommandBuffer::bufferHasBeenUsed() const {
		return hasBeenUsed;
	}
}

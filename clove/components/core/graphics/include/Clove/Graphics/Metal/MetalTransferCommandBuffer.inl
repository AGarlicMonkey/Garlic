namespace garlic::clove {
	std::vector<std::function<void(id<MTLBlitCommandEncoder>)>> const &MetalTransferCommandBuffer::getCommands() const {
		return commands;
	}
	
	CommandBufferUsage MetalTransferCommandBuffer::getCommandBufferUsage() const {
		return currentUsage;
	}
	
	void MetalTransferCommandBuffer::markAsUsed() {
		hasBeenUsed = true;
	}
	
	bool MetalTransferCommandBuffer::bufferHasBeenUsed() const {
		return hasBeenUsed;
	}
}

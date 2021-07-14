namespace garlic::clove {
	std::vector<MetalGraphicsCommandBuffer::RenderPass> const &MetalGraphicsCommandBuffer::getEncodedRenderPasses() const {
		return passes;
	}
	
	CommandBufferUsage MetalGraphicsCommandBuffer::getCommandBufferUsage() const {
		return currentUsage;
	}
	
	void MetalGraphicsCommandBuffer::markAsUsed() {
		hasBeenUsed = true;
	}
	
	bool MetalGraphicsCommandBuffer::bufferHasBeenUsed() const {
		return hasBeenUsed;
	}
}

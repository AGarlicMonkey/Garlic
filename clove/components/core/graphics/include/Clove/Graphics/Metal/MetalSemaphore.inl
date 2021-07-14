namespace garlic::clove {
	id<MTLFence> MetalSemaphore::getFence() const {
		return fence;
	}
}

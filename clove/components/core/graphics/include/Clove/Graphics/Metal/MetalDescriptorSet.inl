namespace garlic::clove {
	id<MTLBuffer> MetalDescriptorSet::getVertexBuffer() const {
		return vertexEncoderBuffer;
	}
	
	id<MTLBuffer> MetalDescriptorSet::getPixelBuffer() const {
		return pixelEncoderBuffer;
	}
	
	id<MTLBuffer> MetalDescriptorSet::getComputeBuffer() const {
		return computeEncoderBuffer;
	}
}

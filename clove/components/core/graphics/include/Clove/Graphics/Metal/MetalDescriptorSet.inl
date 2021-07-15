namespace garlic::clove {
	id<MTLArgumentEncoder> MetalDescriptorSet::getVertexEncoder() const {
		return vertexEncoder;
	}
	
	id<MTLArgumentEncoder> MetalDescriptorSet::getPixelEncoder() const {
		return pixelEncoder;
	}
}

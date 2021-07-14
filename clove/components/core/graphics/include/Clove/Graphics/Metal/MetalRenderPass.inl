namespace garlic::clove{
	MTLRenderPipelineColorAttachmentDescriptorArray* MetalRenderPass::getColourAttachments() const {
		return colourAttachments;
	}
	
	MTLPixelFormat MetalRenderPass::getDepthPixelFormat() const {
		return depthPixelFormat;
	}
	
	GhaRenderPass::Descriptor const &MetalRenderPass::getDescriptor() const {
		return descriptor;
	}
}

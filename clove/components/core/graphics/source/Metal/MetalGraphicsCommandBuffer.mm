#include "Clove/Graphics/Metal/MetalGraphicsCommandBuffer.hpp"

#include "Clove/Graphics/Metal/MetalFramebuffer.hpp"
#include "Clove/Graphics/Metal/MetalGlobals.hpp"
#include "Clove/Graphics/Metal/MetalGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Metal/MetalBuffer.hpp"
#include "Clove/Graphics/Metal/MetalRenderPass.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorSet.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorSetLayout.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	namespace {
		MTLIndexType getIndexType(IndexType garlicType) {
			switch(garlicType) {
				case IndexType::Uint16:
					return MTLIndexTypeUInt16;
				default:
					CLOVE_ASSERT(false, "{0}: Unkown index type", CLOVE_FUNCTION_NAME);
					return MTLIndexTypeUInt16;
			}
		}
	}
	
	MetalGraphicsCommandBuffer::MetalGraphicsCommandBuffer() = default;
	
	MetalGraphicsCommandBuffer::MetalGraphicsCommandBuffer(MetalGraphicsCommandBuffer &&other) noexcept = default;
	
	MetalGraphicsCommandBuffer& MetalGraphicsCommandBuffer::operator=(MetalGraphicsCommandBuffer &&other) noexcept = default;
	
	MetalGraphicsCommandBuffer::~MetalGraphicsCommandBuffer() = default;
	
	void MetalGraphicsCommandBuffer::beginRecording(CommandBufferUsage usageFlag) {
		passes.clear();
	}
	
	void MetalGraphicsCommandBuffer::endRecording() {
		//no op
	}

	void MetalGraphicsCommandBuffer::beginRenderPass(GhaRenderPass &renderPass, GhaFramebuffer &frameBuffer, RenderArea const &renderArea, std::span<ClearValue> clearValues) {
		std::vector<ClearValue> clearValuesCopy{ clearValues.begin(), clearValues.end() };
		
		currentPass = &passes.emplace_back(RenderPass{});
		currentPass->begin = [this, renderPass = &renderPass, frameBuffer = &frameBuffer, renderArea, clearValuesCopy](id<MTLCommandBuffer> commandBuffer) mutable {
			GhaRenderPass::Descriptor const &renderPassDescriptor{ polyCast<MetalRenderPass>(renderPass)->getDescriptor() };
			
			//Modify the attachments to have the correct clear values.
			MTLRenderPassDescriptor *frameBufferDescriptor{ polyCast<MetalFramebuffer>(frameBuffer)->getRenderPassDescriptor() };
			for(size_t i{ 0 }; i < renderPassDescriptor.colourAttachments.size(); ++i) {
				ColourValue const colour{ std::get<ColourValue>(clearValuesCopy[i]) };
				frameBufferDescriptor.colorAttachments[i].clearColor = MTLClearColor{ colour.r, colour.g, colour.b, colour.a };
			}
			DepthStencilValue const depthStencilValue{ std::get<DepthStencilValue>(clearValuesCopy.back()) };
			frameBufferDescriptor.depthAttachment.clearDepth = depthStencilValue.depth;
			
			//TODO: RenderArea
			
			return [commandBuffer renderCommandEncoderWithDescriptor:frameBufferDescriptor];
		};
	}
	
	void MetalGraphicsCommandBuffer::endRenderPass() {
		currentPass->commands.emplace_back([](id<MTLRenderCommandEncoder> encoder){
			[encoder endEncoding];
		});
	}

	void MetalGraphicsCommandBuffer::setViewport(vec2i position, vec2ui size) {
		MTLViewport viewport {
			.originX = static_cast<double>(position.x),
			.originY = static_cast<double>(position.y),
			.width   = static_cast<double>(size.x),
			.height  = static_cast<double>(size.y),
			.znear   = 0.0f,
			.zfar    = 1.0f,
		};
		
		currentPass->commands.emplace_back([viewport](id<MTLRenderCommandEncoder> encoder){
			[encoder setViewport:viewport];
		});
	}
	
	void MetalGraphicsCommandBuffer::setScissor(vec2i position, vec2ui size) {
		MTLScissorRect scissorRect {
			.x      = static_cast<NSUInteger>(position.x),
			.y      = static_cast<NSUInteger>(position.y),
			.width  = size.x,
			.height = size.y,
		};
		
		currentPass->commands.emplace_back([scissorRect](id<MTLRenderCommandEncoder> encoder){
			[encoder setScissorRect:scissorRect];
		});
	}

	void MetalGraphicsCommandBuffer::bindPipelineObject(GhaGraphicsPipelineObject &pipelineObject) {
		currentPass->commands.emplace_back([pipelineObject = &pipelineObject](id<MTLRenderCommandEncoder> encoder){
			[encoder setRenderPipelineState:polyCast<MetalGraphicsPipelineObject const>(pipelineObject)->getPipeline()];
		});
	}
	
	void MetalGraphicsCommandBuffer::bindVertexBuffer(GhaBuffer &vertexBuffer, size_t const offset) {
		currentPass->commands.emplace_back([vertexBuffer = &vertexBuffer, offset](id<MTLRenderCommandEncoder> encoder){
			[encoder setVertexBuffer:polyCast<MetalBuffer>(vertexBuffer)->getBuffer()
							  offset:offset
							 atIndex:vertexBufferBindingIndex];
		});
	}
	
	void MetalGraphicsCommandBuffer::bindIndexBuffer(GhaBuffer &indexBuffer, size_t const offset, IndexType indexType) {
		if(cachedIndexBuffer.buffer != nullptr) {
			[cachedIndexBuffer.buffer release];
		}
		
		cachedIndexBuffer.buffer = [polyCast<MetalBuffer>(&indexBuffer)->getBuffer() retain];
		cachedIndexBuffer.indexType = getIndexType(indexType);
		cachedIndexBuffer.offset = offset;
	}
	
	void MetalGraphicsCommandBuffer::bindDescriptorSet(GhaDescriptorSet &descriptorSet, uint32_t const setNum) {
		currentPass->commands.emplace_back([descriptorSet = &descriptorSet, setNum](id<MTLRenderCommandEncoder> encoder){
			auto *metalDescriptorSet{ polyCast<MetalDescriptorSet>(descriptorSet) };
			
			//TODO
		});
	}
	
	void MetalGraphicsCommandBuffer::pushConstant(GhaShader::Stage const stage, size_t const offset, size_t const size, void const *data) {
		uint8_t *cachedData{ reinterpret_cast<uint8_t*>(malloc(size)) };
		memcpy(cachedData, data, size);
		
		currentPass->commands.emplace_back([stage, data = cachedData, size](id<MTLRenderCommandEncoder> encoder){
			switch(stage) {
				case GhaShader::Stage::Vertex:
					[encoder setVertexBytes:data
									 length:size
									atIndex:pushConstantSlot];
					break;
				case GhaShader::Stage::Pixel:
					[encoder setFragmentBytes:data
									   length:size
									  atIndex:pushConstantSlot];
					break;
				default:
					CLOVE_ASSERT(false, "{0}: Unknown shader stage provided", CLOVE_FUNCTION_NAME_PRETTY);
					break;
			}
			
			delete data;
		});
	}

	void MetalGraphicsCommandBuffer::drawIndexed(size_t const indexCount) {
		currentPass->commands.emplace_back([cachedIndexBuffer = cachedIndexBuffer, indexCount](id<MTLRenderCommandEncoder> encoder){
			[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
								indexCount:indexCount
								 indexType:cachedIndexBuffer.indexType
							   indexBuffer:cachedIndexBuffer.buffer
						 indexBufferOffset:cachedIndexBuffer.offset];
		});
	}

	void MetalGraphicsCommandBuffer::bufferMemoryBarrier(GhaBuffer &buffer, BufferMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
		//TODO
	}
	
	void MetalGraphicsCommandBuffer::imageMemoryBarrier(GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
		//TODO
	}
	
	void MetalGraphicsCommandBuffer::executeCommands(id<MTLCommandBuffer> commandBuffer) {
		for(auto &pass : passes) {
			id<MTLRenderCommandEncoder> encoder{ pass.begin(commandBuffer) };
			for(auto &command : pass.commands) {
				command(encoder);
			}
		}
		
		[commandBuffer commit];
	}
}

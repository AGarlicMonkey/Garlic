#include "Clove/Graphics/Metal/MetalGraphicsCommandBuffer.hpp"

#include "Clove/Graphics/Metal/MetalFramebuffer.hpp"
#include "Clove/Graphics/Metal/MetalGlobals.hpp"
#include "Clove/Graphics/Metal/MetalGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Metal/MetalBuffer.hpp"
#include "Clove/Graphics/Metal/MetalImage.hpp"
#include "Clove/Graphics/Metal/MetalRenderPass.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorSet.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorSetLayout.hpp"
#include "Clove/Graphics/Metal/MetalPipelineObject.hpp"

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
	
	MetalGraphicsCommandBuffer::MetalGraphicsCommandBuffer(bool allowReuse)
		: allowReuse{ allowReuse } {
	}
	
	MetalGraphicsCommandBuffer::MetalGraphicsCommandBuffer(MetalGraphicsCommandBuffer &&other) noexcept = default;
	
	MetalGraphicsCommandBuffer& MetalGraphicsCommandBuffer::operator=(MetalGraphicsCommandBuffer &&other) noexcept = default;
	
	MetalGraphicsCommandBuffer::~MetalGraphicsCommandBuffer() = default;
	
	void MetalGraphicsCommandBuffer::beginRecording(CommandBufferUsage usageFlag) {
		if(!allowReuse && hasBeenUsed) {
			CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Command buffer re-recorded to. Command buffers cannot only be recorded to more than once unless the owning queue has been created with QueueFlags::ReuseBuffers set.");
		}
		
		if(!endRecordingCalled) {
			CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "beginRecording called before endRecording. Command buffer recording must be finished be starting again.");
		}
		endRecordingCalled = false;
		
		currentUsage = usageFlag;
		hasBeenUsed = false;
		passes.clear();
	}
	
	void MetalGraphicsCommandBuffer::endRecording() {
		endRecordingCalled = true;
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
		
		//Set the winding order as the first command. In vulkan this is done as part of the rasteriser
		currentPass->commands.emplace_back([](id<MTLRenderCommandEncoder> encoder) {
			MTLWinding constexpr winding{ MTLWindingClockwise };
			[encoder setFrontFacingWinding:winding];
		});
	}
	
	void MetalGraphicsCommandBuffer::endRenderPass() {
		currentPass = nullptr;
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
			
			if(metalDescriptorSet->getVertexBuffer() != nullptr) {
				[encoder setVertexBuffer:metalDescriptorSet->getVertexBuffer()
								offset:0
								atIndex:setNum];
			}
			if(metalDescriptorSet->getPixelBuffer() != nullptr) {
				[encoder setFragmentBuffer:metalDescriptorSet->getPixelBuffer()
									offset:0
								   atIndex:setNum];
			}
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
		if(currentPass != nullptr) {
			currentPass->commands.emplace_back([buffer = &buffer, sourceStage, destinationStage](id<MTLRenderCommandEncoder> encoder) {
				id<MTLBuffer> mtlBuffer{ polyCast<MetalBuffer>(buffer)->getBuffer() };
				[encoder memoryBarrierWithResources:&mtlBuffer
											  count:1
										afterStages:convertStage(sourceStage)
									   beforeStages:convertStage(destinationStage)];
			});
		}
	}
	
	void MetalGraphicsCommandBuffer::imageMemoryBarrier(GhaImage &image, ImageMemoryBarrierInfo const &barrierInfo, PipelineStage sourceStage, PipelineStage destinationStage) {
		if(currentPass != nullptr) {
			currentPass->commands.emplace_back([image = &image, sourceStage, destinationStage](id<MTLRenderCommandEncoder> encoder) {
				id<MTLTexture> mtlTexture{ polyCast<MetalImage>(image)->getTexture() };
				[encoder memoryBarrierWithResources:&mtlTexture
											  count:1
										afterStages:convertStage(sourceStage)
									   beforeStages:convertStage(destinationStage)];
			});
		} 
	}
}

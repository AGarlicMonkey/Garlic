#include "Clove/Graphics/Metal/MetalFactory.hpp"

#include "Clove/Graphics/ShaderCompiler.hpp"
#include "Clove/Graphics/Metal/MetalBuffer.hpp"
#include "Clove/Graphics/Metal/MetalComputePipelineObject.hpp"
#include "Clove/Graphics/Metal/MetalComputeQueue.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorPool.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorSetLayout.hpp"
#include "Clove/Graphics/Metal/MetalFence.hpp"
#include "Clove/Graphics/Metal/MetalFramebuffer.hpp"
#include "Clove/Graphics/Metal/MetalGlobals.hpp"
#include "Clove/Graphics/Metal/MetalGraphicsPipelineObject.hpp"
#include "Clove/Graphics/Metal/MetalGraphicsQueue.hpp"
#include "Clove/Graphics/Metal/MetalImage.hpp"
#include "Clove/Graphics/Metal/MetalImageView.hpp"
#include "Clove/Graphics/Metal/MetalPresentQueue.hpp"
#include "Clove/Graphics/Metal/MetalRenderPass.hpp"
#include "Clove/Graphics/Metal/MetalSampler.hpp"
#include "Clove/Graphics/Metal/MetalSemaphore.hpp"
#include "Clove/Graphics/Metal/MetalShader.hpp"
#include "Clove/Graphics/Metal/MetalSwapchain.hpp"
#include "Clove/Graphics/Metal/MetalTransferQueue.hpp"
#include "Clove/Graphics/Metal/MetalView.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	namespace {
		MTLVertexFormat convertAttributeFormat(VertexAttributeFormat garlicFormat) {
			switch(garlicFormat) {
				case VertexAttributeFormat::R32_SFLOAT:
					return MTLVertexFormatFloat;
				case VertexAttributeFormat::R32G32_SFLOAT:
					return MTLVertexFormatFloat2;
				case VertexAttributeFormat::R32G32B32_SFLOAT:
					return MTLVertexFormatFloat3;
				case VertexAttributeFormat::R32G32B32A32_SFLOAT:
					return MTLVertexFormatFloat4;
				case VertexAttributeFormat::R32G32B32A32_SINT:
					return MTLVertexFormatInt4;
				default:
					CLOVE_ASSERT(false, "{0}: Unkown format passed", CLOVE_FUNCTION_NAME_PRETTY);
					return MTLVertexFormatFloat;
			}
		}
		
		MTLTextureType convertImageType(MetalImage::Type type, uint32_t arrayCount) {
			switch (type) {
				case MetalImage::Type::_2D:
					if(arrayCount > 1) {
						return MTLTextureType2DArray;
					} else {
						return MTLTextureType2D;
					}
				case MetalImage::Type::_3D:
					return MTLTextureType3D;
				case MetalImage::Type::Cube:
					if(arrayCount > 1) {
						return MTLTextureTypeCubeArray;
					} else {
						return MTLTextureTypeCube;
					}
				default:
					CLOVE_ASSERT(false, "{0}: Unkown type passed", CLOVE_FUNCTION_NAME_PRETTY);
					return MTLTextureType2D;
			}
		}
		
		MTLTextureUsage getUsageFlags(GhaImage::UsageMode flags) {
			MTLTextureUsage mtlFlags{ MTLTextureUsageUnknown };
			
			if((flags & GhaImage::UsageMode::TransferSource) != 0) {
				mtlFlags |= MTLTextureUsageUnknown;
			}
			if((flags & GhaImage::UsageMode::TransferDestination) != 0) {
				mtlFlags |= MTLTextureUsageUnknown;
			}
			if((flags & GhaImage::UsageMode::Sampled) != 0) {
				mtlFlags |= MTLTextureUsageShaderRead;
			}
			if((flags & GhaImage::UsageMode::ColourAttachment) != 0) {
				mtlFlags |= MTLTextureUsageRenderTarget;
			}
			if((flags & GhaImage::UsageMode::DepthStencilAttachment) != 0) {
				mtlFlags |= MTLTextureUsageRenderTarget;
			}
			
			return mtlFlags;
		}
		
		MTLLoadAction convertLoadOp(LoadOperation garlicOperation) {
			switch(garlicOperation) {
				case LoadOperation::Load:
					return MTLLoadActionLoad;
				case LoadOperation::Clear:
					return MTLLoadActionClear;
				case LoadOperation::DontCare:
					return MTLLoadActionDontCare;
				default:
					CLOVE_ASSERT(false, "{0}: Unkown operation", CLOVE_FUNCTION_NAME);
					return MTLLoadActionDontCare;
			}
		}

		MTLStoreAction convertStoreOp(StoreOperation garlicOperation) {
			switch(garlicOperation) {
				case StoreOperation::Store:
					return MTLStoreActionStore;
				case StoreOperation::DontCare:
					return MTLStoreActionDontCare;
				default:
					CLOVE_ASSERT(false, "{0}: Unkown operation", CLOVE_FUNCTION_NAME);
					return MTLStoreActionUnknown;
			}
		}
		
		MTLSamplerMinMagFilter convertMinMagFilter(GhaSampler::Filter filter) {
			switch (filter) {
				case GhaSampler::Filter::Nearest:
					return MTLSamplerMinMagFilterNearest;
				case GhaSampler::Filter::Linear:
					return MTLSamplerMinMagFilterLinear;
				default:
					CLOVE_ASSERT(false, "{0}: Unkown filter", CLOVE_FUNCTION_NAME);
					return MTLSamplerMinMagFilterNearest;
			}
		}
		
		MTLSamplerAddressMode convertAddressMode(GhaSampler::AddressMode addressMode) {
			switch (addressMode) {
				case GhaSampler::AddressMode::Repeat:
					return MTLSamplerAddressModeRepeat;
				case GhaSampler::AddressMode::MirroredRepeat:
					return MTLSamplerAddressModeMirrorRepeat;
				case GhaSampler::AddressMode::ClampToEdge:
					return MTLSamplerAddressModeClampToEdge;
				case GhaSampler::AddressMode::ClampToBorder:
					return MTLSamplerAddressModeClampToBorderColor;
				default:
					CLOVE_ASSERT(false, "{0}: Unkown address mode", CLOVE_FUNCTION_NAME);
					return MTLSamplerAddressModeRepeat;
			}
		}
	}
	
	MetalFactory::MetalFactory(id<MTLDevice> device, MetalView *view)
		: device{ [device retain] }
		, view{ [view retain] } {
		graphicsPresentCommandQueue = [this->device newCommandQueue];
		transferCommandQueue = [this->device newCommandQueue];
		computeCommandQueue = [this->device newCommandQueue];
	}
	
	MetalFactory::MetalFactory(MetalFactory &&other) noexcept = default;

	MetalFactory &MetalFactory::operator=(MetalFactory &&other) noexcept = default;

	MetalFactory::~MetalFactory() {
		[device release];
		[view release];
		[graphicsPresentCommandQueue release];
		[transferCommandQueue release];
		[computeCommandQueue release];
	}

	Expected<std::unique_ptr<GhaGraphicsQueue>, std::runtime_error> MetalFactory::createGraphicsQueue(CommandQueueDescriptor descriptor) {
		return std::unique_ptr<GhaGraphicsQueue>{ std::make_unique<MetalGraphicsQueue>(std::move(descriptor), graphicsPresentCommandQueue) };
	}
	
	Expected<std::unique_ptr<GhaPresentQueue>, std::runtime_error> MetalFactory::createPresentQueue() {
		return std::unique_ptr<GhaPresentQueue>{ std::make_unique<MetalPresentQueue>(graphicsPresentCommandQueue, view) };
	}
	
	Expected<std::unique_ptr<GhaTransferQueue>, std::runtime_error> MetalFactory::createTransferQueue(CommandQueueDescriptor descriptor) {
		return std::unique_ptr<GhaTransferQueue>{ std::make_unique<MetalTransferQueue>(std::move(descriptor), transferCommandQueue) };
	}
	
	Expected<std::unique_ptr<GhaComputeQueue>, std::runtime_error> MetalFactory::createComputeQueue(CommandQueueDescriptor descriptor) {
		return std::unique_ptr<GhaComputeQueue>{ std::make_unique<MetalComputeQueue>(std::move(descriptor), computeCommandQueue) };
	}

	Expected<std::unique_ptr<GhaSwapchain>, std::runtime_error> MetalFactory::createSwapChain(GhaSwapchain::Descriptor descriptor) {
		std::vector<std::shared_ptr<GhaImage>> swapchainImages{};
		GhaImage::Format const drawableFormat{ MetalImage::convertFormat([view.metalLayer pixelFormat]) };
		
		GhaImage::Descriptor const imageDescriptor{
			.type 		 = GhaImage::Type::_2D,
			.usageFlags  = GhaImage::UsageMode::ColourAttachment | GhaImage::UsageMode::TransferSource,
			.dimensions  = descriptor.extent,
			.format 	 = drawableFormat,
			.sharingMode = SharingMode::Concurrent,
		};
		
		//Creating 3 back buffers for now. Will need to synchronise this number across APIs.
		size_t constexpr swapchainImageCount{ 3 };
		
		swapchainImages.reserve(swapchainImageCount);
		for(size_t i{ 0 }; i < swapchainImageCount; ++i) {
			Expected<std::unique_ptr<GhaImage>, std::runtime_error> imageResult{ createImage(imageDescriptor) };
			if(imageResult.hasValue()) {
				swapchainImages.emplace_back(std::move(imageResult.getValue()));
			} else {
				return Unexpected{ std::move(imageResult.getError()) };
			}
		}
		
		return std::unique_ptr<GhaSwapchain>{ std::make_unique<MetalSwapchain>(std::move(swapchainImages), drawableFormat, descriptor.extent) };
	}

	Expected<std::unique_ptr<GhaShader>, std::runtime_error> MetalFactory::createShaderFromFile(std::filesystem::path const &file, GhaShader::Stage shaderStage) {
		Expected<std::vector<uint32_t>, std::runtime_error> compilationResult{ ShaderCompiler::compileFromFile(file, shaderStage) };
		if(compilationResult.hasValue()) {
			Expected<std::string, std::runtime_error> msl{ ShaderCompiler::spirvToMSL(compilationResult.getValue()) };
			if(msl.hasValue()) {
				return createShaderObject(msl.getValue());
			} else {
				return Unexpected{ std::move(msl.getError()) };
			}
		} else {
			return Unexpected{ std::move(compilationResult.getError()) };
		}
	}
	
	Expected<std::unique_ptr<GhaShader>, std::runtime_error> MetalFactory::createShaderFromSource(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage) {
		Expected<std::vector<uint32_t>, std::runtime_error> compilationResult{ ShaderCompiler::compileFromSource(source, std::move(includeSources), shaderName, shaderStage) };
		if(compilationResult.hasValue()) {
			Expected<std::string, std::runtime_error> msl{ ShaderCompiler::spirvToMSL(compilationResult.getValue()) };
			if(msl.hasValue()) {
				return createShaderObject(msl.getValue());
			} else {
				return Unexpected{ std::move(msl.getError()) };
			}
		} else {
			return Unexpected{ std::move(compilationResult.getError()) };
		}
	}

	Expected<std::unique_ptr<GhaRenderPass>, std::runtime_error> MetalFactory::createRenderPass(GhaRenderPass::Descriptor descriptor) {
		MTLRenderPipelineColorAttachmentDescriptorArray* colourAttachments{ [[MTLRenderPipelineColorAttachmentDescriptorArray alloc] init] };
		for(size_t i{ 0 }; i < descriptor.colourAttachments.size(); ++i){
			colourAttachments[i].pixelFormat = MetalImage::convertFormat(descriptor.colourAttachments[i].format);
		}
		
		MTLPixelFormat depthPixelFormat{ MetalImage::convertFormat(descriptor.depthAttachment.format) };
		
		auto result{ std::unique_ptr<GhaRenderPass>{ std::make_unique<MetalRenderPass>(colourAttachments, depthPixelFormat, std::move(descriptor)) }};
		
		[colourAttachments release];
		
		return result;
	}
	
	Expected<std::unique_ptr<GhaDescriptorSetLayout>, std::runtime_error> MetalFactory::createDescriptorSetLayout(GhaDescriptorSetLayout::Descriptor descriptor) {
		@autoreleasepool {
			NSMutableArray<MTLArgumentDescriptor *> *vertexDescriptors{ [[NSMutableArray<MTLArgumentDescriptor *> alloc] init] };
			NSMutableArray<MTLArgumentDescriptor *> *pixelDescriptors{ [[NSMutableArray<MTLArgumentDescriptor *> alloc] init] };
			NSMutableArray<MTLArgumentDescriptor *> *computeDescriptors{ [[NSMutableArray<MTLArgumentDescriptor *> alloc] init] };
			
			for(auto const &binding : descriptor.bindings) {
				MTLArgumentDescriptor *bindingDescriptor{ [[MTLArgumentDescriptor alloc] init]};
				[bindingDescriptor setIndex:binding.binding];
				switch(binding.type) {
					case DescriptorType::SampledImage:
						[bindingDescriptor setDataType:MTLDataTypeTexture];
						break;
					case DescriptorType::Sampler:
						[bindingDescriptor setDataType:MTLDataTypeSampler];
						break;
					default:
						[bindingDescriptor setDataType:MTLDataTypePointer];
						break;
				}
				[bindingDescriptor setArrayLength:binding.arraySize];
				
				if((binding.stage & GhaShader::Stage::Vertex) != 0){
					[vertexDescriptors addObject:bindingDescriptor];
				}
				if((binding.stage & GhaShader::Stage::Pixel) != 0){
					[pixelDescriptors addObject:bindingDescriptor];
				}
				if((binding.stage & GhaShader::Stage::Compute) != 0){
					[computeDescriptors addObject:bindingDescriptor];
				}
			}
		
			return std::unique_ptr<GhaDescriptorSetLayout>{ std::make_unique<MetalDescriptorSetLayout>(std::move(descriptor), vertexDescriptors, pixelDescriptors, computeDescriptors) };
		}
	}

	Expected<std::unique_ptr<GhaGraphicsPipelineObject>, std::runtime_error> MetalFactory::createGraphicsPipelineObject(GhaGraphicsPipelineObject::Descriptor descriptor) {
		@autoreleasepool {
			//Vertex shader
			id<MTLFunction> vertexFunction{ polyCast<MetalShader>(descriptor.vertexShader.get())->getFunction() };
		
			//Pixel shader
			id<MTLFunction> fragmentFunction{ polyCast<MetalShader>(descriptor.pixelShader.get())->getFunction() };
		
			//Vertex input
			MTLVertexDescriptor *vertexDescriptor{ [[[MTLVertexDescriptor alloc] init] autorelease]};
			vertexDescriptor.layouts[vertexBufferBindingIndex].stride = descriptor.vertexInput.stride;
			for(size_t i{ 0 }; i < descriptor.vertexAttributes.size(); ++i){
				auto const &attribute{ descriptor.vertexAttributes[i] };
				vertexDescriptor.attributes[i].bufferIndex = vertexBufferBindingIndex;
				vertexDescriptor.attributes[i].format = convertAttributeFormat(attribute.format);
				vertexDescriptor.attributes[i].offset = attribute.offset;
			}
		
			//Input assembly
			MTLPrimitiveTopologyClass const topology{ MTLPrimitiveTopologyClassTriangle };

			//Depth / Stencil
			MTLDepthStencilDescriptor *depthStencil{ [[[MTLDepthStencilDescriptor alloc] init] autorelease] };
			depthStencil.depthWriteEnabled = static_cast<BOOL>(descriptor.depthState.depthWrite);
			if(descriptor.depthState.depthTest){
				depthStencil.depthCompareFunction = MTLCompareFunctionLess;
			}else{
				depthStencil.depthCompareFunction = MTLCompareFunctionAlways;
			}
		
			id<MTLDepthStencilState> depthStencilState{ [[device newDepthStencilStateWithDescriptor:depthStencil] autorelease] };
		
			//Render pass
			MetalRenderPass const *const renderPass{ polyCast<MetalRenderPass>(descriptor.renderPass.get()) };
			if(renderPass == nullptr) {
				return Unexpected{ std::runtime_error{ "Failed to create GraphicsPipelineObject. RenderPass is nullptr" } };
			}
			
			size_t const colourAttachmentCount{ renderPass->getDescriptor().colourAttachments.size() };
			MTLRenderPipelineColorAttachmentDescriptorArray* colourAttachments{ renderPass->getColourAttachments() };
			MTLPixelFormat const depthPixelFormat{ renderPass->getDepthPixelFormat() };
		
			MTLRenderPipelineDescriptor *pipelineDesc{ [[[MTLRenderPipelineDescriptor alloc] init] autorelease] };
			pipelineDesc.vertexFunction = vertexFunction;
			pipelineDesc.fragmentFunction = fragmentFunction;
			pipelineDesc.vertexDescriptor = vertexDescriptor;
			pipelineDesc.inputPrimitiveTopology = topology;
			for(size_t i{ 0 }; i < colourAttachmentCount; ++i){
				pipelineDesc.colorAttachments[i] = colourAttachments[i];
			
				//Blending
				pipelineDesc.colorAttachments[i].blendingEnabled = static_cast<BOOL>(descriptor.enableBlending);
				pipelineDesc.colorAttachments[i].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
				pipelineDesc.colorAttachments[i].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
				pipelineDesc.colorAttachments[i].rgbBlendOperation = MTLBlendOperationAdd;
				pipelineDesc.colorAttachments[i].sourceAlphaBlendFactor = MTLBlendFactorOne;
				pipelineDesc.colorAttachments[i].destinationAlphaBlendFactor = MTLBlendFactorZero;
				pipelineDesc.colorAttachments[i].alphaBlendOperation = MTLBlendOperationAdd;
			}
			pipelineDesc.depthAttachmentPixelFormat = depthPixelFormat;
		
			NSError *error{ nullptr };
			id<MTLRenderPipelineState> pipelineState{ [[device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error] autorelease] };
			if(error != nullptr && error.code != 0) {
				return Unexpected{ std::runtime_error{ [[error description] cStringUsingEncoding:[NSString defaultCStringEncoding]] } };
			}
		
			return std::unique_ptr<GhaGraphicsPipelineObject>{ std::make_unique<MetalGraphicsPipelineObject>(pipelineState, depthStencilState) };
		}
	}
	
	Expected<std::unique_ptr<GhaComputePipelineObject>, std::runtime_error> MetalFactory::createComputePipelineObject(GhaComputePipelineObject::Descriptor descriptor) {
		@autoreleasepool {
			id<MTLFunction> function{ polyCast<MetalShader>(descriptor.shader.get())->getFunction() };
		
			NSError *error{ nullptr };
			id<MTLComputePipelineState> pipelineState{ [device newComputePipelineStateWithFunction:function
																							 error:&error] };
			if(error != nullptr && error.code != 0) {
				return Unexpected{ std::runtime_error{ [[error description]
														cStringUsingEncoding:[NSString defaultCStringEncoding]] } };
			}
			
			return std::unique_ptr<GhaComputePipelineObject>{ std::make_unique<MetalComputePipelineObject>(pipelineState) };
		}
	}

	Expected<std::unique_ptr<GhaFramebuffer>, std::runtime_error> MetalFactory::createFramebuffer(GhaFramebuffer::Descriptor descriptor) {
		GhaRenderPass::Descriptor const &renderPassDescriptor{ polyCast<MetalRenderPass>(descriptor.renderPass.get())->getDescriptor() };
		
		MTLRenderPassDescriptor *frameBufferDescriptor{ [[MTLRenderPassDescriptor alloc] init] };
		for(size_t i{ 0 }; i < renderPassDescriptor.colourAttachments.size(); ++i) {
			frameBufferDescriptor.colorAttachments[i].texture = polyCast<MetalImageView>(descriptor.attachments[i].get())->getTexture();
			frameBufferDescriptor.colorAttachments[i].loadAction = convertLoadOp(renderPassDescriptor.colourAttachments[i].loadOperation);
			frameBufferDescriptor.colorAttachments[i].storeAction = convertStoreOp(renderPassDescriptor.colourAttachments[i].storeOperation);
		}
		frameBufferDescriptor.depthAttachment.texture = polyCast<MetalImageView>(descriptor.attachments.back().get())->getTexture();
		frameBufferDescriptor.depthAttachment.loadAction = convertLoadOp(renderPassDescriptor.depthAttachment.loadOperation);
		frameBufferDescriptor.depthAttachment.storeAction = convertStoreOp(renderPassDescriptor.depthAttachment.storeOperation);
		
		auto result{ std::unique_ptr<GhaFramebuffer>{ std::make_unique<MetalFramebuffer>(frameBufferDescriptor) }};
		
		[frameBufferDescriptor release];
		
		return result;
	}
	
	Expected<std::unique_ptr<GhaDescriptorPool>, std::runtime_error> MetalFactory::createDescriptorPool(GhaDescriptorPool::Descriptor descriptor) {
		return std::unique_ptr<GhaDescriptorPool>{ std::make_unique<MetalDescriptorPool>(std::move(descriptor), device) };
	}

	Expected<std::unique_ptr<GhaSemaphore>, std::runtime_error> MetalFactory::createSemaphore() {
		id<MTLFence> fence{ [device newFence] };
		auto result{ std::unique_ptr<GhaSemaphore>{ std::make_unique<MetalSemaphore>(fence) } };
		
		[fence release];
		
		return result;
	}
	
	Expected<std::unique_ptr<GhaFence>, std::runtime_error> MetalFactory::createFence(GhaFence::Descriptor descriptor) {
		return std::unique_ptr<GhaFence>{ std::make_unique<MetalFence>(descriptor.signaled) };
	}

	Expected<std::unique_ptr<GhaBuffer>, std::runtime_error> MetalFactory::createBuffer(GhaBuffer::Descriptor descriptor) {
		MTLResourceOptions resourceOptions{};
		switch (descriptor.memoryType) {
			case MemoryType::VideoMemory:
				resourceOptions |= MTLResourceStorageModePrivate;
				break;
			case MemoryType::SystemMemory:
				resourceOptions |= MTLResourceStorageModeShared | MTLResourceCPUCacheModeDefaultCache;//Including MTLResourceCPUCacheModeDefaultCache here as this makes mapping memory more simple
				break;
			default:
				break;
		}
		
		id<MTLBuffer> buffer{ [device newBufferWithLength:descriptor.size options:resourceOptions] };
		auto result{ std::unique_ptr<GhaBuffer>{ std::make_unique<MetalBuffer>(buffer, descriptor) }};
		
		[buffer release];
		
		return result;
	}
	
	Expected<std::unique_ptr<GhaImage>, std::runtime_error> MetalFactory::createImage(GhaImage::Descriptor descriptor) {
		MTLTextureDescriptor *mtlDescriptor{ [[MTLTextureDescriptor alloc] init] };
		mtlDescriptor.textureType = convertImageType(descriptor.type, descriptor.arrayCount);
		mtlDescriptor.pixelFormat = MetalImage::convertFormat(descriptor.format);
		mtlDescriptor.width = descriptor.dimensions.x;
		mtlDescriptor.height = descriptor.dimensions.y;
		mtlDescriptor.arrayLength = descriptor.arrayCount;
		mtlDescriptor.resourceOptions = MTLResourceStorageModePrivate;
		mtlDescriptor.usage = getUsageFlags(descriptor.usageFlags);
		
		id<MTLTexture> texture{ [device newTextureWithDescriptor:mtlDescriptor] };
		auto result{ std::unique_ptr<GhaImage>{ std::make_unique<MetalImage>(texture, descriptor) }};
		
		[mtlDescriptor release];
		[texture release];
		
		return result;
	}

	Expected<std::unique_ptr<GhaSampler>, std::runtime_error> MetalFactory::createSampler(GhaSampler::Descriptor descriptor) {
		MTLSamplerDescriptor *samplerDescriptor{ [[MTLSamplerDescriptor alloc] init] };
		samplerDescriptor.minFilter     = convertMinMagFilter(descriptor.minFilter);
		samplerDescriptor.magFilter     = convertMinMagFilter(descriptor.magFilter);
		samplerDescriptor.sAddressMode  = convertAddressMode(descriptor.addressModeU);
		samplerDescriptor.tAddressMode  = convertAddressMode(descriptor.addressModeV);
		samplerDescriptor.rAddressMode  = convertAddressMode(descriptor.addressModeW);
		samplerDescriptor.maxAnisotropy = static_cast<NSUInteger>(descriptor.maxAnisotropy);
		samplerDescriptor.supportArgumentBuffers = true;
		
		id<MTLSamplerState> samplerState{ [device newSamplerStateWithDescriptor:samplerDescriptor] };
		
		auto result{ std::unique_ptr<GhaSampler>{ std::make_unique<MetalSampler>(samplerState) } };
		
		[samplerDescriptor release];
		[samplerState release];
		
		return result;
	}
	
	Expected<std::unique_ptr<GhaShader>, std::runtime_error> MetalFactory::createShaderObject(std::string const &mslSource) {
		@autoreleasepool {
			NSString *librarySource{ [NSString stringWithCString:mslSource.c_str() encoding:[NSString defaultCStringEncoding]] };
			if(librarySource == nullptr) {
				return Unexpected{ std::runtime_error{ "Shader creation failed. Could not convert std::string to NSString" } };
			}
			
			NSError *libError{ nullptr };
			id<MTLLibrary> library{ [[device newLibraryWithSource:librarySource options:nil error:&libError] autorelease] };
		
			if(libError != nullptr && libError.code != 0){
				return Unexpected{ std::runtime_error{ [[libError description] cStringUsingEncoding:[NSString defaultCStringEncoding]] } };
			}
		
			return std::unique_ptr<GhaShader>{ std::make_unique<MetalShader>([[library newFunctionWithName:@"main0"] autorelease]) }; //MSL can't have main so we use main0 (generated by spirv)
		}
	}
}

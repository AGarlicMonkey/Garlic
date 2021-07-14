#include "Clove/Graphics/Metal/MetalDescriptorPool.hpp"

#include "Clove/Graphics/Metal/MetalDescriptorSet.hpp"
#include "Clove/Graphics/Metal/MetalDescriptorSetLayout.hpp"

#include <Clove/Cast.hpp>

namespace garlic::clove {
	MetalDescriptorPool::MetalDescriptorPool(Descriptor descriptor, id<MTLDevice> device)
		: descriptor{ std::move(descriptor) }
		, device{ [device retain] } {
	}
	
	MetalDescriptorPool::MetalDescriptorPool(MetalDescriptorPool &&other) noexcept = default;
	
	MetalDescriptorPool &MetalDescriptorPool::operator=(MetalDescriptorPool &&other) noexcept = default;
	
	MetalDescriptorPool::~MetalDescriptorPool() {
		[device release];
	}
	
	GhaDescriptorPool::Descriptor const &MetalDescriptorPool::getDescriptor() const {
		return descriptor;
	}

	std::shared_ptr<GhaDescriptorSet> MetalDescriptorPool::allocateDescriptorSets(std::shared_ptr<GhaDescriptorSetLayout> const &layout) {
		return allocateDescriptorSets(std::vector{ layout })[0];
	}
	
	std::vector<std::shared_ptr<GhaDescriptorSet>> MetalDescriptorPool::allocateDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSetLayout>> const &layouts) {
		size_t const setNum{ layouts.size() };
		
		std::vector<MetalDescriptorSetLayout *> metalDescriptorSets{};
		metalDescriptorSets.reserve(setNum);
		for(size_t i{ 0 }; i < layouts.size(); ++i) {
			metalDescriptorSets.emplace_back(polyCast<MetalDescriptorSetLayout>(layouts[i].get()));
		}
		
		std::vector<std::shared_ptr<GhaDescriptorSet>> descriptorSets{};
		descriptorSets.reserve(setNum);
		@autoreleasepool {
			for(size_t i{ 0 }; i < layouts.size(); ++i) {
				id<MTLArgumentEncoder> vertexEncoder{ nullptr };
				id<MTLBuffer> vertexEncoderBuffer{ nullptr };
				if(metalDescriptorSets[i]->getVertexDescriptors().count > 0){
					vertexEncoder       = [device newArgumentEncoderWithArguments:metalDescriptorSets[i]->getVertexDescriptors()];
					vertexEncoderBuffer = [device newBufferWithLength:vertexEncoder.encodedLength options:0];
					
					[vertexEncoder setArgumentBuffer:vertexEncoderBuffer offset:0];
				}
				
				id<MTLArgumentEncoder> pixelEncoder{ nullptr };
				id<MTLBuffer> pixelEncoderBuffer{ nullptr };
				if(metalDescriptorSets[i]->getPixelDescriptors().count > 0){
					pixelEncoder       = [device newArgumentEncoderWithArguments:metalDescriptorSets[i]->getPixelDescriptors()];
					pixelEncoderBuffer = [device newBufferWithLength:pixelEncoder.encodedLength options:0];
					
					[pixelEncoder setArgumentBuffer:pixelEncoderBuffer offset:0];
				}
				
				id<MTLArgumentEncoder> computeEncoder{ nullptr };
				id<MTLBuffer> computeEncoderBuffer{ nullptr };
				if(metalDescriptorSets[i]->getComputeDescriptors().count > 0){
					computeEncoder       = [device newArgumentEncoderWithArguments:metalDescriptorSets[i]->getComputeDescriptors()];
					computeEncoderBuffer = [device newBufferWithLength:computeEncoder.encodedLength options:0];
					
					[computeEncoder setArgumentBuffer:computeEncoderBuffer offset:0];
				}
				
				descriptorSets.emplace_back(std::make_shared<MetalDescriptorSet>(vertexEncoder, vertexEncoderBuffer, pixelEncoder, pixelEncoderBuffer, computeEncoder, computeEncoderBuffer, layouts[i]));
			}
		}
		
		return descriptorSets;
	}

	void MetalDescriptorPool::freeDescriptorSets(std::shared_ptr<GhaDescriptorSet> const &descriptorSet) {
		//no op
	}
	
	void MetalDescriptorPool::freeDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSet>> const &descriptorSets) {
		//no op
	}

	void MetalDescriptorPool::reset() {
		//no op
	}
}

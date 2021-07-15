#include "Clove/Graphics/Metal/MetalDescriptorPool.hpp"

#include "Clove/Graphics/Metal/MetalDescriptorSet.hpp"

namespace garlic::clove {
	MetalDescriptorPool::MetalDescriptorPool(Descriptor descriptor)
		: descriptor{ std::move(descriptor) } {
	}
	
	MetalDescriptorPool::MetalDescriptorPool(MetalDescriptorPool &&other) noexcept = default;
	
	MetalDescriptorPool &MetalDescriptorPool::operator=(MetalDescriptorPool &&other) noexcept = default;
	
	MetalDescriptorPool::~MetalDescriptorPool() = default;
	
	GhaDescriptorPool::Descriptor const &MetalDescriptorPool::getDescriptor() const {
		return descriptor;
	}

	std::shared_ptr<GhaDescriptorSet> MetalDescriptorPool::allocateDescriptorSets(std::shared_ptr<GhaDescriptorSetLayout> const &layout) {
		return allocateDescriptorSets(std::vector{ layout })[0];
	}
	
	std::vector<std::shared_ptr<GhaDescriptorSet>> MetalDescriptorPool::allocateDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSetLayout>> const &layouts) {
		std::vector<std::shared_ptr<GhaDescriptorSet>> descriptorSets{};
		descriptorSets.reserve(layouts.size());
		
		//TODO: Argument buffer is created from the shader object
		
		for(size_t i{ 0 }; i < layouts.size(); ++i) {
			//descriptorSets.emplace_back(std::make_shared<MetalDescriptorSet>(layouts[i]));
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

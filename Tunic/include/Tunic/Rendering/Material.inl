#include "Tunic/Application.hpp"
#include "Clove/Graphics/Core/RenderFactory.hpp"

namespace tnc::rnd{
	template<typename DataType>
	void Material::setData(clv::gfx::BufferBindingPoint bindingPoint, DataType&& data, clv::gfx::ShaderType shaderType){
		if(auto iter = shaderData.find(bindingPoint); iter != shaderData.end()){
			iter->second.buffer->updateData(&data);
		} else{
			clv::gfx::BufferDescriptor srdesc{};
			srdesc.elementSize	= 0;
			srdesc.bufferSize	= sizeof(data);
			srdesc.bufferType	= clv::gfx::BufferType::ShaderResourceBuffer;
			srdesc.bufferUsage	= clv::gfx::BufferUsage::Dynamic;

			auto buffer = Application::get().getGraphicsFactory().createBuffer(srdesc, &data);
			shaderData[bindingPoint] = { buffer, shaderType };
		}
	}
}
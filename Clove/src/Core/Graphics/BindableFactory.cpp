#include "BindableFactory.hpp"

//GL
#include "Graphics/OpenGL-4/Bindables/GL4VertexBuffer.hpp"
#include "Graphics/OpenGL-4/Bindables/GL4IndexBuffer.hpp"
#include "Graphics/OpenGL-4/Bindables/GL4Shader.hpp"
#include "Graphics/OpenGL-4/Bindables/GL4Texture.hpp"

//DX
#if CLV_PLATFORM_WINDOWS
#include "Graphics/DirectX-11/Bindables/DX11VertexBuffer.hpp"
#include "Graphics/DirectX-11/Bindables/DX11IndexBuffer.hpp"
#include "Graphics/DirectX-11/Bindables/DX11Shader.hpp"
#include "Graphics/DirectX-11/Bindables/DX11Texture.hpp"
#endif

namespace clv::gfx::BindableFactory{
	std::shared_ptr<VertexBuffer> createVertexBuffer(const VertexBufferData& bufferData, Shader& shader){
		switch(RenderAPI::getAPIType()){
			case API::OpenGL4:
				return std::make_shared<GL4VertexBuffer>(bufferData, shader);

			#if CLV_PLATFORM_WINDOWS
			case API::DirectX11:
				return std::make_shared<DX11VertexBuffer>(bufferData, shader);
			#endif

			default:
				CLV_ASSERT(false, "Unkown API in: {0}", CLV_FUNCTION_NAME);
				return std::shared_ptr<VertexBuffer>();
		}
	}

	std::shared_ptr<IndexBuffer> createIndexBuffer(const std::vector<uint32>& indexData){
		switch(RenderAPI::getAPIType()){
			case API::OpenGL4:
				return std::make_shared<GL4IndexBuffer>(indexData);

			#if CLV_PLATFORM_WINDOWS
			case API::DirectX11:
				return std::make_shared<DX11IndexBuffer>(indexData);
			#endif

			default:
				CLV_ASSERT(false, "Unkown API in: {0}", CLV_FUNCTION_NAME);
				return std::shared_ptr<IndexBuffer>();
		}
	}

	std::shared_ptr<Shader> createShader(ShaderStyle style){
		switch(RenderAPI::getAPIType()){
			case API::OpenGL4:
				return std::make_shared<GL4Shader>(style);

			#if CLV_PLATFORM_WINDOWS
			case API::DirectX11:
				return std::make_shared<DX11Shader>(style);
			#endif

			default:
				CLV_ASSERT(false, "Unkown API in: {0}", CLV_FUNCTION_NAME);
				return std::shared_ptr<Shader>();
		}
	}

	std::shared_ptr<Texture> createTexture(const std::string& filePath, uint32 bindingPoint){
		switch(RenderAPI::getAPIType()){
			case API::OpenGL4:
				return std::make_shared<GL4Texture>(filePath, bindingPoint);

			#if CLV_PLATFORM_WINDOWS
			case API::DirectX11:
				return std::make_shared<DX11Texture>(filePath, bindingPoint);
			#endif

			default:
				CLV_ASSERT(false, "Unkown API in: {0}", CLV_FUNCTION_NAME);
				return std::shared_ptr<Texture>();
		}
	}

	std::shared_ptr<Texture> createTexture(void* bufferData, uint32 bindingPoint, const gfx::TextureDescriptor& descriptor){
		switch(RenderAPI::getAPIType()){
			case API::OpenGL4:
				return std::make_shared<GL4Texture>(bufferData, bindingPoint, descriptor);

			#if CLV_PLATFORM_WINDOWS
			case API::DirectX11:
				return std::make_shared<DX11Texture>(bufferData, bindingPoint, descriptor);
			#endif

			default:
				CLV_ASSERT(false, "Unkown API in: {0}", CLV_FUNCTION_NAME);
				return std::shared_ptr<Texture>();
		}
	}

	std::shared_ptr<Texture> createTexture(uint32 bindingPoint, const gfx::TextureDescriptor& descriptor){
		switch(RenderAPI::getAPIType()){
			case API::OpenGL4:
				return std::make_shared<GL4Texture>(bindingPoint, descriptor);

			#if CLV_PLATFORM_WINDOWS
			case API::DirectX11:
				return std::make_shared<DX11Texture>(bindingPoint, descriptor);
			#endif

			default:
				CLV_ASSERT(false, "Unkown API in: {0}", CLV_FUNCTION_NAME);
				return std::shared_ptr<Texture>();
		}
	}
}
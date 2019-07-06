#include "clvpch.hpp"
#include "DX11Shader.hpp"

#include "Clove/Application.hpp"
#include "Clove/Platform/Window.hpp"
#include "Graphics/DirectX-11/DX11Exception.hpp"
#include "Graphics/DirectX-11/DX11Renderer.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>

namespace clv::gfx{
	DX11Shader::DX11Shader() = default;

	DX11Shader::DX11Shader(DX11Shader&& other) noexcept{
		shaders = std::move(other.shaders);
		vertexShader = other.vertexShader;
	}

	DX11Shader& DX11Shader::operator=(DX11Shader&& other) noexcept{
		shaders = std::move(other.shaders);
		vertexShader = other.vertexShader;

		return *this;
	}

	DX11Shader::~DX11Shader() = default;

	void DX11Shader::bind(Renderer& renderer){
		for(const auto& [key, shader] : shaders){
			shader->bind(renderer);
		}
	}

	void DX11Shader::unbind(){
	}

	void DX11Shader::attachShader(ShaderTypes type){
		switch(type){
			case ShaderTypes::Vertex:
				{
					auto vs = std::make_unique<DX11VertexShader>(L"Default-vs.cso");
					vertexShader = vs.get();
					shaders[type] = std::move(vs);
				}
				break;
			case ShaderTypes::Pixel:
				shaders[type] = std::make_unique<DX11PixelShader>(L"Default-ps.cso");
				break;

			default:
				CLV_ASSERT(false, "Unknown type! " __FUNCTION__);
				break;
		}
	}

	DX11VertexShader& DX11Shader::getVertexShader(){
		CLV_ASSERT(vertexShader != nullptr, "No vertex shader attached!");
		return *vertexShader;
	}

	DX11VertexShader::DX11VertexShader(DX11VertexShader&& other) noexcept = default;

	DX11VertexShader& DX11VertexShader::operator=(DX11VertexShader&& other) noexcept = default;

	DX11VertexShader::~DX11VertexShader() = default;

	DX11VertexShader::DX11VertexShader(const std::wstring& path){
		DX11Renderer* dxrenderer = static_cast<DX11Renderer*>(&Application::get().getRenderer());
		DX11_INFO_PROVIDER(dxrenderer);

		DX11_THROW_INFO(D3DReadFileToBlob(path.c_str(), &byteCode));
		DX11_THROW_INFO(dxrenderer->getDevice().CreateVertexShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &vertexShader));
	}

	void DX11VertexShader::bind(Renderer& renderer){
		DX11Renderer* dxrenderer = static_cast<DX11Renderer*>(&renderer);
		dxrenderer->getContext().VSSetShader(vertexShader.Get(), nullptr, 0u);
	}

	void DX11VertexShader::unbind(){
	}

	ID3DBlob* DX11VertexShader::getByteCode() const{
		return byteCode.Get();
	}

	DX11PixelShader::DX11PixelShader(DX11PixelShader&& other) noexcept = default;

	DX11PixelShader& DX11PixelShader::operator=(DX11PixelShader&& other) noexcept = default;

	DX11PixelShader::~DX11PixelShader() = default;

	DX11PixelShader::DX11PixelShader(const std::wstring& path){
		DX11Renderer* dxrenderer = static_cast<DX11Renderer*>(&Application::get().getRenderer());
		DX11_INFO_PROVIDER(dxrenderer);

		DX11_THROW_INFO(D3DReadFileToBlob(path.c_str(), &byteCode));
		DX11_THROW_INFO(dxrenderer->getDevice().CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &pixelShader));
	}

	void DX11PixelShader::bind(Renderer& renderer){
		DX11Renderer* dxrenderer = static_cast<DX11Renderer*>(&renderer);
		dxrenderer->getContext().PSSetShader(pixelShader.Get(), nullptr, 0u);
	}

	void DX11PixelShader::unbind(){
	}

	ID3DBlob* DX11PixelShader::getByteCode() const{
		return byteCode.Get();
	}
}
#include "clvpch.hpp"
#include "DX11VertexBuffer.hpp"

#include "Clove/Application.hpp"
#include "Clove/Platform/Window.hpp"
#include "Graphics/DirectX-11/DX11Renderer.hpp"

#include <d3d11.h>

namespace clv::gfx{
	DX11VertexBuffer::DX11VertexBuffer(DX11VertexBuffer&& other) noexcept = default;

	DX11VertexBuffer& DX11VertexBuffer::operator=(DX11VertexBuffer&& other) noexcept = default;

	DX11VertexBuffer::~DX11VertexBuffer() = default;

	DX11VertexBuffer::DX11VertexBuffer(const std::vector<float>& vertices){
		Renderer& renderer = Application::get().getWindow().getRenderer();
		if(DX11Renderer* dxrenderer = dynamic_cast<DX11Renderer*>(&renderer)){
			DX11_INFO_PROVIDER(dxrenderer);

			D3D11_BUFFER_DESC vbd = {};
			vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbd.Usage = D3D11_USAGE_DEFAULT;
			vbd.CPUAccessFlags = 0;
			vbd.MiscFlags = 0u;
			vbd.ByteWidth = vertices.size() * sizeof(float);
			vbd.StructureByteStride = sizeof(float);

			D3D11_SUBRESOURCE_DATA vsrd = {};
			vsrd.pSysMem = vertices.data();

			//TODO: This will only compile on release without the info manager
			DX11_THROW_INFO(dxrenderer->getDevice().CreateBuffer(&vbd, &vsrd, &vertexBuffer));
		}
	}

	void DX11VertexBuffer::bind(Renderer& renderer){
		if(DX11Renderer* dxrenderer = dynamic_cast<DX11Renderer*>(&renderer)){
			const UINT stride = sizeof(float) * 8; //TODO: This needs to keep increasing when adding new elements - replace the data with a vertex struct so we can just size of that
			const UINT offset = 0u;
			dxrenderer->getContext().IASetVertexBuffers(0u, 1u, vertexBuffer.GetAddressOf(), &stride, &offset);
		}
	}

	void DX11VertexBuffer::unbind(){
	}
}
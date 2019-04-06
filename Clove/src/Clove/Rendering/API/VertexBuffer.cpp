#include "clvpch.h"
#include "VertexBuffer.h"

#include "Clove/Rendering/Renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace clv{
	VertexBuffer::VertexBuffer(const void* data, unsigned int size){
		GLCall(glGenBuffers(1, &rendererID));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, rendererID));
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	}

	VertexBuffer::VertexBuffer(VertexBuffer&& other){
		rendererID = other.rendererID;

		other.rendererID = 0;
	}

	VertexBuffer::~VertexBuffer(){
		GLCall(glDeleteBuffers(1, &rendererID));
	}

	void VertexBuffer::bind() const{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, rendererID));
	}

	void VertexBuffer::unbind() const{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other){
		rendererID = other.rendererID;

		other.rendererID = 0;

		return *this;
	}
}
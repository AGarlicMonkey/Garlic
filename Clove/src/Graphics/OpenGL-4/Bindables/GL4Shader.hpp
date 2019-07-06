#pragma once

#include "Clove/Graphics/Bindables/Shader.hpp"

#include "Graphics/OpenGL-4/Bindables/GL4UniformBufferObject.hpp"

namespace clv::gfx{
	class Renderer;

	class GL4Shader : public Shader{
		//VARIABLES
	private:
		unsigned int programID = 0;

		//FUNCTIONS
	public:
		GL4Shader();
		GL4Shader(const GL4Shader& other) = delete;
		GL4Shader(GL4Shader&& other) noexcept;
		GL4Shader& operator=(const GL4Shader& other) = delete;
		GL4Shader& operator=(GL4Shader&& other) noexcept;
		~GL4Shader();

		virtual void bind(Renderer& renderer) override;
		virtual void unbind() override;

		virtual void attachShader(ShaderTypes type) override;

	private:
		std::string getPathForShader(ShaderTypes shader);

		std::string parseShader(const std::string& filepath);
		unsigned int compileShader(unsigned int type, const std::string& source);
	};
}
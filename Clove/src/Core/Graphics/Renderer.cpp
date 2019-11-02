#include "Renderer.hpp"

#include "Core/Graphics/RenderCommand.hpp"
#include "Core/Graphics/Bindables/VertexBuffer.hpp"
#include "Core/Graphics/Bindables/IndexBuffer.hpp"
#include "Core/Graphics/BindableFactory.hpp"
#include "Core/Graphics/Bindables/Shader.hpp"
#include "Core/Graphics/Bindables/Texture.hpp"
#include "Core/Graphics/RenderTarget.hpp"
#include "Core/Application.hpp"
#include "Core/Platform/Window.hpp"
#include "Core/Graphics/Mesh.hpp"
#include "Core/Graphics/Material.hpp"
#include "Core/Graphics/Sprite.hpp"
#include "Core/Application.hpp"
#include "Core/Platform/Window.hpp"
#include "Core/Graphics/Bindables/Shader.hpp"
#include "Core/Graphics/ShaderBufferTypes.hpp"

namespace clv::gfx{
	struct SceneData{
		//VARIABLES
	public:
		CameraRenderData currentCamData;

		std::vector<std::shared_ptr<Mesh>> meshesToRender;

		PointLightShaderData currentLightInfo;
		PointShadowDepthData currentShadowDepth;
		uint32 numLights;
		std::array<std::array<math::Matrix4f, 6>, MAX_LIGHTS> shadowTransforms = {}; 

		MaterialInstance cubeShadowMaterial;

		//Other
		std::shared_ptr<RenderTarget> shadowMapRenderTarget;
		std::shared_ptr<Texture> shadowMapTexture;

		std::shared_ptr<RenderTarget> customRenderTarget;

		//FUNCTIONS
	public:
		SceneData()
			: cubeShadowMaterial(std::make_shared<Material>(ShaderStyle::CubeShadowMap)){

			shadowMapTexture = BindableFactory::createTexture(TBP_Shadow, { TextureStyle::Cubemap, TextureUsage::RenderTarget_Depth, { shadowMapSize, shadowMapSize }, MAX_LIGHTS });
			shadowMapRenderTarget = RenderTarget::createRenderTarget(nullptr, shadowMapTexture.get());
		}

		~SceneData() = default;

		template<typename FunctionPointer>
		void forEachMesh(FunctionPointer&& function){
			std::for_each(meshesToRender.begin(), meshesToRender.end(), function);
		}
	} *currentSceneData = nullptr;

	void Renderer::initialise(){
		CLV_LOG_TRACE("Initialising renderer");
		currentSceneData = new SceneData();
		RenderCommand::setBlendState(true);
	}

	void Renderer::shutDown(){
		CLV_LOG_TRACE("Shutting down renderer");
		delete currentSceneData;
	}

	void Renderer::beginScene(){
		currentSceneData->numLights = 0;
	}

	void Renderer::endScene(){
		const auto draw = [](const std::shared_ptr<Mesh>& mesh){
			auto& meshMaterial = mesh->getMaterialInstance();

			//TODO: I like setting them on the material, but I either need a global material or set it on the base material
			//Perhaps some sort of 'material codex' where I can pull the base material for a shader type?

			//Camera
			meshMaterial.setData(BBP_CameraMatrices, ViewData{ currentSceneData->currentCamData.lookAt, currentSceneData->currentCamData.projection }, ShaderType::Vertex);
			meshMaterial.setData(BBP_ViewData, ViewPos{ currentSceneData->currentCamData.position }, ShaderType::Pixel);
			
			//Lights
			meshMaterial.setData(BBP_PointLightData, PointLightShaderData{ currentSceneData->currentLightInfo }, ShaderType::Pixel);
			meshMaterial.setData(BBP_CubeDepthData, PointShadowDepthData{ currentSceneData->currentShadowDepth }, ShaderType::Pixel);
			meshMaterial.setData(BBP_CurrentLights, LightNumAlignment{ currentSceneData->numLights }, ShaderType::Pixel);

			mesh->bind();

			RenderCommand::drawIndexed(mesh->getIndexCount());
		};

		const auto generateShadowMap = [](const std::shared_ptr<Mesh>& mesh){
			mesh->bind();
			currentSceneData->cubeShadowMaterial.bind(); //Bind in the shader / SBOs for generating the cubemap
			RenderCommand::drawIndexed(mesh->getIndexCount());
		};

		RenderCommand::setDepthBuffer(true);

		//Calculate shadow map
		for(uint32 i = 0; i < currentSceneData->numLights; ++i){
			currentSceneData->cubeShadowMaterial.setData(BBP_ShadowData, PointShadowShaderData{ currentSceneData->shadowTransforms[i] }, ShaderType::Geometry);
			currentSceneData->cubeShadowMaterial.setData(BBP_CurrentFaceIndex, LightNumAlignment{ i * 6 }, ShaderType::Geometry);
			currentSceneData->cubeShadowMaterial.setData(BBP_CurrentDepthData, PointShadowData{ currentSceneData->currentShadowDepth.depths[i] }, ShaderType::Pixel);

			RenderCommand::setViewPortSize(shadowMapSize, shadowMapSize);
			RenderCommand::setRenderTarget(*currentSceneData->shadowMapRenderTarget);
			currentSceneData->forEachMesh(generateShadowMap);
			RenderCommand::setViewPortSize(Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight());
		}
		
		RenderCommand::removeCurrentGeometryShader();

		RenderCommand::resetRenderTarget(); //Reset render target before binding the shadow map

		currentSceneData->shadowMapTexture->bind(); //Bind this in before rendering the real mesh

		//Render any other render targets
		if(currentSceneData->customRenderTarget){
			RenderCommand::setRenderTarget(*currentSceneData->customRenderTarget);
			currentSceneData->forEachMesh(draw);
			RenderCommand::resetRenderTarget();
		}

		//Render scene
		currentSceneData->forEachMesh(draw);

		RenderCommand::removeTextureAtSlot(TBP_Shadow);

		currentSceneData->meshesToRender.clear();
	}

	void Renderer::setRenderTarget(const std::shared_ptr<RenderTarget>& inRenderTarget){
		currentSceneData->customRenderTarget = inRenderTarget;
	}

	void Renderer::clearRenderTargets(){
		RenderCommand::setRenderTarget(*currentSceneData->shadowMapRenderTarget);
		RenderCommand::clear(); //TODO: Might need to just do a clear depth command
		if(currentSceneData->customRenderTarget){
			RenderCommand::setRenderTarget(*currentSceneData->customRenderTarget);
			RenderCommand::clear();
		}
		RenderCommand::resetRenderTarget();
		RenderCommand::clear();
	}

	void Renderer::removeRenderTarget(){
		currentSceneData->customRenderTarget.reset();
		RenderCommand::resetRenderTarget();
	}

	void Renderer::submitMesh(const std::shared_ptr<Mesh>& mesh){
		currentSceneData->meshesToRender.push_back(mesh);
	}

	void Renderer::setCamera(const CameraRenderData& data){
		currentSceneData->currentCamData = data;
	}

	void Renderer::submitPointLight(const PointLightData& data){
		const int32 lightIndex = currentSceneData->numLights++;
		currentSceneData->currentLightInfo.intensities[lightIndex] = data.intensity;

		currentSceneData->shadowTransforms[lightIndex] = data.shadowTransforms;

		currentSceneData->currentShadowDepth.depths[lightIndex].farPlane = data.farPlane;
		currentSceneData->currentShadowDepth.depths[lightIndex].lightPos = data.intensity.position;
	}
}
set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/Clove)
set(ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets)

add_subdirectory(components)
add_subdirectory(external)

#Prepare SOURCEs to be inbedded
set(
	ShaderIncludes
		${SOURCE}/Shaders/Constants.glsl
)

set(
	ShaderFiles
		${SOURCE}/Shaders/Mesh-v.glsl
		${SOURCE}/Shaders/MeshShadowMap-v.glsl
		${SOURCE}/Shaders/MeshCubeShadowMap-v.glsl

		${SOURCE}/Shaders/Mesh-p.glsl
		${SOURCE}/Shaders/MeshShadowMap-p.glsl
		${SOURCE}/Shaders/MeshCubeShadowMap-p.glsl

		${SOURCE}/Shaders/UI-v.glsl
		${SOURCE}/Shaders/Widget-p.glsl
		${SOURCE}/Shaders/Font-p.glsl

		${SOURCE}/Shaders/Skinning-c.glsl

		${ShaderIncludes}
)

#Convert SOURCEs into c strings
set(ResourceBinaries)

function(embeddSOURCE SOURCEs readType)
	set(files)

	foreach(res IN ITEMS ${SOURCEs})
		get_filename_component(fileName ${res} NAME_WLE)

		string(REPLACE "-" "_" fileName ${fileName})
		string(TOLOWER ${fileName} fileName)

		add_custom_command(
			OUTPUT ${fileName}.c
			COMMAND reb ${fileName} ${res} ${readType}
			DEPENDS ${res}
		)

		set(files ${files} "${fileName}.c")
	endforeach()

	set(ResourceBinaries ${ResourceBinaries} ${files} PARENT_SCOPE)
endfunction()

#Assets
embeddSOURCE("${ASSETS}/fonts/Roboto/Roboto-Black.ttf" "rb")

#Shader include
embeddSOURCE("${SOURCE}/Shaders/Constants.glsl" "r")

#Shader source
embeddSOURCE("${SOURCE}/Shaders/Mesh-v.glsl" "r")
embeddSOURCE("${SOURCE}/Shaders/MeshShadowMap-v.glsl" "r")
embeddSOURCE("${SOURCE}/Shaders/MeshCubeShadowMap-v.glsl" "r")

embeddSOURCE("${SOURCE}/Shaders/Mesh-p.glsl" "r")
embeddSOURCE("${SOURCE}/Shaders/MeshShadowMap-p.glsl" "r")
embeddSOURCE("${SOURCE}/Shaders/MeshCubeShadowMap-p.glsl" "r")

embeddSOURCE("${SOURCE}/Shaders/UI-v.glsl" "r")
embeddSOURCE("${SOURCE}/Shaders/Widget-p.glsl" "r")
embeddSOURCE("${SOURCE}/Shaders/Font-p.glsl" "r")

embeddSOURCE("${SOURCE}/Shaders/Skinning-c.glsl" "r")

#Library
add_library(
	Clove STATIC

		${INCLUDE}/Components/AnimatedModelComponent.hpp
		${INCLUDE}/Components/AudioListenerComponent.hpp
		${INCLUDE}/Components/AudioSourceComponent.hpp
		${INCLUDE}/Components/CameraComponent.hpp
		${INCLUDE}/Components/CollisionResponseComponent.hpp
		${INCLUDE}/Components/CollisionShapeComponent.hpp
		${INCLUDE}/Components/DirectionalLightComponent.hpp
		${INCLUDE}/Components/ParentComponent.hpp
		${INCLUDE}/Components/PointLightComponent.hpp
		${INCLUDE}/Components/RigidBodyComponent.hpp
		${SOURCE}/Components/RigidBodyComponent.cpp
		${INCLUDE}/Components/StaticModelComponent.hpp
		${INCLUDE}/Components/TransformComponent.hpp
		${SOURCE}/Components/TransformComponent.cpp

		${INCLUDE}/Rendering/AnimationTypes.hpp
		${INCLUDE}/Rendering/Animator.hpp
		${SOURCE}/Rendering/Animator.cpp
		${INCLUDE}/Rendering/Camera.hpp
		${INCLUDE}/Rendering/Camera.inl
		${SOURCE}/Rendering/Camera.cpp
		${INCLUDE}/Rendering/ForwardRenderer3D.hpp
		${SOURCE}/Rendering/ForwardRenderer3D.cpp
		${INCLUDE}/Rendering/GraphicsImageRenderTarget.hpp
		${SOURCE}/Rendering/GraphicsImageRenderTarget.cpp
		${INCLUDE}/Rendering/Material.hpp
		${INCLUDE}/Rendering/Material.inl
		${SOURCE}/Rendering/Material.cpp
		${INCLUDE}/Rendering/RenderingConstants.hpp
		${INCLUDE}/Rendering/RenderingHelpers.hpp
		${SOURCE}/Rendering/RenderingHelpers.cpp
		${INCLUDE}/Rendering/RenderTarget.hpp
		${INCLUDE}/Rendering/SwapchainRenderTarget.hpp
		${SOURCE}/Rendering/SwapchainRenderTarget.cpp
		${INCLUDE}/Rendering/ShaderBufferTypes.hpp
		${INCLUDE}/Rendering/Vertex.hpp
		${INCLUDE}/Rendering/Renderables/Font.hpp
		${SOURCE}/Rendering/Renderables/Font.cpp
		${INCLUDE}/Rendering/Renderables/Mesh.hpp
		${INCLUDE}/Rendering/Renderables/Mesh.inl
		${SOURCE}/Rendering/Renderables/Mesh.cpp
		${INCLUDE}/Rendering/Renderables/AnimatedModel.hpp
		${INCLUDE}/Rendering/Renderables/AnimatedModel.inl
		${SOURCE}/Rendering/Renderables/AnimatedModel.cpp
		${INCLUDE}/Rendering/Renderables/StaticModel.hpp
		${INCLUDE}/Rendering/Renderables/StaticModel.inl
		${SOURCE}/Rendering/Renderables/StaticModel.cpp

		${INCLUDE}/Rendering/RenderGraph/CacheUtils.hpp
		${INCLUDE}/Rendering/RenderGraph/CacheUtils.inl
		${INCLUDE}/Rendering/RenderGraph/RenderGraph.hpp
		${SOURCE}/Rendering/RenderGraph/RenderGraph.cpp
		${INCLUDE}/Rendering/RenderGraph/Bindings.hpp
		${INCLUDE}/Rendering/RenderGraph/RgBuffer.hpp
		${INCLUDE}/Rendering/RenderGraph/RgBuffer.inl
		${SOURCE}/Rendering/RenderGraph/RgBuffer.cpp
		${INCLUDE}/Rendering/RenderGraph/RgComputePass.hpp
		${INCLUDE}/Rendering/RenderGraph/RgComputePass.inl
		${SOURCE}/Rendering/RenderGraph/RgComputePass.cpp
		${INCLUDE}/Rendering/RenderGraph/RgFrameCache.hpp
		${SOURCE}/Rendering/RenderGraph/RgFrameCache.cpp
		${INCLUDE}/Rendering/RenderGraph/RgGlobalCache.hpp
		${SOURCE}/Rendering/RenderGraph/RgGlobalCache.cpp
		${INCLUDE}/Rendering/RenderGraph/RgId.hpp
		${INCLUDE}/Rendering/RenderGraph/RgImage.hpp
		${INCLUDE}/Rendering/RenderGraph/RgImage.inl
		${SOURCE}/Rendering/RenderGraph/RgImage.cpp
		${INCLUDE}/Rendering/RenderGraph/RgPass.hpp
		${INCLUDE}/Rendering/RenderGraph/RgPass.inl
		${SOURCE}/Rendering/RenderGraph/RgPass.cpp
		${INCLUDE}/Rendering/RenderGraph/RgRenderPass.hpp
		${INCLUDE}/Rendering/RenderGraph/RgRenderPass.inl
		${SOURCE}/Rendering/RenderGraph/RgRenderPass.cpp
		${INCLUDE}/Rendering/RenderGraph/RgResource.hpp
		${INCLUDE}/Rendering/RenderGraph/RgResource.inl
		${SOURCE}/Rendering/RenderGraph/RgResource.cpp
		${INCLUDE}/Rendering/RenderGraph/RgShader.hpp
		${INCLUDE}/Rendering/RenderGraph/RgTransferPass.hpp
		${INCLUDE}/Rendering/RenderGraph/RgTransferPass.inl
		${SOURCE}/Rendering/RenderGraph/RgTransferPass.cpp
		
		${INCLUDE}/Rendering/RenderPasses/ForwardColourPass.hpp
		${SOURCE}/Rendering/RenderPasses/ForwardColourPass.cpp
		${INCLUDE}/Rendering/RenderPasses/DirectionalLightPass.hpp
		${SOURCE}/Rendering/RenderPasses/DirectionalLightPass.cpp
		${INCLUDE}/Rendering/RenderPasses/GeometryPass.hpp
		${INCLUDE}/Rendering/RenderPasses/GeometryPass.inl
		${SOURCE}/Rendering/RenderPasses/GeometryPass.cpp
		${INCLUDE}/Rendering/RenderPasses/PointLightPass.hpp
		${SOURCE}/Rendering/RenderPasses/PointLightPass.cpp
		${INCLUDE}/Rendering/RenderPasses/SkinningPass.hpp
		${SOURCE}/Rendering/RenderPasses/SkinningPass.cpp
		${INCLUDE}/Rendering/Techniques/ForwardLightingTechnique.hpp
		${SOURCE}/Rendering/Techniques/ForwardLightingTechnique.cpp
		${INCLUDE}/Rendering/Techniques/Technique.hpp

		${INCLUDE}/Layers/AudioLayer.hpp
		${SOURCE}/Layers/AudioLayer.cpp
		${INCLUDE}/Layers/PhysicsLayer.hpp
		${SOURCE}/Layers/PhysicsLayer.cpp
		${INCLUDE}/Layers/RenderLayer.hpp
		${SOURCE}/Layers/RenderLayer.cpp
		${INCLUDE}/Layers/TransformLayer.hpp
		${SOURCE}/Layers/TransformLayer.cpp

		${INCLUDE}/UI/DrawableElement.hpp
		${INCLUDE}/UI/Image.hpp
		${INCLUDE}/UI/Image.inl
		${SOURCE}/UI/Image.cpp
		${INCLUDE}/UI/InputElement.hpp
		${INCLUDE}/UI/Text.hpp
		${INCLUDE}/UI/Text.inl
		${SOURCE}/UI/Text.cpp
		${INCLUDE}/UI/UIFrame.hpp
		${SOURCE}/UI/UIFrame.cpp

		${INCLUDE}/Application.hpp
		${INCLUDE}/Application.inl
		${SOURCE}/Application.cpp

		${INCLUDE}/InputEvent.hpp
		${INCLUDE}/InputResponse.hpp

		${INCLUDE}/Layer.hpp
		${INCLUDE}/Layer.inl
		${SOURCE}/Layer.cpp
		
		${INCLUDE}/ModelLoader.hpp
		${SOURCE}/ModelLoader.cpp

		${INCLUDE}/SerialisationCommon.hpp

		${INCLUDE}/SoundFile.hpp
		${SOURCE}/SoundFile.cpp

		${INCLUDE}/TextureLoader.hpp
		${SOURCE}/TextureLoader.cpp

		${INCLUDE}/Surface.hpp
		${INCLUDE}/WindowSurface.hpp
		${SOURCE}/WindowSurface.cpp

		${ShaderFiles}

		${ResourceBinaries}
)

#Set as header only so VS doesn't try to compile them
set_source_files_properties(${ShaderFiles} PROPERTIES HEADER_FILE_ONLY TRUE)

#Includes
target_include_directories(
	Clove

	PUBLIC
		include
)

#Libraries
target_link_libraries(
	Clove

	#Components 
	PUBLIC 
		#Core 
		CloveAudio 
		CloveGraphics 
		ClovePlatform 
 
		#Systems
		CloveAI
		CloveECS
		CloveSerialisation
		CloveStateMachine
		 
		#Utilities
		CloveCast 
		CloveDefinitions 
		CloveDelegate 
		CloveDeltaTime 
		CloveEvent 
		CloveExpected 
		CloveLog 
		CloveMaths 
		CloveMemory 
		CloveTimer

	PRIVATE
		CONAN_PKG::assimp
		CONAN_PKG::bullet3
		CONAN_PKG::freetype
		CONAN_PKG::libsndfile
		stb
)
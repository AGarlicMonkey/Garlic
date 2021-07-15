#include "Clove/Graphics/Graphics.hpp"

#if CLOVE_PLATFORM_WINDOWS
    #include "Clove/Graphics/Vulkan/VulkanDevice.hpp"
#elif CLOVE_PLATFORM_MACOS
	#include "Clove/Graphics/Metal/MetalDevice.hpp"
#elif CLOVE_PLATFORM_LINUX
    #include "Clove/Graphics/Vulkan/VulkanDevice.hpp"
#endif

#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>

namespace garlic::clove {
    std::unique_ptr<GhaDevice> createGraphicsDevice(GraphicsApi api, std::any nativeWindow) { //NOLINT Temp no lint on macos until this is implemented
        switch(api) {
#if CLOVE_PLATFORM_WINDOWS
            case GraphicsApi::Vulkan:
                return std::make_unique<VulkanDevice>(std::move(nativeWindow));
#elif CLOVE_PLATFORM_MACOS
			case GraphicsApi::Metal:
				return std::make_unique<MetalDevice>(std::move(nativeWindow));
#elif CLOVE_PLATFORM_LINUX
            case GraphicsApi::Vulkan:
                return std::make_unique<VulkanDevice>(std::move(nativeWindow));
#endif
            default:
                CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Default statement hit. Could not initialise RenderAPI: {0}", CLOVE_FUNCTION_NAME);
                return nullptr;
        }
    }
}

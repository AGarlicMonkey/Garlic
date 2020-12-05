#pragma once

namespace garlic::clove {
    enum class GraphicsApi {
        None,
#if CLOVE_PLATFORM_WINDOWS
        Vulkan,
#elif CLOVE_PLATFORM_LINUX
        Vulkan,
#elif CLOVE_PLATFORM_MACOS
//TODO: Metal
#endif
    };
}
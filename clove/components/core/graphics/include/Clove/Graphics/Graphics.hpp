#pragma once

#include "Clove/Graphics/GraphicsAPI.hpp"

#include <any>
#include <memory>

namespace garlic::clove {
    class GhaDevice;
}

namespace garlic::clove {
    /**
     * @brief Create a Graphics Device to communicate with the graphics hardware.
     * @param api What API to use.
     * @param nativeWindow The window to bind to. Using an empty std::any will create a headless device.
     */
    std::unique_ptr<GhaDevice> createGraphicsDevice(GraphicsApi api, std::any nativeWindow);
}

#pragma once

#include <memory>

namespace garlic::clove {
    enum class AudioApi {
        OpenAl
    };

    class AhaDevice;
}

namespace garlic::clove {
    std::unique_ptr<AhaDevice> createAudioDevice(AudioApi api);
}
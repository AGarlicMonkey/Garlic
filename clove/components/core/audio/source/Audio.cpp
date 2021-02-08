#include "Clove/Audio/Audio.hpp"

#include "Clove/Audio/OpenAL/OpenAlDevice.hpp"

namespace garlic::clove {
    std::unique_ptr<AhaDevice> createAudioDevice(AudioApi api) {
        switch(api) {
            case AudioApi::OpenAl:
                return std::make_unique<OpenAlDevice>();
        }
    }
}
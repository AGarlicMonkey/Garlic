#include "Clove/Layer.hpp"

namespace garlic::clove {
    Layer::Layer(std::string name)
        : debugName(std::move(name)) {
    }

    InputResponse Layer::onInputEvent(InputEvent const &inputEvent) {
        return InputResponse::Ignored;
    }
}

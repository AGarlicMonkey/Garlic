#pragma once

#include "Clove/Input/KeyCodes.hpp"
#include "Clove/Input/MouseButtonCodes.hpp"

namespace clv{
	namespace input{
		bool isKeyPressed(Key key);

		bool isMouseButtonPressed(MouseButton button);
		std::pair<float, float> getMousePosition();
		float getMouseX();
		float getMouseY();
	};
}
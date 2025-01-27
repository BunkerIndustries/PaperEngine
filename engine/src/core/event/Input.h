#pragma once
#include "Engine.h"

#include "ButtonCodes.h"

namespace Paper {

	class Input
	{
	public:
		static bool IsKeyPressed(Key code);
		static bool IsKeyHeld(Key code);
		static bool IsKeyDown(Key code);
		static bool IsKeyReleased(Key code);

		static bool IsMouseButtonPressed(MouseButton button);
		static bool IsMouseButtonHeld(MouseButton button);
		static bool IsMouseButtonDown(MouseButton button);
		static bool IsMouseButtonReleased(MouseButton button);
		static glm::vec2 GetMousPos();

		static void UpdateKeyState(Key key, KeyState keyState);
		static void UpdateMouseButtonState(MouseButton button, KeyState keyState);

	private:
		friend class Application;

		static void Update();

		static std::unordered_map<Key, KeyState> keyStates;
		static std::unordered_map<MouseButton, KeyState> mouseButtonStates;
	};

}

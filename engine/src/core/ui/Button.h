#pragma once
#include "Engine.h"

#include "UIObject.h"
#include "event/Event.h"

namespace engine
{
	using ButtonEventCallbackFunction = std::function<void()>;

	class Button : public UIObject
	{
		friend class Input;
	public:
		Button(glm::vec4 color, const Transform& transform, Type type, const ButtonEventCallbackFunction& buttonEventFunction, std::string name = "Button");
		~Button() override;

		void Render(core_id idToRender) override;

	private:
		Type type;
		ButtonEventCallbackFunction buttonEventFunction;
	};
}


#pragma once
#include "Engine.h"

#include "UIObject.h"

namespace engine
{
	class Label : public UIObject
	{
	public:
		Label(std::string text, glm::vec4 color, const Transform& transform, const Shr<Font>& font, std::string name = "Label");
		~Label() override;

		void Render(core_id idToRender) override;

		std::string text;
	private:
		Shr<Font> font;
	};
}


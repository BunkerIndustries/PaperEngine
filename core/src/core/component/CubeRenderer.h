﻿#pragma once
#include "_Core.h"

#include "generic/Component.h"

namespace core
{
	class CubeRenderer : public RenderComponent
	{
	public:
		CubeRenderer(glm::vec4 color);
		CubeRenderer(glm::vec4 color, Shr<Texture> texture);
		CubeRenderer(glm::vec4 color, Shr<Texture> texture, float tilingFactor);
		~CubeRenderer() override = default;

		void OnStart() override { };
		void OnStop() override { };
		void OnUpdate() override;
		void OnEvent(Event& event) override { };

	private:
		glm::vec4 color;
		Shr<Texture> texture;
		float tilingFactor;
	};
}

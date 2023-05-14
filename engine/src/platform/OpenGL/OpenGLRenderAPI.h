#pragma once
#include "Engine.h"
#include "utility.h"

#include "core/renderer/RenderAPI.h"

namespace engine
{
	class OpenGLRenderAPI : public RenderAPI
	{
	public:
		void Init() override;
		void SetClearColor(glm::vec4& color) override;
		void Clear() override;
		void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetPolygonModel(Polygon pol) override;

		void EnableDepthTesting(bool enabled) override;
		bool IsDepthTestingEnabled() override;

		void DrawElements(Shr<VertexArray>& vertexArray, uint32_t elementCount) override;
		
		void DrawLines(Shr<VertexArray>& vertexArray, uint32_t vertexCount, float thickness) override;
		void SetLineWidth(float thickness) override;

	private:
		bool depthTestingEnabled = false;
	};
	
}


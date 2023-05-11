#pragma once
#include "_Core.h"
#include "utility.h"

#include "renderer/VertexArray.h"
#include "renderer/RenderAPI.h"
#include "renderer/FrameBuffer.h"

namespace core {
	class RenderCommand
	{
	private:
		static Shr<RenderAPI> rendererAPI;
		static Shr<Framebuffer> framebuffer;

	public:
		static void Init();

		inline static void ClearColor(glm::vec4& color)
		{
			rendererAPI->SetClearColor(color);
		}

		static void Clear();

		static void ClearFramebuffer();

		inline static void DrawElements(Shr<VertexArray>& vertexArray, uint32_t elementCount)
		{
			rendererAPI->DrawElements(vertexArray, elementCount);
		}

		inline static void DrawLines(Shr<VertexArray>& vertexArray, uint32_t vertexCount, float thickness)
		{
			rendererAPI->DrawLines(vertexArray, vertexCount, thickness);
		}

		inline static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			rendererAPI->SetViewPort(x, y, width, height);
		}

		inline static void SetLineThickness(float width)
		{
			rendererAPI->SetLineWidth(width);
		}

		static void DrawIndexed(const Shr<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			rendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void SetPolygonModel(Polygon pol)
		{
			rendererAPI->SetPolygonModel(pol);
		}

		inline static Shr<Framebuffer> GetFramebuffer()
		{
			return framebuffer;
		}
	};

}


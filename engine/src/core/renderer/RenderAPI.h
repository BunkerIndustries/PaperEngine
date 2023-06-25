#pragma once
#include "Engine.h"
#include "utility.h"

#include "renderer/VertexArray.h"

namespace ppr {

	enum class Polygon
	{
		OFF = 0,
		LINE,
		POINT
	};

	class RenderAPI
	{
	public:
		enum API
		{
			NONE = 0,
			OPENGL,
			VULKAN
		};

		

		virtual ~RenderAPI() = default;

		virtual void Init() = 0;

		virtual void SetClearColor(glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void SetPolygonModel(Polygon pol) = 0;

		virtual void EnableDepthTesting(bool enable) = 0;
		virtual bool IsDepthTestingEnabled() = 0;

		virtual void DrawElements(Shr<VertexArray>& vertexArray, uint32_t elementCount = 0) = 0;

		virtual void DrawLines(Shr<VertexArray>& vertexArray, uint32_t vertexCount, float thicknesss) = 0;
		virtual void SetLineWidth(float thickness) = 0;

		static Shr<RenderAPI> CreateAPI();

		static API GetAPI() { return api; }

	private:
		static API api;

	};

}


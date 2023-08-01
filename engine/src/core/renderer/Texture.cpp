#include "Engine.h"
#include "utility.h"
#include "renderer/Texture.h"

#include "OpenGL/OpenGLTexture.h"

#include "renderer/RenderAPI.h"

namespace Paper {

    Shr<Texture> Texture::CreateTexture(std::filesystem::path filePath, std::string name)
    {
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::NONE: CORE_ASSERT(false, "'NONE' is a non valid API"); return nullptr;
		case RenderAPI::OPENGL: return MakeShr<OpenGLTexture>(filePath, name);
		case RenderAPI::VULKAN: CORE_ASSERT(false, "'VULKAN' is currently a not supportet API"); return nullptr;;
		}

		CORE_ASSERT(false, "");
		return nullptr;
    }


	Shr<Texture> Texture::CreateTexture(TextureSpecification specification)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::NONE: CORE_ASSERT(false, "'NONE' is a non valid API"); return nullptr;
		case RenderAPI::OPENGL: return MakeShr<OpenGLTexture>(specification);
		case RenderAPI::VULKAN: CORE_ASSERT(false, "'VULKAN' is currently a not supportet API"); return nullptr;;
		}

		CORE_ASSERT(false, "");
		return nullptr;
	}
}
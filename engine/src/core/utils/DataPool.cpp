#include "Engine.h"

#include "utils/DataPool.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/Font.h"

namespace Paper {

    // declare static maps in order to find a reference
    Map DataPool::dataPool;

    Shr<Shader> DataPool::GetShader(std::string shaderName) {
        // set default path
        std::string path = "resources/shaders/" + shaderName + ".glsl";
        std::string shader_id = "shader_" + shaderName;

        Map::iterator it = dataPool.find(shader_id);

        std::shared_ptr<Shader> shader;

        if (it == dataPool.end())
        {
            shader = Shader::CreateShader(path);
            dataPool.emplace(shader_id, shader);
        }
        else
        {
            shader = std::static_pointer_cast<Shader>(it->second);
        }

        return shader;
    }

    Shr<Texture> DataPool::GetAssetTexture(std::string textureName, bool wholePath) {
        // set default path
        const std::string path = wholePath ? textureName : "assets/textures/" + textureName;

        std::string texture_id = "assettexture_" + textureName;

        Map::iterator it = dataPool.find(texture_id);

        std::shared_ptr<Texture> texture;

        if (it == dataPool.end())
        {
            texture = Texture::CreateTexture(path, textureName);
            dataPool.emplace(texture_id, texture);
        }
        else
        {
            texture = std::static_pointer_cast<Texture>(it->second);
        }

        return texture;
    }

    Shr<Texture> DataPool::GetTexture(std::string textureName, bool wholePath) {
        // set default path
        const std::string path = wholePath ? textureName : "resources/textures/" + textureName;

        std::string texture_id = "recourcetexture_" + textureName;

        Map::iterator it = dataPool.find(texture_id);

        std::shared_ptr<Texture> texture;

        if (it == dataPool.end())
        {
            texture = Texture::CreateTexture(path, textureName);
            dataPool.emplace(texture_id, texture);
        }
        else
        {
            texture = std::static_pointer_cast<Texture>(it->second);
        }

        return texture;
    }

    Shr<Font> DataPool::GetFont(std::string fontName) {
        // set default path
        const std::string path = "resources/fonts/" + fontName;

        std::string fontID = "font_" + fontName;

        const auto it = dataPool.find(fontID);

        std::shared_ptr<Font> font;

        if (it == dataPool.end())
        {
            
            font = MakeShr<Font>(path);
            dataPool.emplace(fontID, font);
        }
        else
        {
            font = std::static_pointer_cast<Font>(it->second);
        }

        return font;
    }

    void DataPool::ErasePool()
    {
        dataPool.clear();
    }


    int ProjectionModeToInt(const ProjectionMode& mode)
    {
	    switch (mode)
	    {
	    case ProjectionMode::PERSPECTIVE:
            return 0;
	    case ProjectionMode::ORTHOGRAPHIC:
            return 1;
	    case ProjectionMode::SCREEN:
            return 2;
	    }
        return -1;
    }

}

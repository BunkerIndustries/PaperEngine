#include "Engine.h"

#include "renderer/Renderer2D.h"
#include "renderer/RenderCommand.h"
#include "utils/DataPool.h"
#include "renderer/Shader.h"
#include "generic/Application.h"
#include "imgui/ImGuiLayer.h"

namespace engine {

    struct EdgeVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoords;
        float tilingFactor;
        int texIndex;
        int projectionMode;
        int coreID;
        int uiID;
        int alphaCoreID;
    };

    struct LineVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        int projectionMode;
        int coreID;
        int uiID;
    };

    struct CircleVertex
    {
        glm::vec3 worldPos;
        glm::vec2 localPos;

        glm::vec2 texCoords;
        float tilingFactor;
        int texIndex;

        glm::vec4 color;
        float thickness;
        float fade;

        int projectionMode;
        int coreID;
        int alphaCoreID;
    };

    struct TextVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;

        int projectionMode;
        int coreID;
        int uiID;
        int alphaCoreID;
    };

    

    struct RenderData2D
    {
        static constexpr uint32_t MAX_VERTICES = 40000;
        static constexpr uint32_t MAX_ELEMENTS = 60000;
        static constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

        Shr<Shader> edgeGeometryShader;
        Shr<Shader> lineGeometryShader;
        Shr<Shader> circleGeometryShader;
        Shr<Shader> textShader;

        Shr<VertexArray> rectangleVertexArray;
        Shr<VertexBuffer> rectangleVertexBuffer;

        Shr<VertexArray> lineVertexArray;
        Shr<VertexBuffer> lineVertexBuffer;

        Shr<VertexArray> triangleVertexArray;
        Shr<VertexBuffer> triangleVertexBuffer;

        Shr<VertexArray> circleVertexArray;
        Shr<VertexBuffer> circleVertexBuffer;

        Shr<VertexArray> textVertexArray;
        Shr<VertexBuffer> textVertexBuffer;
        
        uint32_t rectangleElementCount = 0;
        EdgeVertex* rectangleVertexBufferBase = nullptr;
        EdgeVertex* rectangleVertexBufferPtr = nullptr;

        uint32_t triangleElementCount = 0;
        EdgeVertex* triangleVertexBufferBase = nullptr;
        EdgeVertex* triangleVertexBufferPtr = nullptr;

        uint32_t lineElementCount = 0;
        LineVertex* lineVertexBufferBase = nullptr;
        LineVertex* lineVertexBufferPtr = nullptr;

        uint32_t circleElementCount = 0;
        CircleVertex* circleVertexBufferBase = nullptr;
        CircleVertex* circleVertexBufferPtr = nullptr;

        uint32_t textElementCount = 0;
        TextVertex* textVertexBufferBase = nullptr;
        TextVertex* textVertexBufferPtr = nullptr;

        std::array<Shr<Texture>, MAX_TEXTURE_SLOTS> rectangleTextureSlots;
        uint32_t rectangleTextureSlotIndex = 0;

        std::array<Shr<Texture>, MAX_TEXTURE_SLOTS> triangleTextureSlots;
        uint32_t triangleTextureSlotIndex = 0;

        std::array<Shr<Texture>, MAX_TEXTURE_SLOTS> circleTextureSlots;
        uint32_t circleTextureSlotIndex = 0;

        float lineWidth = 1.0f;

        glm::vec4 rectangleVertexData[4];
        glm::vec4 triangleVertexData[3];

        Shr<Texture> fontAtlasTexture;

    };

    static RenderData2D data;
    static int texSlots[data.MAX_TEXTURE_SLOTS - 1];

    void Renderer2D::Init()
    {
        BufferLayout edgeGeometryLayout = {
            { GLSLDataType::FLOAT3, "aPos" },
            { GLSLDataType::FLOAT4, "aColor" },

            { GLSLDataType::FLOAT2, "aTexCoord" },
            { GLSLDataType::FLOAT , "aTilingFactor"},
            { GLSLDataType::INT , "aTexID" },

			{ GLSLDataType::INT , "aProjectionMode" },
            { GLSLDataType::INT , "aCoreID" },
            { GLSLDataType::INT , "aUIID" },
			{ GLSLDataType::INT , "aAlphaCoreID" }
        };

        BufferLayout lineGeometryLayout = {
            { GLSLDataType::FLOAT3, "aPos" },
            { GLSLDataType::FLOAT4, "aColor" },

			{ GLSLDataType::INT , "aProjectionMode" },
            { GLSLDataType::INT , "aCoreID" },
            { GLSLDataType::INT , "aUIID" }
        };

        BufferLayout circleGeometryLayout = {
            { GLSLDataType::FLOAT3, "aWorldPos" },
            { GLSLDataType::FLOAT2, "aLocalPos" },

			{ GLSLDataType::FLOAT2, "aTexCoord" },
			{ GLSLDataType::FLOAT , "aTilingFactor"},
            { GLSLDataType::INT , "aTexID" },

            { GLSLDataType::FLOAT4, "aColor" },
            { GLSLDataType::FLOAT,  "aThickness" },
            { GLSLDataType::FLOAT,  "aFade" },

			{ GLSLDataType::INT , "aProjectionMode" },
            { GLSLDataType::INT, "aCoreID" },
            { GLSLDataType::INT , "aAlphaCoreID" }
        };

        BufferLayout textLayout
        {
            { GLSLDataType::FLOAT3, "aPos" },
            { GLSLDataType::FLOAT4, "aColor" },
            { GLSLDataType::FLOAT2, "aTexCoord" },

            { GLSLDataType::INT ,   "aProjectionMode" },
            { GLSLDataType::INT,    "aCoreID" },
            { GLSLDataType::INT , "aUIID" },
            { GLSLDataType::INT , "aAlphaCoreID" }
        };

        data.edgeGeometryShader = DataPool::GetShader("EdgeGeometryShader_2D");
        data.edgeGeometryShader->Compile();

        data.lineGeometryShader = DataPool::GetShader("LineGeometryShader_2D");
        data.lineGeometryShader->Compile();

        data.circleGeometryShader = DataPool::GetShader("CircleGeometryShader_2D");
        data.circleGeometryShader->Compile();

        data.textShader = DataPool::GetShader("TextShader_2D");
        data.textShader->Compile();

        data.rectangleVertexArray = VertexArray::CreateArray();
        data.rectangleVertexBuffer = VertexBuffer::CreateBuffer(edgeGeometryLayout, data.MAX_VERTICES * sizeof(EdgeVertex));
        data.rectangleVertexArray->SetVertexBuffer(data.rectangleVertexBuffer);
        
        data.triangleVertexArray = VertexArray::CreateArray();
        data.triangleVertexBuffer = VertexBuffer::CreateBuffer(edgeGeometryLayout, data.MAX_VERTICES * sizeof(EdgeVertex));
        data.triangleVertexArray->SetVertexBuffer(data.triangleVertexBuffer);

        data.circleVertexArray = VertexArray::CreateArray();
        data.circleVertexBuffer = VertexBuffer::CreateBuffer(circleGeometryLayout, data.MAX_VERTICES * sizeof(CircleVertex));
        data.circleVertexArray->SetVertexBuffer(data.circleVertexBuffer);

        data.lineVertexArray = VertexArray::CreateArray();
        data.lineVertexBuffer = VertexBuffer::CreateBuffer(lineGeometryLayout, data.MAX_VERTICES * sizeof(LineVertex));
        data.lineVertexArray->SetVertexBuffer(data.lineVertexBuffer);

        data.textVertexArray = VertexArray::CreateArray();
        data.textVertexBuffer = VertexBuffer::CreateBuffer(textLayout, data.MAX_VERTICES * sizeof(TextVertex));
        data.textVertexArray->SetVertexBuffer(data.textVertexBuffer);

        

        uint32_t* rectangleElements = new uint32_t[data.MAX_ELEMENTS];

        int offsetRectangle = 0;
        for (int i = 0; i < data.MAX_ELEMENTS; i += 6)
        {
            // first triangle
            rectangleElements[i + 0] = offsetRectangle + 0;
            rectangleElements[i + 1] = offsetRectangle + 1;
            rectangleElements[i + 2] = offsetRectangle + 2;
            // second triangle               
            rectangleElements[i + 3] = offsetRectangle + 2;
            rectangleElements[i + 4] = offsetRectangle + 3;
            rectangleElements[i + 5] = offsetRectangle + 0;

            offsetRectangle += 4;
        }

        Shr<ElementBuffer> rectangleElementbuffer = ElementBuffer::CreateBuffer(rectangleElements, data.MAX_ELEMENTS);
        data.rectangleVertexArray->SetElementBuffer(rectangleElementbuffer);
        delete[] rectangleElements;

        uint32_t* triangleElements = new uint32_t[data.MAX_ELEMENTS];
        
        int offsetTriangle = 0;
        for (int i = 0; i < data.MAX_ELEMENTS; i += 3)
        {
            triangleElements[i + 0] = offsetTriangle + 0;
            triangleElements[i + 1] = offsetTriangle + 1;
            triangleElements[i + 2] = offsetTriangle + 2;
        
            offsetTriangle += 3;
        }
        
        Shr<ElementBuffer> triangleElementbuffer = ElementBuffer::CreateBuffer(triangleElements, data.MAX_ELEMENTS);
        data.triangleVertexArray->SetElementBuffer(triangleElementbuffer);
        delete[] triangleElements;

        data.textVertexArray->SetElementBuffer(rectangleElementbuffer);
        data.circleVertexArray->SetElementBuffer(rectangleElementbuffer);

        data.rectangleVertexData[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
        data.rectangleVertexData[1] = glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f);
        data.rectangleVertexData[2] = glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f);
        data.rectangleVertexData[3] = glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f);

        data.triangleVertexData[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
        data.triangleVertexData[1] = glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f);
        data.triangleVertexData[2] = glm::vec4( 0.0f,  0.5f, 0.0f, 1.0f);

        data.rectangleVertexBufferBase = new EdgeVertex[data.MAX_VERTICES];
        data.triangleVertexBufferBase = new EdgeVertex[data.MAX_VERTICES];
        data.circleVertexBufferBase = new CircleVertex[data.MAX_VERTICES];
        data.lineVertexBufferBase = new LineVertex[data.MAX_VERTICES];
        data.textVertexBufferBase = new TextVertex[data.MAX_VERTICES];

        for (uint32_t i = 0; i < data.MAX_TEXTURE_SLOTS - 1; i++)
        {
            texSlots[i] = i;
        }


    }

    void Renderer2D::Shutdown()
    {
        delete[] data.rectangleVertexBufferBase;
        delete[] data.triangleVertexBufferBase;
        delete[] data.lineVertexBufferBase;
        delete[] data.textVertexBufferBase;
     }

    void Renderer2D::ResizeWindow(uint32_t width, uint32_t height)
    {
        
    }

    void Renderer2D::BeginRender()
    {
        RenderCommand::EnableDepthTesting(false);

        StartBatch();
    }

    void Renderer2D::EndRender()
    {
        Render(ALL);
    }

    void Renderer2D::StartBatch()
    {
        data.rectangleElementCount = 0;
        data.rectangleVertexBufferPtr = data.rectangleVertexBufferBase;
        data.rectangleTextureSlotIndex = 0;

        data.triangleElementCount = 0;
        data.triangleVertexBufferPtr = data.triangleVertexBufferBase;
        data.triangleTextureSlotIndex = 0;

        data.lineElementCount = 0;
        data.lineVertexBufferPtr = data.lineVertexBufferBase;

        data.circleElementCount = 0;
        data.circleVertexBufferPtr = data.circleVertexBufferBase;
        data.circleTextureSlotIndex = 0;

        data.textElementCount = 0;
        data.textVertexBufferPtr = data.textVertexBufferBase;
    }

    void Renderer2D::NextBatch(RenderTarget2D target)
    {
        Render(target);
        StartBatch();
    }

    void Renderer2D::Render(RenderTarget2D target)
    {
        const SharedRenderData& sharedData = RenderCommand::sharedData;
        sharedData.cameraUniformBuffer->Bind();

	    if (data.rectangleElementCount && (target == RECTANGLE || target == ALL))
	    {
            const uint32_t dataSize = (uint32_t)((uint8_t*)data.rectangleVertexBufferPtr - (uint8_t*)data.rectangleVertexBufferBase);
            data.rectangleVertexBuffer->AddData(data.rectangleVertexBufferBase, dataSize);
            RenderCommand::GetStats().dataSize += dataSize;

            //bind textures
            for (uint32_t i = 0; i < data.rectangleTextureSlotIndex; i++)
                data.rectangleTextureSlots[i]->Bind(i);

            data.edgeGeometryShader->Bind();
            data.edgeGeometryShader->UploadIntArray("uTexture", data.MAX_TEXTURE_SLOTS - 1, texSlots);
            RenderCommand::DrawElements(data.rectangleVertexArray, data.rectangleElementCount);
            data.edgeGeometryShader->Unbind();
            RenderCommand::GetStats().drawCalls++;

            //unbind textures
            for (uint32_t i = 0; i < data.rectangleTextureSlotIndex; i++)
                data.rectangleTextureSlots[i]->Unbind();
	    }

        if (data.triangleElementCount && (target == TRIANGLE || target == ALL))
        {
            const uint32_t dataSize = (uint32_t)((uint8_t*)data.triangleVertexBufferPtr - (uint8_t*)data.triangleVertexBufferBase);
            data.triangleVertexBuffer->AddData(data.triangleVertexBufferBase, dataSize);
            RenderCommand::GetStats().dataSize += dataSize;
        
            //bind textures
            for (uint32_t i = 0; i < data.triangleTextureSlotIndex; i++)
                data.triangleTextureSlots[i]->Bind(i);

            //data.framebuffer->BindAttachmentAsTexture(1, data.MAX_TEXTURE_SLOTS - 1);

            data.edgeGeometryShader->Bind();
            data.edgeGeometryShader->UploadIntArray("uTexture", data.MAX_TEXTURE_SLOTS, texSlots);
        	RenderCommand::DrawElements(data.triangleVertexArray, data.triangleElementCount);
            data.edgeGeometryShader->Unbind();
            RenderCommand::GetStats().drawCalls++;
        
            //unbind textures
            for (uint32_t i = 0; i < data.triangleTextureSlotIndex; i++)
                data.triangleTextureSlots[i]->Unbind();
        }

        if (data.circleElementCount && (target == CIRCLE || target == ALL))
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)data.circleVertexBufferPtr - (uint8_t*)data.circleVertexBufferBase);
            data.circleVertexBuffer->AddData(data.circleVertexBufferBase, dataSize);
            RenderCommand::GetStats().dataSize += dataSize;


            //bind textures
            for (uint32_t i = 0; i < data.circleTextureSlotIndex; i++)
                data.circleTextureSlots[i]->Bind(i);

            data.circleGeometryShader->Bind();
            data.circleGeometryShader->UploadIntArray("uTexture", data.MAX_TEXTURE_SLOTS, texSlots);
            RenderCommand::DrawElements(data.circleVertexArray, data.circleElementCount);
            data.circleGeometryShader->Unbind();
            RenderCommand::GetStats().drawCalls++;

            for (uint32_t i = 0; i < data.circleTextureSlotIndex; i++)
                data.circleTextureSlots[i]->Unbind();
        }

        if (data.lineElementCount && (target == LINE || target == ALL))
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)data.lineVertexBufferPtr - (uint8_t*)data.lineVertexBufferBase);
            data.lineVertexBuffer->AddData(data.lineVertexBufferBase, dataSize);
            RenderCommand::GetStats().dataSize += dataSize;

            
            data.lineGeometryShader->Bind();
            RenderCommand::SetLineThickness(data.lineWidth);
            RenderCommand::DrawLines(data.lineVertexArray, data.lineElementCount, data.lineWidth);
            data.lineGeometryShader->Unbind();
            RenderCommand::GetStats().drawCalls++;
        }

        if (data.textElementCount && (target == TEXT || target == ALL))
        {
            uint32_t dataSize = (uint32_t)((uint8_t*)data.textVertexBufferPtr - (uint8_t*)data.textVertexBufferBase);
            data.textVertexBuffer->AddData(data.textVertexBufferBase, dataSize);
            RenderCommand::GetStats().dataSize += dataSize;


            data.fontAtlasTexture->Bind(0);

            data.textShader->Bind();
            RenderCommand::DrawElements(data.textVertexArray, data.textElementCount);
            RenderCommand::GetStats().drawCalls++;
        }
    }

    void Renderer2D::DrawRectangle(const EdgeRenderData& renderData)
    {
        const uint32_t rectangleVertexCount = 4;

        glm::mat4 transform = renderData.transform.GetMatrix();

        if (data.rectangleElementCount >= data.MAX_ELEMENTS)
        {
            NextBatch(RECTANGLE);
        }

    	int texIndex = -1;
        if (renderData.texture != nullptr)
        {
            for (uint32_t i = 0; i < data.rectangleTextureSlotIndex; i++)
            {
                if (*data.rectangleTextureSlots[i] == *renderData.texture)
                {
                    texIndex = i;
                    break;
                }
            }

            if (texIndex == -1)
            {
                if (data.rectangleTextureSlotIndex >= data.MAX_TEXTURE_SLOTS)
                    NextBatch(RECTANGLE);

                texIndex = data.rectangleTextureSlotIndex;
                data.rectangleTextureSlots[data.rectangleTextureSlotIndex] = renderData.texture;
                data.rectangleTextureSlotIndex++;
            }
        }

        for (int i = 0; i < rectangleVertexCount; i++)
        {
            data.rectangleVertexBufferPtr->position = transform * data.rectangleVertexData[i];
            data.rectangleVertexBufferPtr->color = renderData.color;
            data.rectangleVertexBufferPtr->texCoords = renderData.texCoords[i];
            data.rectangleVertexBufferPtr->tilingFactor = renderData.tilingFactor;
            data.rectangleVertexBufferPtr->texIndex = texIndex;
            data.rectangleVertexBufferPtr->projectionMode = ProjectionModeToInt(renderData.mode);
            data.rectangleVertexBufferPtr->coreID = renderData.coreID;
            data.rectangleVertexBufferPtr->alphaCoreID = renderData.coreIDToAlphaPixels;
            data.rectangleVertexBufferPtr++;

            RenderCommand::GetStats().vertexCount++;
        }

        data.rectangleElementCount += 6;

        RenderCommand::GetStats().elementCount += 6;
        RenderCommand::GetStats().objectCount++;
    }

    void Renderer2D::DrawTriangle(const EdgeRenderData& renderData)
    {
        const uint32_t triangleVertexCount = 3;

        glm::mat4 transform = renderData.transform.GetMatrix();

        if (data.triangleElementCount >= data.MAX_ELEMENTS)
        {
            NextBatch(TRIANGLE);
        }

        int texIndex = -1;
        if (renderData.texture != nullptr)
        {
            for (uint32_t i = 0; i < data.triangleTextureSlotIndex; i++)
            {
                if (*data.triangleTextureSlots[i] == *renderData.texture)
                {
                    texIndex = i;
                    break;
                }
            }

            if (texIndex == -1)
            {
                if (data.triangleTextureSlotIndex >= data.MAX_TEXTURE_SLOTS)
                    NextBatch(TRIANGLE);

                texIndex = data.triangleTextureSlotIndex;
                data.triangleTextureSlots[data.triangleTextureSlotIndex] = renderData.texture;
                data.triangleTextureSlotIndex++;
            }
        }

        for (int i = 0; i < triangleVertexCount; i++)
        {
            data.triangleVertexBufferPtr->position = transform * data.triangleVertexData[i];
            data.triangleVertexBufferPtr->color = renderData.color;
            data.triangleVertexBufferPtr->texCoords = renderData.texCoords[i];
            data.triangleVertexBufferPtr->tilingFactor = renderData.tilingFactor;
            data.triangleVertexBufferPtr->texIndex = texIndex;
            data.triangleVertexBufferPtr->projectionMode = ProjectionModeToInt(renderData.mode);
            data.triangleVertexBufferPtr->coreID = renderData.coreID;
            data.triangleVertexBufferPtr->alphaCoreID = renderData.coreIDToAlphaPixels;
            data.triangleVertexBufferPtr++;

            RenderCommand::GetStats().vertexCount++;
        }

        data.triangleElementCount += 3;

        RenderCommand::GetStats().elementCount += 3;
        RenderCommand::GetStats().objectCount++;
    }


    void Renderer2D::DrawLine(const LineRenderData& renderData)
    {
        data.lineVertexBufferPtr->position = renderData.point0;
        data.lineVertexBufferPtr->color = renderData.color;
        data.lineVertexBufferPtr->projectionMode = ProjectionModeToInt(renderData.mode);
        data.lineVertexBufferPtr->coreID = renderData.coreID;
        data.lineVertexBufferPtr++;

        RenderCommand::GetStats().vertexCount++;
        
        data.lineVertexBufferPtr->position = renderData.point1;
        data.lineVertexBufferPtr->color = renderData.color;
        data.lineVertexBufferPtr->projectionMode = ProjectionModeToInt(renderData.mode);
        data.lineVertexBufferPtr->coreID = renderData.coreID;
        data.lineVertexBufferPtr++;

        RenderCommand::GetStats().vertexCount++;

        data.lineElementCount += 1;

        RenderCommand::GetStats().elementCount += 1;
        RenderCommand::GetStats().objectCount++;

        data.lineWidth = renderData.thickness;
        NextBatch(LINE);
    }

    void Renderer2D::DrawCircle(const CircleRenderData& renderData)
    {
        const uint32_t circleVertexCount = 4;

        glm::mat4 transform = renderData.transform.GetMatrix();

        if (data.circleElementCount >= data.MAX_ELEMENTS)
        {
            NextBatch(CIRCLE);
        }

        int texIndex = -1;
        if (renderData.texture != nullptr)
        {
            for (uint32_t i = 0; i < data.circleTextureSlotIndex; i++)
            {
                if (*data.circleTextureSlots[i] == *renderData.texture)
                {
                    texIndex = i;
                    break;
                }
            }

            if (texIndex == -1)
            {
                if (data.circleTextureSlotIndex >= data.MAX_TEXTURE_SLOTS)
                    NextBatch(CIRCLE);

                texIndex = data.circleTextureSlotIndex;
                data.circleTextureSlots[data.circleTextureSlotIndex] = renderData.texture;
                data.circleTextureSlotIndex++;
            }
        }

        for (int i = 0; i < circleVertexCount; i++)
        {
            data.circleVertexBufferPtr->worldPos = transform * data.rectangleVertexData[i];
            data.circleVertexBufferPtr->localPos = data.rectangleVertexData[i] * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)); // just multiplying x and y by 2
            data.circleVertexBufferPtr->texCoords = renderData.texCoords[i];
            data.circleVertexBufferPtr->tilingFactor = renderData.tilingFactor;
            data.circleVertexBufferPtr->texIndex = texIndex;
            data.circleVertexBufferPtr->color = renderData.color;
            data.circleVertexBufferPtr->thickness = renderData.thickness;
            data.circleVertexBufferPtr->fade = renderData.fade;
            data.circleVertexBufferPtr->projectionMode = ProjectionModeToInt(renderData.mode);
            data.circleVertexBufferPtr->coreID = renderData.coreID;
            data.circleVertexBufferPtr->alphaCoreID = renderData.coreIDToAlphaPixels;
            data.circleVertexBufferPtr++;
            RenderCommand::GetStats().vertexCount++;
        }

        data.circleElementCount += 6;

        RenderCommand::GetStats().elementCount += 6;
        RenderCommand::GetStats().objectCount++;
    }


    void Renderer2D::DrawString(const TextRenderData& renderData)
    {
        if (data.textElementCount >= data.MAX_ELEMENTS)
        {
            NextBatch(TEXT);
        }

        std::string string = renderData.string;
        auto& font = renderData.font;

        size_t dataSize = renderData.string.length() * 4;
        size_t index = 0;

        float leftVertex = 0.0f;
        float rightVertex = 0.0f;
        float highestVertex = 0.0f;
        float lowestVertex = 0.0f;

        std::vector<glm::vec4> vertexData;
        vertexData.resize(dataSize);
        std::vector<glm::vec2> texCoordData;
        texCoordData.resize(dataSize);

        const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
        const auto& metrics = fontGeometry.getMetrics();
        Shr<Texture> fontAtlas = font->GetAtlasTexture();

        data.fontAtlasTexture = fontAtlas;

        double x = 0.0;
        double fsScale = 1.0;// / (metrics.ascenderY - metrics.descenderY);
        double y = 0.0;
        float lineHeightOffset = 0.0f;

        glm::mat4 transform = renderData.transform.GetMatrix();

        for (size_t i = 0; i < string.length(); i++)
        {
            char character = string[i];
            if (character == '\r')
                continue;

            if (character == '\n')
            {
                x = 0;
                y -= fsScale * metrics.lineHeight + lineHeightOffset;
                continue;
            }
            auto glyph = fontGeometry.getGlyph(character);
            if (!glyph)
                glyph = fontGeometry.getGlyph('?');
            if (!glyph)
                return;

            if (character == '\t')
                glyph = fontGeometry.getGlyph('   ');

            double al, ab, ar, at;
            glyph->getQuadAtlasBounds(al, ab, ar, at);
            glm::vec2 texCoordMin((float)al, (float)ab);
            glm::vec2 texCoordMax((float)ar, (float)at);

            double pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            glm::vec2 quadMin((float)pl, (float)pb);
            glm::vec2 quadMax((float)pr, (float)pt);

            quadMin *= fsScale, quadMax *= fsScale;
            quadMin += glm::vec2(x, y);
            quadMax += glm::vec2(x, y);

            float texelWidth = 1.0f / fontAtlas->GetWidth();
            float texelHeight = 1.0f / fontAtlas->GetHeight();
            texCoordMin *= glm::vec2(texelWidth, texelHeight);
            texCoordMax *= glm::vec2(texelWidth, texelHeight);


            //save data in buffers
            vertexData[index + 0] = glm::vec4(quadMin, 0.0f, 1.0f);
            vertexData[index + 1] = glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
            vertexData[index + 2] = glm::vec4(quadMax, 0.0f, 1.0f);
            vertexData[index + 3] = glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);

            texCoordData[index + 0] = texCoordMin;
            texCoordData[index + 1] = { texCoordMin.x, texCoordMax.y };
            texCoordData[index + 2] = texCoordMax;
            texCoordData[index + 3] = { texCoordMax.x, texCoordMin.y };

            index += 4;

            if (quadMax.y > highestVertex) highestVertex = quadMax.y;
            if (quadMin.y < lowestVertex) lowestVertex = quadMin.y;

        	if (quadMin.x < rightVertex) rightVertex = quadMin.x;
            if (quadMax.x > leftVertex) leftVertex = quadMax.x;

            if (i < string.size() - 1)
            {
                double advance = glyph->getAdvance();
                char nextCharacter = string[i + 1];
                fontGeometry.getAdvance(advance, character, nextCharacter);

                float kerningOffset = 0.0f;
                x += fsScale * advance + kerningOffset;
            }
        }

    	glm::vec4 stringSize((leftVertex - rightVertex) / 2, (highestVertex - lowestVertex) / 2 + lowestVertex, 0, 0); // idk what this is; tried until it worked

        for (size_t i = 0; i < dataSize; i++)
        {
            vertexData[i] -= stringSize;

            data.textVertexBufferPtr->position = transform * vertexData.at(i);
            data.textVertexBufferPtr->color = renderData.color;
            data.textVertexBufferPtr->texCoord = texCoordData.at(i);
            data.textVertexBufferPtr->projectionMode = ProjectionModeToInt(renderData.mode);
            data.textVertexBufferPtr->coreID = renderData.coreID;
            data.textVertexBufferPtr->alphaCoreID = renderData.coreIDToAlphaPixels;
            data.textVertexBufferPtr++;
            RenderCommand::GetStats().vertexCount++;

            
        }

        data.textElementCount += dataSize / 4 * 6;

        RenderCommand::GetStats().elementCount += dataSize / 4 * 6;
        RenderCommand::GetStats().objectCount += dataSize / 4;
    }
}

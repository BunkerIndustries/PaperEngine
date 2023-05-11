#include "_Core.h"
#include "utility.h"

#include "imgui/ImGuiLayer.h"

#include "generic/Application.h"
#include "renderer/RenderCommand.h"
#include "renderer/Renderer2D.h"
#include "renderer/FrameBuffer.h"
#include "ui/UIObject.h"

#include <GLFW/glfw3.h>



namespace core {

    static void darkmode(ImGuiIO& io, ImGuiStyle& style)
    {
        // enable dark mode
        ImGui::StyleColorsDark();

        auto& colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
        }
    }


    ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
    {
        this->viewportSize = glm::vec2();
    }

    ImGuiLayer::~ImGuiLayer()
    {
        Detach();
    }

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        //create context to OpenGL
        ImGui::CreateContext();
        ImPlot::CreateContext();

        //ImGui Config
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        darkmode(io, style);

        //fonts
        ImFontAtlas* fontAtlas = io.Fonts;
        ImFontConfig& fontConfig = ImFontConfig();

        //set default range (uft-8)
        fontConfig.GlyphRanges = fontAtlas->GetGlyphRangesDefault();

        fontAtlas->AddFontFromFileTTF("assets/fonts/mononoki.ttf", 18, &fontConfig);

        //any new fonts were added to the font pool
        fontConfig.MergeMode = true;

        //init backend
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)Application::GetWindow()->GetNativeWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin(const float dt)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(Application::GetWindow()->GetWidth(), Application::GetWindow()->GetHeight());
        io.DeltaTime = dt;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(Application::GetWindow()->GetWidth(), Application::GetWindow()->GetHeight());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* context_current = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(context_current);
        }
    }



    void ImGuiLayer::DockPanel(std::string name, ImGuiID dock_id)
    {
        if (dockPanelQueue.find(name) == dockPanelQueue.end())
        {
            dockPanelQueue.emplace(name, dock_id);
        }
    }

    void ImGuiLayer::Update(const float dt)
    {
        // Resize
        if (FramebufferSpecification spec = RenderCommand::GetFramebuffer()->GetSpecification();
            viewportSize.x > 0.0f && viewportSize.y > 0.0f && // zero sized framebuffer is invalid
            (spec.width != viewportSize.x || spec.height != viewportSize.y))
        {
            RenderCommand::GetFramebuffer()->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
        }

        if (Application::GetImGuiEnabled()) {
            mousePosViewportRelative = *(glm::vec2*)&ImGui::GetMousePos();
            mousePosViewportRelative.x -= viewportBounds[0].x;
            mousePosViewportRelative.y -= viewportBounds[0].y - 24;
            glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];

            mousePosViewportRelative.y = viewportSize.y - mousePosViewportRelative.y;

        }
        
    }

	void ImGuiLayer::Imgui(const float dt)
    {
        ImGuiDockNodeFlags dockflags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        window_flags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        ImGuiViewport& viewport = *ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport.Pos);
        ImGui::SetNextWindowSize(viewport.Size);
        ImGui::SetNextWindowViewport(viewport.ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("docking", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        dockspace_id = ImGui::GetID("dockspace");

        static bool initialized = false;
        if (!initialized)
        {
            initialized = true;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(Application::GetWindow()->GetWidth() + 500, Application::GetWindow()->GetHeight() + 500));

            dock_id_main = dockspace_id;
            dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.2f, nullptr, &dock_id_main);
            dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.2f, nullptr, &dock_id_main);
            dock_id_top = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Up, 0.2f, nullptr, &dock_id_main);
            dock_id_down = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.25f, nullptr, &dock_id_main);
            dock_id_right2 = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Left, 0.2f, nullptr, &dock_id_right);
            dock_id_left_bottom = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, nullptr, &dock_id_left);


            ImGui::DockBuilderFinish(dockspace_id);
        }
        for (auto [key, val] : dockPanelQueue)
        {
            ImGui::DockBuilderDockWindow(key.c_str(), val);
        }
        dockPanelQueue.clear();

        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockflags);
        ImGui::End();

        static bool first = true;
        ApplicationPanel(dt, first);
        CustomPanel(dt, first);
        LayerPanel(dt, first);
        ViewPortPanel(dt, first);
        InspectorPanel(dt, first);
        if (first) first = false;
    }

    void ImGuiLayer::OnEvent(Event& e)
    {
        if (!viewportFocused && Application::GetImGuiEnabled())
        {
            ImGuiIO& io = ImGui::GetIO();
            e.handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            e.handled |= e.IsInCategory(EventCategoryKeyBoard) & io.WantCaptureKeyboard;
        }
    }

    void ImGuiLayer::ApplicationPanel(const float dt, bool first)
    {
        const char* name = "Application: ";
        std::stringstream stream;
        if (first) 
            Application::GetImGuiLayer().DockPanel(name, Application::GetImGuiLayer().GetDockspaceRight());

        ImGui::Begin(name);

        static float time = 0;
        static float timehelper = -1;
        static float history = 5;
        static int flags = ImPlotAxisFlags_NoTickLabels;
        static ScrollingBuffer sbuff_dt(300);

        if (timehelper >= 0.016f || timehelper == -1) {
            timehelper = 0;
            sbuff_dt.AddPoint(time, 1000 * dt);
        }
        timehelper += dt;
        time += dt;

        if (ImGui::TreeNode("Time"))
        {
		    ImGui::SliderFloat("Time", &history, 1, 5, "%.1f");

		    stream << "ms per frame: " << 1000 * dt;
		    ImGui::Text(stream.str().c_str()); stream.str("");
		    if (ImPlot::BeginPlot("##ms_per_frame", ImVec2(-1, 100))) {
		        ImPlot::SetupAxes(NULL, NULL, flags, flags);
		        ImPlot::SetupAxisLimits(ImAxis_X1, time - history, time, ImGuiCond_Always);
		        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 20);
		        ImPlot::SetNextFillStyle(ImVec4(0.0f, 0.0f, 1.0f, -1.0f), 0.5f);
		        ImPlot::PlotLine("##ms", &sbuff_dt.Data[0].x, &sbuff_dt.Data[0].y, sbuff_dt.Data.size(), 0, sbuff_dt.Offset, 2 * sizeof(float));
		        ImPlot::EndPlot();
		    }

		    stream << "frames per sec: " << 1 / dt;
		    ImGui::Text(stream.str().c_str()); stream.str("");
		    
		    stream << "Frames rendered: " << Application::GetFramesRendered();
		    ImGui::BulletText(stream.str().c_str()); stream.str("");

		    static bool vsync = true;
		    ImGui::Checkbox("V-Sync", &vsync);
		    Application::GetWindow()->SetVSync(vsync);

		    

			ImGui::Text("");
            ImGui::TreePop();
        }


        if (ImGui::TreeNode("Render Stats"))
        {
            stream << "Draw calls: " << Renderer2D::GetStats().drawCalls;
            ImGui::BulletText(stream.str().c_str()); stream.str("");

            stream << "Object count: " << Renderer2D::GetStats().objectCount;
            ImGui::BulletText(stream.str().c_str()); stream.str("");

            stream << "Data size: " << Renderer2D::GetStats().dataSize << " Bytes";
            ImGui::BulletText(stream.str().c_str()); stream.str("");

            stream << "Vertex count: " << Renderer2D::GetStats().vertexCount;
            ImGui::BulletText(stream.str().c_str()); stream.str("");

            stream << "Indices count: " << Renderer2D::GetStats().elementCount;
            ImGui::BulletText(stream.str().c_str()); stream.str("");

            ImGui::Text("");

            stream << "Polygon Model: ";
            ImGui::Text(stream.str().c_str()); stream.str("");

            static int selected = 0;
            if (ImGui::Selectable("OFF", selected == 0))
                RenderCommand::SetPolygonModel(Polygon::OFF);
            if (ImGui::Selectable("LINE", selected == 2))
                RenderCommand::SetPolygonModel(Polygon::LINE);
            if (ImGui::Selectable("POINT", selected == 1))
                RenderCommand::SetPolygonModel(Polygon::POINT);

            

            ImGui::BeginDisabled();
            if (ImGui::TreeNode("Textures in use"))
            {
                /*
	            for (Shr<Texture> texture : Renderer2D::GetTexturesInUse())
	            {
                    
	            	ImGui::Selectable(texture->GetName().c_str(), false);
                    
	            }
                */
	            ImGui::Text("");
	            ImGui::TreePop();
            }
            ImGui::EndDisabled();


            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Camera"))
        {
            if (ImGui::Button("Reset Camera"))
            {
                Application::GetActiveScene()->GetCamera()->fov = 1.0f;
                Application::GetActiveScene()->GetCamera()->position = glm::vec3(0.0f, 0.0f, 5.0f);
            }

            glm::vec3 pos = Application::GetActiveScene()->GetCamera()->position;
            float arrayPos[]{ pos.x, pos.y, pos.z };
            ImGui::DragFloat3("XYZ", arrayPos, 0.5f);
            Application::GetActiveScene()->GetCamera()->position = glm::vec3(arrayPos[0], arrayPos[1], arrayPos[2]);

            float fov = Application::GetActiveScene()->GetCamera()->fov;
            ImGui::DragFloat("FOV", &fov, 0.5f);
            Application::GetActiveScene()->GetCamera()->fov = fov;

            ImGui::TreePop();
        }

        ImGui::End();
    }


    void ImGuiLayer::CustomPanel(const float dt, bool first)
    {
        const char* name = "Track Variables: ";
        std::stringstream stream;

        if (first)
			Application::GetImGuiLayer().DockPanel(name, Application::GetImGuiLayer().GetDockspaceBottom());

        ImGui::Begin(name);
        ImGui::End();
    }

    void ImGuiLayer::LayerPanel(const float dt, bool first)
    {
        const char* name = "Layers: ";
        std::stringstream stream;

        if (first)
			Application::GetImGuiLayer().DockPanel(name, Application::GetImGuiLayer().GetDockspaceLeft());

        ImGui::Begin(name);

        for (Layer* layer : Application::GetLayerStack()) 
        {
            if (layer == &Application::GetImGuiLayer()) continue;
            std::vector<Entity*> gameobjects = layer->GetEntitys();
            std::vector<UIObject*> uiObjects = layer->GetUIObjects();

            if (ImGui::TreeNode(layer->GetName().c_str())) 
            {
                if (ImGui::TreeNode("UIObjects: "))
                {
                    for (int i = 0; i < uiObjects.size(); i++)
                    {
                        if (ImGui::Selectable((uiObjects[i]->GetName() + std::string("##" + std::to_string(i))).c_str(), uiObjects[i] == selectedObject)) {
                            selectedObject = uiObjects[i];
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Entitys: "))
                {
                    for (int i = 0; i < gameobjects.size(); i++)
                    {
                        if (ImGui::Selectable((gameobjects[i]->GetName() + std::string(" (ObjectID = " + std::to_string(gameobjects[i]->GetCoreID()) + std::string(")")) + std::string("##" + std::to_string(i))).c_str(), gameobjects[i] == selectedObject)) {
                            selectedObject = gameobjects[i];
                        }
                    }
                    ImGui::TreePop();
                }

                ImGui::Text("");
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    void ImGuiLayer::InspectorPanel(const float dt, bool first) {
        const char* name = "Inspector: ";
        std::stringstream stream;

        if (first)
            Application::GetImGuiLayer().DockPanel(name, Application::GetImGuiLayer().GetDockspaceLeftBottom());

        ImGui::Begin(name);
        if (dynamic_cast<Entity*>(selectedObject) != nullptr) {
            dynamic_cast<Entity*>(selectedObject)->Imgui(dt);
        }
        ImGui::End();
    }

    void ImGuiLayer::ViewPortPanel(const float dt, bool first)
    {
        const char* name = "ViewPort: ";

        if (first)
			Application::GetImGuiLayer().DockPanel(name, Application::GetImGuiLayer().GetDockspaceMain());

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(name);
        //RenderCommand::GetFramebuffer()->Bind();

        auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto viewportOffset = ImGui::GetWindowPos();
        viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
        viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

        viewportFocused = ImGui::IsWindowFocused();
        viewportHovered = ImGui::IsWindowHovered();

        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
    	viewportSize = { viewport_panel_size.x, viewport_panel_size.y };

        uint32_t textureID = RenderCommand::GetFramebuffer()->GetColorID(0);
        ImGui::Image((void*)textureID, ImVec2(viewportSize.x, viewportSize.y), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::End();
        ImGui::PopStyleVar();
        
    }

    void ImGuiLayer::ScreenPanel()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        window_flags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;// | ImGuiWindowFlags_MenuBar;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGuiViewport& viewport = *ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport.Pos);
        ImGui::SetNextWindowSize(viewport.Size);
        ImGui::SetNextWindowViewport(viewport.ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin(" ", nullptr, window_flags);
        ImGui::PopStyleVar(3);

    	RenderCommand::GetFramebuffer()->Bind();
        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        viewportSize = { viewport_panel_size.x, viewport_panel_size.y };
        
        uint32_t textureID = RenderCommand::GetFramebuffer()->GetColorID(0);
        ImGui::Image((void*)textureID, ImVec2(viewportSize.x, viewportSize.y), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        
        RenderCommand::GetFramebuffer()->Unbind();
        
        ImGui::End();
    }

}
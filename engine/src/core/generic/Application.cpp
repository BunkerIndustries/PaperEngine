#include "Engine.h"

#include "generic/Application.h"

#include "event/Input.h"
#include "imgui/ImGuiLayer.h"
#include "renderer/RenderCommand.h"
#include "scripting/ScriptEngine.h"
#include "utils/DataPool.h"

namespace Paper {

	Application* Application::instance;

	Application::Application(const WindowProps& props)
	{
		Log::Init();
 
		instance = this;

		//Core::Init();

		window = Window::Create(props);
		SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		RenderCommand::Init();
		ScriptEngine::Init();

		imguiLayer = new ImGuiLayer();
	}

	Application::~Application()
	{
		ScriptEngine::Shutdown(!this->restart);
		RenderCommand::Shutdown();
		DataPool::ErasePool();
		Log::Shutdown();
	}


	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = layerStack.end(); it != layerStack.begin(); )
		{
			if (event.handled)
				break;
			(*--it)->OnEvent(event);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Exit();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		resizing = true;
		RenderCommand::SetViewPort(0, 0, e.getWidth(), e.getHeight());
		return false;
	}

	void Application::ExecuteMainThreadQueues()
	{
		std::scoped_lock<std::mutex> lock(GetInstance()->mainThreadQueueMutex);
		for (std::function<void()> func : mainThreadQueue)
		{
			func();
		}
		mainThreadQueue.clear();
	}

	void Application::Run() 
	{
		restart = false;
		AddOverlay(imguiLayer);
		Init();

		// start of the calculations
		float begin_time = window->GetTime();
		dt = 0.0167f;
		bool starting = true;

		while (gameRunning)
		{
			if (!starting) {}
				window->PollEvents();

			ExecuteMainThreadQueues();
			
			imguiLayer->Begin(dt);

			for (Layer* layer : layerStack)
			{
				if (layer->IsAttached())
					layer->Update(dt);
			}

			for (Layer* layer : layerStack) {
				layer->Imgui(dt);
			}

			imguiLayer->End();

			window->SwapBuffers();

			Input::Update();

			resizing = false;
			starting = false;

			dt = window->GetTime() - begin_time;
			begin_time = window->GetTime();
		}

		LOG_CORE_WARN("Reached end of game function. Shutting down.");
	}

	void Application::ChangeScene(Scene* new_scene)
	{
		GetInstance()->queuedScene = new_scene;
	}

	void Application::Exit(bool restart)
	{
		this->gameRunning = false;
		this->restart = restart;
	}

	void Application::RemoveAllLayers()
	{
		for (auto& layer : layerStack)
		{
			if (layer->IsAttached())
			{
				if (layer->GetOverlayStatus())
					RemoveOverlay(layer);
				else
					RemoveLayer(layer);
			}
		}
	}

	void Application::AddLayer(Layer* layer)
	{
		layerStack.AddLayer(layer);
		layer->Attach();
	}

	void Application::AddOverlay(Layer* layer)
	{
		layerStack.AddOverlay(layer);
		layer->Attach();
	}

	void Application::RemoveLayer(Layer* layer)
	{
		layer->Detach();
		layerStack.RemoveLayer(layer);
	}

	void Application::RemoveOverlay(Layer* layer)
	{
		layer->Detach();
		layerStack.RemoveOverlay(layer);
	}

	void Application::SubmitToMainThread(const std::function<void()>& func)
	{
		std::scoped_lock<std::mutex> lock(GetInstance()->mainThreadQueueMutex);
		GetInstance()->mainThreadQueue.push_back(func);
	}
}

#pragma once
#include "Engine.h"
#include "utility.h"

#include "event/Event.h"

namespace ppr {

    enum FRAMEWORK
    {
        NONE = 0,
        GLFW
    };

    struct WindowProps
    {
        std::string title;
        unsigned int width, height;

        WindowProps(std::string title = "Window", unsigned int width = 1280, unsigned int height = 720)
	        : title(title), width(width), height(height) { }
    };

    class Window {
    public:

        virtual ~Window() = default;

        virtual void PollEvents() = 0;
        virtual void SwapBuffers() = 0;

        virtual unsigned int GetWidth() const = 0;
        virtual unsigned int GetHeight() const = 0;

        virtual float GetTime() const = 0;

        virtual void SetEventCallback(const EventCallbackFunction& callback_function) = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;

        virtual void CursorEnabled(bool enabled) = 0;
        virtual bool IsCursorEnabled() const = 0;

        virtual void* GetNativeWindow() const = 0;

        static FRAMEWORK GetFramework() { return framework; }

    	static Shr<Window> Create(const WindowProps& windowProps = WindowProps());

    private:
        static FRAMEWORK framework;
    };
}
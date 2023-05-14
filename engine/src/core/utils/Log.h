#pragma once
#include "Engine.h"
#include "utility.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
namespace engine {
	class Log
	{
	private:
		static std::shared_ptr<spdlog::logger> coreLogger;
		static std::shared_ptr<spdlog::logger> clientLogger;

	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return clientLogger; }
	};
}

#ifdef BUILD_DEBUG

//Core Log
#define LOG_CORE_CRITICAL(...) ::engine::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define LOG_CORE_ERROR(...)	   ::engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_WARN(...)	   ::engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_TRACE(...)	   ::engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_DEBUG(...)	   ::engine::Log::GetCoreLogger()->debug(__VA_ARGS__)

//Client Log
#define LOG_CRITICAL(...) ::engine::Log::GetClientLogger()->critical(__VA_ARGS__)
#define LOG_ERROR(...)	  ::engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_WARN(...)	  ::engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_TRACE(...)	  ::engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_DEBUG(...)	  ::engine::Log::GetClientLogger()->debug(__VA_ARGS__)

#else 

//Core Log
#define LOG_CORE_CRITICAL(...)
#define LOG_CORE_ERROR(...)
#define LOG_CORE_WARN(...)
#define LOG_CORE_TRACE(...)
#define LOG_CORE_DEBUG(...)

//Client Log
#define LOG_CRITICAL(...)
#define LOG_ERROR(...)
#define LOG_WARN(...)
#define LOG_TRACE(...)
#define LOG_DEBUG(...)

#endif
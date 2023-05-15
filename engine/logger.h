#pragma once
#include <string_view>
#include "fmt/core.h"
#include "fmt/os.h"
#include "fmt/color.h"
#include <chrono>
#include <fmt/chrono.h>
#include <vulkan/vulkan.h>

#define LOG_FATAL(message,...) Logger::Get().log(LogLevel::Fatal,message, ##__VA_ARGS__);
#define LOG_ERROR(message,...) Logger::Get().log(LogLevel::Error,message, ##__VA_ARGS__);
#define LOG_INFO(message,...) Logger::Get().log(LogLevel::Info,message, ##__VA_ARGS__);
#define LOG_TRACE(message,...) Logger::Get().log(LogLevel::Trace,message, ##__VA_ARGS__);
#define LOG_WARNING(message,...) Logger::Get().log(LogLevel::Warning,message, ##__VA_ARGS__);
#define LOG_SUCCESS(message,...) Logger::Get().log(LogLevel::Success,message, ##__VA_ARGS__);

enum class LogLevel {
	Fatal,
	Error,
	Warning,
	Success,
	Info,
	Trace
};

template<>
struct fmt::formatter<VkResult>
{
	const char * get_result_string(VkResult result)
	{
		switch ((int)result) {
		case VK_SUCCESS:
			return "VK_SUCCESS";
		case VK_NOT_READY:
			return "VK_NOT_READY";
		case VK_TIMEOUT:
			return "VK_TIMEOUT";
		case VK_EVENT_SET:
			return "VK_EVENT_SET";
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET";
		case VK_INCOMPLETE:
			return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:
			return "VK_ERROR_UNKNOWN";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		case VK_ERROR_FRAGMENTATION:
			return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:
			return "VK_ERROR_INVALID_SHADER_NV";
	#if VK_HEADER_VERSION >= 135 && VK_HEADER_VERSION < 162
		case VK_ERROR_INCOMPATIBLE_VERSION_KHR:
			return "VK_ERROR_INCOMPATIBLE_VERSION_KHR";
	#endif
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		case VK_ERROR_NOT_PERMITTED_EXT:
			return "VK_ERROR_NOT_PERMITTED_EXT";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:
			return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:
			return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:
			return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return "VK_OPERATION_NOT_DEFERRED_KHR";
		case VK_PIPELINE_COMPILE_REQUIRED_EXT:
			return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		default:
			break;
		}
		if (result < 0) {
			return "VK_ERROR_<Unknown>";
		}
		return "VK_<Unknown>";
	}

	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx)
	{
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(VkResult const& result, FormatContext& ctx)
	{
		return fmt::format_to(ctx.out(), get_result_string(result));
	}
};


class Logger {
private:
	LogLevel _verboseLevel = LogLevel::Info;
public:
	void SetVerboseLevel(LogLevel level) {_verboseLevel = level;}

	template <typename... Args>
	inline static void print(std::string_view message, Args... args)
	{
		fmt::print((message), args...);
		fmt::print("\n");
	}

	template <typename... Args>
	inline static void log(LogLevel type,std::string_view message, Args... args)
	{
		if (type > Logger::Get()._verboseLevel) return;
		print_time();
		switch (type)
		{
		case LogLevel::Fatal:
			fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
				"[FATAL]   ");
			break;
		case LogLevel::Error:
			fmt::print(fg(fmt::color::crimson),
				"[ERROR]   ");
			break;
		case LogLevel::Warning:
			fmt::print(fg(fmt::color::yellow),
				"[WARNING] ");
			break;
		case LogLevel::Success:
			fmt::print(fg(fmt::color::light_green),
				"[SUCCESS] ");
			break;
		case LogLevel::Info:
			fmt::print(fg(fmt::color::white),
				"[INFO]    ");
			break;
		case LogLevel::Trace:
			fmt::print(fg(fmt::color::gray),
				"[TRACE]   ");
			break;
		}		
		
		print(message, args...);

		if (type == LogLevel::Fatal)
		{
			abort();
		}
	}

	inline static void print_time() {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

		auto time_point = now - Logger::Get().start_time_;

		
		fmt::print("[{:%M:%S}]", time_point);
	}

	inline static Logger& Get() {
		static Logger handler{};
		return handler;
	}

	void set_time() {
		start_time_ = std::chrono::system_clock::now();
	}

	std::chrono::time_point<std::chrono::system_clock> start_time_;

};
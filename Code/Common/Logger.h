/*
*   Copyright (C) {2015}  {VK, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at:
*/

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>
#include <functional>

namespace W2PP {

class Logger {
public:
    // Initialize the logger with a name and optional log directory
    static void Initialize(const std::string& name, const std::string& logDir = "./logs");
    
    // Get the logger instance
    static std::shared_ptr<spdlog::logger> Get();
    
    // Shutdown and cleanup
    static void Shutdown();
    
    // Set a GUI callback for compatibility during transition (optional)
    // The callback receives (message, log_level) where log_level is:
    // 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical
    static void SetGuiCallback(std::function<void(const std::string&, int)> callback);
    
    // Legacy compatibility: Log to file (used by existing Log() function)
    static void LogToFile(const std::string& message);

private:
    static std::shared_ptr<spdlog::logger> s_logger;
    static std::function<void(const std::string&, int)> s_guiCallback;
    static std::string s_logDir;
    static std::string s_name;
};

// Convenience macros for logging
#define LOG_TRACE(...)    W2PP::Logger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    W2PP::Logger::Get()->debug(__VA_ARGS__)
#define LOG_INFO(...)     W2PP::Logger::Get()->info(__VA_ARGS__)
#define LOG_WARN(...)     W2PP::Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    W2PP::Logger::Get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) W2PP::Logger::Get()->critical(__VA_ARGS__)

// Legacy compatibility macro
#define LOG_FILE(msg)     W2PP::Logger::LogToFile(msg)

} // namespace W2PP

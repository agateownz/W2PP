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

#include "Logger.h"
#include <filesystem>
#include <iostream>

namespace W2PP {

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
std::function<void(const std::string&, int)> Logger::s_guiCallback = nullptr;
std::string Logger::s_logDir = "./logs";
std::string Logger::s_name = "W2PP";

void Logger::Initialize(const std::string& name, const std::string& logDir) {
    s_name = name;
    s_logDir = logDir;
    
    try {
        // Create log directory if it doesn't exist
        std::filesystem::create_directories(logDir);
        
        // Create a vector of sinks
        std::vector<spdlog::sink_ptr> sinks;
        
        // Console sink with colors
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::info);
        consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        sinks.push_back(consoleSink);
        
        // Rotating file sink (max 10MB per file, max 3 files)
        std::string logFile = logDir + "/" + name + ".log";
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFile, 1024 * 1024 * 10, 3);
        fileSink->set_level(spdlog::level::debug);
        fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        sinks.push_back(fileSink);
        
        // Create the logger
        s_logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        s_logger->set_level(spdlog::level::debug);
        s_logger->flush_on(spdlog::level::info);
        
        // Log initialization
        s_logger->info("Logger initialized: {}", name);
        
    } catch (const std::exception& e) {
        // Fallback to console if file logging fails
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        s_logger = spdlog::stdout_color_mt(name);
    }
}

std::shared_ptr<spdlog::logger> Logger::Get() {
    if (!s_logger) {
        // Auto-initialize with default settings if not already done
        Initialize("W2PP");
    }
    return s_logger;
}

void Logger::Shutdown() {
    if (s_logger) {
        s_logger->info("Logger shutting down");
        s_logger->flush();
        s_logger.reset();
    }
    spdlog::shutdown();
}

void Logger::SetGuiCallback(std::function<void(const std::string&, int)> callback) {
    s_guiCallback = callback;
}

void Logger::LogToFile(const std::string& message) {
    // Legacy compatibility: write to a separate file for day logs
    // This is used by the existing Log() function in Basedef.cpp
    if (s_logger) {
        s_logger->info("[FILE] {}", message);
    }
}

} // namespace W2PP

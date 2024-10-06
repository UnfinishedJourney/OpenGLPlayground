#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h> 
#include <memory>

class Logger {
public:
    static void Init();
    inline static std::shared_ptr<spdlog::logger> GetLogger() { return s_Logger; }
private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};
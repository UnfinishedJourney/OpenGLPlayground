#include "Logger.h"

#include <spdlog/async.h> 
#include <spdlog/async_logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>
#include <iostream>

std::shared_ptr<spdlog::logger> Logger::s_Logger;

void Logger::Init() {
    try {
        // Define the size of the queue (number of messages that can be queued)
        size_t queue_size = 8192;
        // Number of threads to process the queue
        size_t thread_count = 1;

        // Initialize the thread pool
        spdlog::init_thread_pool(queue_size, thread_count);

        // Create console and file sinks
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("../logs/engine.log", true);

        // Combine sinks into a vector
        std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };

        // Create an asynchronous logger
        s_Logger = std::make_shared<spdlog::async_logger>(
            "ENGINE",
            sinks.begin(),
            sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        // Set the default logger to our asynchronous logger
        spdlog::set_default_logger(s_Logger);
        spdlog::set_level(spdlog::level::info); // Set log level to info
        spdlog::flush_on(spdlog::level::info);  // Flush logs on info level and above

        s_Logger->info("Logger initialized successfully.");
    }
    catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Error initializing spdlog: " << ex.what() << std::endl;
    }
}
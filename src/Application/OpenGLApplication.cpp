#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Application/Application.h"

// If you use EasyProfiler or any other profiler, include its headers here:
#define USING_EASY_PROFILER
#include <easy/profiler.h>

/**
 * @brief Main entry point for the OpenGL Application.
 *
 * @return int Returns zero on successful execution, non-zero otherwise.
 */
int main()
{
    // Initialize EasyProfiler (if you're using it)
    EASY_PROFILER_ENABLE;
    profiler::startListen();

    // Create and initialize the application
    Application app;
    if (!app.Init())
    {
        // If initialization fails, log and exit
        SPDLOG_ERROR("Application failed to initialize. Exiting.");
        return -1;
    }

    // Run the main loop
    app.Run();

    // (Optional) dump profiler data
    profiler::dumpBlocksToFile("profile_data.prof");

    return 0;
}
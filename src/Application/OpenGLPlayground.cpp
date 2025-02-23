#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Application/Application.h"
#define USING_EASY_PROFILER
#include <easy/profiler.h>

int main() {
    EASY_PROFILER_ENABLE;
    profiler::startListen();

    Application app;
    if (!app.Init()) {
        SPDLOG_ERROR("Application failed to initialize. Exiting.");
        return -1;
    }

    app.Run();
    profiler::dumpBlocksToFile("profile_data.prof");

    return 0;
}
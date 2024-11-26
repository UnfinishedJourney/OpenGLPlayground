#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Application/Application.h"

//add macro to cmake
#define USING_EASY_PROFILER
#include <easy/profiler.h>

//rename file to main?
int main()
{
    EASY_PROFILER_ENABLE;
    profiler::startListen();

    Application app;
    app.Init();
    app.Run();

    profiler::dumpBlocksToFile("profile_data.prof");
    return 0;
}
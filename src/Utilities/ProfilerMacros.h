#pragma once

#include "Config.h"

// Define ENABLE_PROFILING to enable profiling.
// You can define this in your build system (e.g., compiler flags) or uncomment the line below for testing.
// #define ENABLE_PROFILING
#ifdef ENABLE_PROFILING
#define USING_EASY_PROFILER
#include <easy/profiler.h>

#define PROFILE_FUNCTION(color) EASY_FUNCTION(profiler::colors::color)
#define PROFILE_BLOCK(name, color) EASY_BLOCK(name, profiler::colors::color)
#define PROFILE_CUSTOM(name, value) profiler::custom_data(name, value)
#else
    // If profiling is disabled, define empty macros to avoid any overhead.
#define PROFILE_FUNCTION(color)
#define PROFILE_BLOCK(name, color)
#define PROFILE_CUSTOM(name, value)
#endif
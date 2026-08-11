#pragma once

#include <farplug-wide.h>

struct PluginContext {
    PluginStartupInfo Info;
    FarStandardFunctions* FSF;
};
#pragma once

#include <farplug-wide.h>  

struct PluginContext {
    PluginStartupInfo Info;
    FarStandardFunctions* FSF;
    // AliasManager используется как синглтон, поэтому не храним его здесь
};
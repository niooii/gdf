#pragma once

#include <gdfe/../../gdfe/include/gdfe/core.h>

typedef struct GDF_ClientSettings {
    GDF_BOOL verbose_output;
    GDF_BOOL client_show_console;
} GDF_ClientSettings;

GDF_BOOL GDF_ClientSettings_Load();
GDF_BOOL GDF_ClientSettings_Save();
GDF_ClientSettings* GDF_AppSettings_Get();
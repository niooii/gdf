#pragma once
#include <core.h>
#include <logging.h>
#include <os/window.h>
#include <os/sysinfo.h>
#include <os/io.h>
#include <subsystems.h>
#include <serde/serde.h>
#include <client/client_settings.h>
#include <render/renderer.h>

// TODO! this file and app.c are quite useless put these into main.c

bool GDF_InitApp();
// responsible for initializing directories and files with default values.
// TODO!
bool GDF_InitFirstLaunch();
// returns the time the app ran for in seconds
f64 GDF_RunApp();
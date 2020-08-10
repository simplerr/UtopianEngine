#pragma once

#if defined(_WIN32)

#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR 1

#elif defined(__linux__)

#define VK_USE_PLATFORM_XCB_KHR 1
#include <xcb/xcb.h>

#endif

// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <sdkddkver.h>
#define NTDDI_VERSION WDK_NTDDI_VERSION
#include <Windows.h>
#include <shlwapi.h>
#include <fwpmu.h>
#include <string>
#include <vector>
#include <memory>
#include <concepts>
#include <type_traits>
#include <unordered_map>

#endif //PCH_H

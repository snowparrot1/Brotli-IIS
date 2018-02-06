// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _WIN32_WINNT 0x0601
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <brotli\encode.h>

#pragma comment(lib, "brotlicommon-static.lib")
#pragma comment(lib, "brotlienc-static.lib")

#define DLL_EXPORT extern "C" __declspec(dllexport)
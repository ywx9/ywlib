#pragma once

#include <array>
#include <compare>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <Windows.h>
#include <commdlg.h>
#include <d2d1_3.h>
#include <d3d11_3.h>
#include <d3dcompiler.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <shellapi.h>
#include <wincodec.h>
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "uuid.lib")
#undef max
#undef min

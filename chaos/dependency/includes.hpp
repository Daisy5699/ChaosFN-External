#pragma once
#define _CRT_SECURE_NO_WARNINGS

#ifdef __cplusplus

#include <Windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <aclapi.h>
#include <dbghelp.h>

#include <chrono>
#include <ctime>
#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <functional>
#include <map>
#include <string>
#include <memory>
#include <random>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <cstdint>
#include <numbers>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#include "client/utility/console/console.hpp"
#include "client/utility/output/output.hpp"
#include "client/driver/driver.hpp"
#include "client/sdk/sdk.hpp"
#include "client/graphics/graphics.hpp"
#include "client/features/exploits/exploits.hpp"

#endif
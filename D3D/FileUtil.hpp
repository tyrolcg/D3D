#pragma once

#include <string>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

bool SearchFilePath(const wchar_t* fileName, std::wstring& result);


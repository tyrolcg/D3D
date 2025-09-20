#include "FileUtil.hpp"


	bool SearchFilePath(const wchar_t* fileName, std::wstring& result)
	{
		if (fileName == nullptr) return false;
			
		if (wcscmp(fileName, L" ") == 0 || wcscmp(fileName, L" ") == 0) return false;

		const int exePathCount = 520;
		wchar_t exePath[exePathCount] = {};
		GetModuleFileNameW(nullptr, exePath, _countof(exePath));
		// nullèIí[ï∂éöÇê›íË
		exePath[exePathCount - 1] = L'\0';
		PathRemoveFileSpecW(exePath);

		wchar_t dstPath[exePathCount] = {};

		wcscpy_s(dstPath, fileName);
		if (PathFileExists(dstPath) == TRUE) 
		{

		}
	}
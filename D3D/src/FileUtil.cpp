#include "FileUtil.h"


	bool SearchFilePath(const wchar_t* fileName, std::wstring& result)
	{
		if (fileName == nullptr) return false;
			
		if (wcscmp(fileName, L" ") == 0 || wcscmp(fileName, L" ") == 0) return false;

		const int exePathCount = 520;
		wchar_t exePath[exePathCount] = {};
		GetModuleFileNameW(nullptr, exePath, _countof(exePath));
		// null終端文字を設定
		exePath[exePathCount - 1] = L'\0';
		PathRemoveFileSpecW(exePath);

		wchar_t dstPath[exePathCount] = {};

		wcscpy_s(dstPath, fileName);
		if (PathFileExists(dstPath) == TRUE) 
		{
			result = dstPath;
			return true;
		}


		swprintf_s(dstPath, L"..\\%s", fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}

		swprintf_s(dstPath, L"..\..\\%s", fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}
		
		swprintf_s(dstPath, L"\\res\\%s", fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}

		swprintf_s(dstPath, L"%s\\%s", exePath, fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}

		swprintf_s(dstPath, L"%s\\..\\%s", exePath, fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}

		swprintf_s(dstPath, L"%s\\..\\..\\%s", exePath, fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}

		swprintf_s(dstPath, L"%s\\res\\%s", exePath, fileName);
		if (PathFileExistsW(dstPath))
		{
			result = dstPath;
			return true;
		}

		return false;

	}
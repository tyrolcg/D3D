#include "App.h"

/// <summary>
/// メインエントリーポイント
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="evnp"></param>
/// <returns></returns>
int wmain(int argc, wchar_t argv, wchar_t** evnp)
{
	App app(960, 540);
	app.Run();

	return 0;
}
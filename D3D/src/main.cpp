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
	// プログラム終了時のメモリリークチェック、出力を行う
#if defined(DEBUG) || defined(_DEBUG)
	{
		_CrtSetDbgFlag(
			_CRTDBG_ALLOC_MEM_DF    // デバッグ用のヒープメモリが使われるようになる
			| _CRTDBG_LEAK_CHECK_DF // プログラム終了時にメモリリークチェックを行う
		);
	}
#endif
	App app(960, 540);
	app.Run();

	return 0;
}
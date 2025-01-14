#include "App.h"

namespace /* anonymous */
{
	const auto ClassName = TEXT("SampleWindowClass");
}

App::App(uint32_t width, uint32_t height)
	: mhInst(nullptr) // 初期化子リスト 変数名(初期値)
	, mhWnd(nullptr)
	, mWidth(width)
	, mHeight(height)
{
	// do nothing
}

App::~App() {
	/* do nothing */
}

void App::Run() {
	if (InitApp())
	{
		MainLoop();
	}

	TermApp();
}

bool App::InitApp()
{
	if (!InitWnd())
	{
		return false;
	}
}

///<summary>
/// ウィンドウの初期化処理
///</summary>
bool App::InitWnd() {
	// インスタンスハンドルを取得
	auto hInst = GetModuleHandle(nullptr); // https://learn.microsoft.com/ja-jp/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea
	if (hInst == nullptr)
	{
		return false;
	}
	// ウィンドウの設定
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX); // WNDCLASSEX構造体のサイズ
	wc.style = CS_HREDRAW  // 水平方向のサイズが変更されたとき再描画する
			 | CS_VREDRAW; // 垂直方向のサイズが変更されたとき再描画する
	wc.lpfnWndProc = WndProc; // ウィンドウへのメッセージ(クリック、サイズ変更など)が送られてきたときに呼び出されるコールバック関数
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION); // アイコンのハンドル
	wc.hCursor = LoadCursor(hInst, IDC_ARROW); // カーソルのハンドル
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND); // 背景ブラシへのハンドル。物理ブラシも色の値も指定できる。要するに背景として塗りつぶす色を指定する。
	wc.lpszMenuName = nullptr; // メニューバーに表示されるメニューのリソース名
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);


	// ウィンドウの登録
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// インスタンスハンドルの設定
	mhInst = hInst;

	// ウィンドウのサイズを設定
	RECT rc = {};
	rc.right = static_cast<LONG>(mWidth);
	rc.bottom = static_cast<LONG>(mHeight);
	
	// ウィンドウのスタイルを設定
	// https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-styles
	auto style = WS_OVERLAPPED // オーバーラップウィンドウに設定
			   | WS_CAPTION	   // タイトルバーを付ける
			   | WS_SYSMENU;   // タイトルバーにウィンドウメニューを付ける

	// スタイルを元にサイズ調整
	AdjustWindowRect(&rc, style, FALSE);

	// ウィンドウを生成
	mhWnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("SAMPLE"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		mhInst,
		nullptr
	);
	if (mhWnd == nullptr)
	{
		return false;
	}

	// ウィンドウ表示
	ShowWindow(mhWnd, SW_SHOWNORMAL);

	// ウィンドウを更新
	UpdateWindow(mhWnd);

	// ウィンドウにフォーカスを設定
	SetFocus(mhWnd);

	// 正常終了
	return true;
}

///<summary>
/// メインループ
///</summary>
void App::MainLoop()
{
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

/// <summary>
/// アプリケーションの終了処理
/// </summary>
void App::TermApp()
{
	// ウィンドウを終了
	TermWnd();
}

/// <summary>
/// ウィンドウの終了処理
/// </summary>
void App::TermWnd()
{
	// ウィンドウの登録を解除
	if (mhInst != nullptr)
	{
		UnregisterClass(ClassName, mhInst);
	}

	// ハンドルのクリア
	mhInst = nullptr;
	mhWnd = nullptr;
}

/// <summary>
/// ウィンドウプロシージャ
/// </summary>
/// <param name="hWnd">ウィンドウハンドル</param>
/// <param name="msg">メッセージコード</param>
/// <param name="wp">追加パラメータ</param>
/// <param name="lp">追加パラメータ</param>
/// <returns></returns>
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_DESTROY:
			{ PostQuitMessage(0); }
			break;
		default:
			{ /* do nothing */ }
			break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}
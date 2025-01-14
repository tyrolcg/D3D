#include "App.h"

namespace /* anonymous */
{
	const auto ClassName = TEXT("SampleWindowClass");
}

template<typename T>
void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}
App::App(uint32_t width, uint32_t height)
	: m_hInst(nullptr) // 初期化子リスト 変数名(初期値)
	, m_hWnd(nullptr)
	, m_Width(width)
	, m_Height(height)
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
	m_hInst = hInst;

	// ウィンドウのサイズを設定
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);
	
	// ウィンドウのスタイルを設定
	// https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-styles
	auto style = WS_OVERLAPPED // オーバーラップウィンドウに設定
			   | WS_CAPTION	   // タイトルバーを付ける
			   | WS_SYSMENU;   // タイトルバーにウィンドウメニューを付ける

	// スタイルを元にサイズ調整
	AdjustWindowRect(&rc, style, FALSE);

	// ウィンドウを生成
	m_hWnd = CreateWindowEx(
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
		m_hInst,
		nullptr
	);
	if (m_hWnd == nullptr)
	{
		return false;
	}

	// ウィンドウ表示
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// ウィンドウを更新
	UpdateWindow(m_hWnd);

	// ウィンドウにフォーカスを設定
	SetFocus(m_hWnd);

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
	if (m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}

	// ハンドルのクリア
	m_hInst = nullptr;
	m_hWnd = nullptr;
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

/// <summary>
/// Direct3Dの初期化処理
/// </summary>
/// <returns>正常終了か</returns>
bool App::InitD3D()
{
	// デバイスの作成
	// https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
	auto hr = D3D12CreateDevice(
		nullptr, // 使用するビデオアダプタへのポインタ。既定のアダプタを利用する場合はnullptrを渡す。
		D3D_FEATURE_LEVEL_11_0, // 最小D3D_FEATURE_LEVEL
		IID_PPV_ARGS(&m_pDevice) // デバイスインターフェースを受け取るポインタ。IID_PPV_ARGSマクロを使うことで、uuidofによるGUIDの取得とvoid**へのキャストを行う。
	);
	if (FAILED(hr))
	{
		return false;
	}

	// コマンドキューの作成
	// https://cocoa-programing.hatenablog.com/entry/2018/11/19/%E3%80%90DirectX12%E3%80%91%E3%82%B3%E3%83%9E%E3%83%B3%E3%83%89%E3%82%AD%E3%83%A5%E3%83%BC%E3%81%A8%E3%82%B9%E3%83%AF%E3%83%83%E3%83%97%E3%83%81%E3%82%A7%E3%82%A4%E3%83%B3%E3%81%AE%E4%BD%9C
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// スワップチェイン
	{
		// DXGIファクトリの生成
		// DXGIに関するオブジェクトを生成するためのファクトリクラス
		// スワップチェインの作成に必要
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
		{
			return false;
		}

		// スワップチェインの設定
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60; // リフレッシュレートの分母
		desc.BufferDesc.RefreshRate.Denominator = 1; // リフレッシュレートの分子
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Quality = 0;
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// スワップチェインの作成
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue, &desc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			return false;
		}
	}

}
#include "App.h"
#include <cassert>

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
	// デバッグレイヤーの有効化
	// APIのエラーや警告を出力できるようにする
	// https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/understanding-the-d3d12-debug-layer
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

		if (SUCCEEDED(hr))
		{
			debug->EnableDebugLayer();
		}
	}
#endif
	// デバイスの作成
	// https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
	auto hr = D3D12CreateDevice(
		nullptr, // 使用するビデオアダプタへのポインタ。既定のアダプタを利用する場合はnullptrを渡す。
		D3D_FEATURE_LEVEL_11_0, // 最小D3D_FEATURE_LEVEL
		IID_PPV_ARGS(m_pDevice.GetAddressOf()) // デバイスインターフェースを受け取るポインタ。IID_PPV_ARGSマクロを使うことで、uuidofによるGUIDの取得とvoid**へのキャストを行う。
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
		// https://learn.microsoft.com/ja-jp/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60; // リフレッシュレートの分母
		desc.BufferDesc.RefreshRate.Denominator = 1; // リフレッシュレートの分子
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // 走査線の処理順の指定をしない
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // スケーリングの設定
		// https://learn.microsoft.com/ja-jp/windows-hardware/drivers/display/scaling-the-desktop-image
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 表示フォーマットの指定
		// https://learn.microsoft.com/ja-jp/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
		desc.SampleDesc.Quality = 0; // 画像の品質レベル
		desc.SampleDesc.Count = 1; // ピクセルあたりのマルチサンプリング数
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// スワップチェインの作成
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			return false;
		}

		// IDXGISwapChain3を取得
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			SafeRelease(&pSwapChain);
			return false;
		}

		// 現在のバックバッファのインデックスを取得
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// 不要になったので解放する
		SafeRelease(&pFactory);
		SafeRelease(&pSwapChain);


	}

	// コマンドアロケータの作成
	// コマンドリストに割り当てられたメモリを管理する
	{
		for (auto i = 0u; i < FrameCount; i++)
		{
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_pCmdAllocator[i])
			);
			if (FAILED(hr))
			{
				return false;
			}
		}

	}

	// コマンドリストの作成
	{
		hr = m_pDevice->CreateCommandList(
			0, // 複数のGPUノードがある場合に識別するためのビットマスク。GPUが1つの場合は0を割り当てる
			D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストのタイプ。Directはコマンドキューに直接登録可能なリスト
			m_pCmdAllocator[m_FrameIndex].Get(), // バックバッファのアロケータを使う
			nullptr, // パイプラインステート。後で明示的に設定するためnullptrを渡しておく
			IID_PPV_ARGS(&m_pCmdList) // GUID
		);
		if (FAILED(hr))
		{
			return false;
		}
	}

	// ディスクリプタヒープの作成
	// GPU上のディスクリプタを保存するための配列
	// 今回はバッファをリソースとする
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FrameCount; // ヒープ内のディスクリプタの数。今回はバッファの分だけ作成する。
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // _SHADER_VISIBLEの場合はシェーダから参照できるようになる。
	desc.NodeMask = 0; // GPUノードの識別
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // ディスクリプタヒープの種類を指定。RTVはレンダーターゲットビューのこと。
	
	hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
	if (FAILED(hr))
	{
		return false;
	}

	// ディスクリプタヒープの先頭メモリ位置を取得
	auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
	// ディスクリプタのメモリサイズを取得
	// GetDescriptorHandleIncrementSize(type)はデバイス依存の固定値を返す
	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(desc.Type);

	for (auto i = 0u; i < FrameCount; i++)
	{
		// バックバッファを取得
		hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
		if (FAILED(hr))
		{
			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // どのような次元でリソースにアクセスするか？
		viewDesc.Texture2D.MipSlice = 0; // ミップマップレベル
		viewDesc.Texture2D.PlaneSlice = 0; // 使用テクスチャの平面のインデックス

		// レンダーターゲットビューの作成
		m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);
		m_HandleRTV[i] = handle;
		handle.ptr += incrementSize; // 次のディスクリプタの位置を設定
	}

	// フェンスの作成
	// https://glhub.blogspot.com/2016/07/dx12-id3d12fence.html
	{
		// フェンスカウンタの初期化
		for (auto i = 0u; i < FrameCount; i++)
		{
			m_FenceCounter[i] = 0;
		}

		// フェンスの作成
		// CPUとGPUの同期をとるために使う
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE, // 共有オプション
			IID_PPV_ARGS(&m_pFence)
		);
		if (FAILED(hr))
		{
			return false;
		}

		m_FenceCounter[m_FrameIndex]++;

		// イベントオブジェクトの作成
		// イベントオブジェクト:シグナル状態と非シグナル状態を持つ。処理が終了したらシグナル状態になり、それを取得して同期をとる。
		// https://goodfuture.candypop.jp/nifty/overlapped.htm
		m_FenceEvent = CreateEvent(
			nullptr, // SECURITY_ATTRIBUTES構造体。子プロセスでのハンドルの継承可能性を指定する。
			FALSE, // 自動のリセットオブジェクトを作成する。
			FALSE, // イベントオブジェクトの初期状態をシグナル状態にするか?
			nullptr // イベントオブジェクトの名前
		);
		if (m_FenceEvent == nullptr)
		{
			return false;
		}

		// コマンドリストを閉じる
		m_pCmdList->Close();
		return true;
	}






}

/// <summary>
/// 描画処理
/// </summary>
void App::Render()
{
	// コマンドの記録を開始するための初期化処理
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	// リソースバリアの設定
	// 利用中のリソースへの割り込み処理を防ぐ
	// https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // 表示する→書き込む、の用途の状態遷移を示すバリア
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // 
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get(); // バリアを設定するリソース
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// リソースバリアを追加
	m_pCmdList->ResourceBarrier(
		1, // リソースバリアの数
		&barrier
	);

	// レンダーターゲットの設定
	m_pCmdList->OMSetRenderTargets(
		1, // ディスクリプタの数
		&m_HandleRTV[m_FrameIndex],
		FALSE,
		nullptr
	);

	// クリアカラーの設定
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// レンダーターゲットをクリア
	m_pCmdList->ClearRenderTargetView(
		m_HandleRTV[m_FrameIndex],
		clearColor,
		0,
		nullptr
	);

	// 描画処理
	{
		/* do nothing */
	}

	// リソースバリアの設定
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// リソースバリアを追加
	m_pCmdList->ResourceBarrier(1, &barrier);

	// コマンドの記録を終了
	m_pCmdList->Close();

	// コマンドの実行
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get()};
	m_pQueue->ExecuteCommandLists(
		1, // コマンドリストの数
		ppCmdLists // コマンドリスト配列のポインタ
	);

	// 画面に表示
	Present(1);
}

/// <summary>
/// 画面表示と次フレームの準備を行う
/// </summary>
/// <param name="interval">垂直同期回数</param>
void App::Present(uint32_t interval)
{
	// 表示処理
	m_pSwapChain->Present(interval, 0);

	// 現在のフェンスカウンターを取得
	const auto currentFenceValue = m_FenceCounter[m_FrameIndex];
	// ここまでにCommandQueueに設定されたコマンドリストを実行すると
	// フェンスはcurrentFenceValueの値になる
	m_pQueue->Signal(m_pFence.Get(), currentFenceValue);

	// バックバッファの番号を更新
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// 次のフレーム(バックバッファ処理)が準備中であれば待機
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		// フェンスの値が特定の値になったとき、イベントをシグナル状態にする
		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		// フェンスイベントがシグナル状態になるまで or タイムアウト間隔が経過するまで待機する
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	// 次のフレームのフェンスカウンタを増やす
	m_FenceCounter[m_FrameIndex] = currentFenceValue + 1;
}

void App::WaitGpu()
{
	assert(m_pQueue != nullptr);
	assert(m_pFence != nullptr);
	assert(m_FenceEvent != nullptr);

	m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);

	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);

	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	m_FenceCounter[m_FrameIndex]++;
}

void App::TermD3D()
{
	WaitGpu();

	// フェンスイベントの破棄
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	m_pFence.Reset();

	// RTVの破棄
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; i++)
	{
		m_pColorBuffer[i].Reset();
	}

	// コマンドリストの破棄
	m_pCmdList.Reset();

	// コマンドアロケータの破棄
	for (auto i = 0u; i < FrameCount; i++)
	{
		m_pCmdAllocator[i].Reset();
	}

	// スワップチェインの破棄
	m_pSwapChain.Reset();

	// コマンドキューの破棄
	m_pQueue.Reset();

	// デバイスの破棄
	m_pDevice.Reset();

}
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
	, m_RotateAngle(0.0f)
	, m_Viewport({ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f }) // ビューポート初期化
	, m_Scissor({ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) }) // シザー矩形初期化
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
	if (!InitD3D()) 
	{
		return false;
	}
	if (!OnInit())
	{
		return false;
	}
	return true;
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
		else
		{
			Render();
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
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = m_Width;
		desc.Height = m_Height;
		desc.Scaling = DXGI_SCALING_STRETCH; // スケーリングの設定
		// https://learn.microsoft.com/ja-jp/windows-hardware/drivers/display/scaling-the-desktop-image
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 表示フォーマットの指定
		// https://learn.microsoft.com/ja-jp/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
		desc.SampleDesc.Quality = 0; // 画像の品質レベル
		desc.SampleDesc.Count = 1; // ピクセルあたりのマルチサンプリング数
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// スワップチェインの作成
		IDXGISwapChain1* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChainForHwnd(
			m_pQueue.Get(), // スワップチェインを使用するコマンドキュー
			m_hWnd, // スワップチェインを関連付けるウィンドウのハンドル
			&desc, // スワップチェインの設定
			nullptr, // フルスクリーンモードの設定。nullptrを指定すると、ウィンドウモードになる。
			nullptr, // 出力制御用のインターフェース。通常はnullptrでよい。
			&pSwapChain // 生成されたスワップチェインインターフェースへのポインタ
		);
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			return false;
		}

		// IDXGISwapChain3を取得し
		// フィールドに格納する
		// IDXGISwapChain3 m_pSwapChain;
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

bool App::OnInit()
{
	{

		// 頂点データ
		Vertex vertices[] = {
			{DirectX::XMFLOAT3(-1, 1, 0), DirectX::XMFLOAT4(0,0,1,1)},
			{DirectX::XMFLOAT3(1, 1, 0), DirectX::XMFLOAT4(0,1,0,1)},
			{DirectX::XMFLOAT3(1, -1, 0), DirectX::XMFLOAT4(1,0,0,1)},
			{DirectX::XMFLOAT3(-1, -1, 0), DirectX::XMFLOAT4(1,0,1,1)}
		};

		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU転送するための宣言。CPU書き込みが1回、GPU読み込みが1回のデータが適している
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.MipLevels = 1;
		desc.Width = sizeof(vertices);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;


		// リソースを生成
		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pVB.GetAddressOf())
		);

		if (FAILED(hr))
		{
			return false;
		}

		// マッピング
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}

		// 頂点データをバッファにコピー
		memcpy(ptr, vertices, sizeof(vertices));
		
		m_pVB->Unmap(0, nullptr);

		// 頂点バッファビューの設定
		// バッファは頂点を保持するのか、定数バッファなのか、自身では判断できない。ビューが必要。
		m_VBV = {};
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
	}

	// インデックスバッファビュー
	{
		uint32_t indices[] = {
			0,1,2,
			0,2,3
		};
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC indexDesc = {};
		indexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		indexDesc.Alignment = 0;
		indexDesc.Width = sizeof(indices);
		indexDesc.Height = 1;
		indexDesc.DepthOrArraySize = 1;
		indexDesc.MipLevels = 1;
		indexDesc.Format = DXGI_FORMAT_UNKNOWN;
		indexDesc.SampleDesc.Count = 1;
		indexDesc.SampleDesc.Quality = 0;
		indexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // 行優先
		indexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// リソースを生成
		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&indexDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pIB.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// マッピング
		void* ptr = nullptr;
		hr = m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}

		// インデックスデータをバッファにコピー
		memcpy(ptr, indices, sizeof(indices));
		m_pIB->Unmap(0, nullptr);
		// インデックスバッファビューの設定
		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_IBV.Format = DXGI_FORMAT_R32_UINT; // インデックスのフォーマット。uint32_tなのでR32_UINT
		m_IBV.SizeInBytes = static_cast<UINT>(sizeof(indices));
	}
	// 定数バッファ用のヒープを作成
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
		cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvDesc.NodeMask = 0;
		cbvDesc.NumDescriptors = 1 * FrameCount;
		cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		auto hr = m_pDevice->CreateDescriptorHeap(
			&cbvDesc,
			IID_PPV_ARGS(m_pHeapCBV.GetAddressOf())
			);
		if (FAILED(hr))
		{
			return false;
		}

	}

	// 定数バッファの生成
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		
		// リソース設定
		D3D12_RESOURCE_DESC cbDesc = {};
		cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbDesc.Alignment = 0;
		cbDesc.Width = sizeof(Transform);
		cbDesc.Height = 1;
		cbDesc.DepthOrArraySize = 1;
		cbDesc.MipLevels = 1;
		cbDesc.Format = DXGI_FORMAT_UNKNOWN;
		cbDesc.SampleDesc.Count = 1;
		cbDesc.SampleDesc.Quality = 0;
		cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		cbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		
		for (int i = 0; i < FrameCount; i++)
		{
			auto hr = m_pDevice->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&cbDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_pCB[i].GetAddressOf())
			);
			if (FAILED(hr))
			{
				return false;
			}

			auto address = m_pCB[i]->GetGPUVirtualAddress();
			auto handleCpu = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto handleGpu = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();
			handleCpu.ptr += i * incrementSize;
			handleGpu.ptr += i * incrementSize;

			auto cbvDesc = D3D12_CONSTANT_BUFFER_VIEW_DESC {};
			cbvDesc.BufferLocation = address;
			cbvDesc.SizeInBytes = (sizeof(Transform) + 255) & ~255; // 256バイトアラインメント

			m_CBV[i].HandleCpu = handleCpu;
			m_CBV[i].HandleGpu = handleGpu;
			m_CBV[i].Desc = cbvDesc;

			m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, m_CBV[i].HandleCpu);

			// マッピング
			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));
			if (FAILED(hr))
			{
				return false;
			}
			auto eyePos = DirectX::XMVectorSet(0, 0, 5, 0);
			auto targetPos = DirectX::XMVectorSet(0, 0, 0, 0);
			auto upward = DirectX::XMVectorSet(0, 1, 0, 0);

			auto fovY = DirectX::XMConvertToRadians(45.0f);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

			// 変換行列設定
			m_CBV[i].pBuffer->World = DirectX::XMMatrixIdentity();
			m_CBV[i].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			m_CBV[i].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);


		}
	}

	// RootSignature
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// ルートパラメータ
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 定数バッファビュー
		param.Descriptor.ShaderRegister = 0; // b0レジスタにバインド
		param.Descriptor.RegisterSpace = 0; // レジスタスペース0にバインド
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 頂点シェーダからのみアクセス可能

		// スタティックサンプラー
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 1; // 異方性フィルタリングを使用しない
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0; // s0レジスタにバインド
		samplerDesc.RegisterSpace = 0; // レジスタスペース0にバインド
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダからのみアクセス可能
		
		// ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.Flags = flag;
		rootDesc.NumParameters = 1; // ルートパラメータは1つ
		rootDesc.pParameters = &param;
		rootDesc.NumStaticSamplers = 1; // スタティックサンプラーは1つ
		rootDesc.pStaticSamplers = &samplerDesc;
		
		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// ルートシグネチャのシリアライズ
		auto hr = D3D12SerializeRootSignature(
			&rootDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&pBlob,
			&pErrorBlob
		);
		if (FAILED(hr))
		{
			return false;
		}

		// ルートシグネチャの生成
		hr = m_pDevice->CreateRootSignature(
			0,
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			IID_PPV_ARGS(m_pRootSignature.GetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}
	}


	// パイプラインステートの作成
	{
		// 入力レイアウト
		D3D12_INPUT_ELEMENT_DESC elementDesc[2];
		elementDesc[0].SemanticName = "POSITION"; // 頂点シェーダで使う変数名
		elementDesc[0].SemanticIndex = 0; // 同じ名前の変数が複数ある場合のインデックス
		elementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // データフォーマット
		elementDesc[0].InputSlot = 0; // 頂点バッファスロット
		elementDesc[0].AlignedByteOffset = 0; // 頂点データのオフセット
		elementDesc[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // 頂点データとして扱う
		elementDesc[0].InstanceDataStepRate = 0; // インスタンスデータとして扱わない

		elementDesc[1].SemanticName = "COLOR";
		elementDesc[1].SemanticIndex = 0;
		elementDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elementDesc[1].InputSlot = 0;
		elementDesc[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // オフセットを自動計算
		elementDesc[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elementDesc[1].InstanceDataStepRate = 0;

		// ラスタライザステート
		D3D12_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID; // ポリゴンの塗りつぶし方法
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // 背面カリングを有効にする
		rasterizerDesc.FrontCounterClockwise = false; // 左手座標系で面の正負判定をする
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS; // 深度バイアス値
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP; // 深度バイアスの最大値
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS; // 傾斜に基づく深度バイアスの値
		rasterizerDesc.DepthClipEnable = false; // 深度クリッピングを無効にする
		rasterizerDesc.MultisampleEnable = false; // マルチサンプリングを無効にする
		rasterizerDesc.AntialiasedLineEnable = false; // アンチエイリアスを無効にする
		rasterizerDesc.ForcedSampleCount = 0; // 強制的にサンプル数を設定しない
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; // 保守的ラスタライザを無効にする
		// REF: https://learn.microsoft.com/en-us/windows/win32/direct3d12/conservative-rasterization
		
		// レンダーターゲットのブレンドステート
		D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
		blendDesc.BlendEnable = false; // ブレンド無効
		blendDesc.LogicOpEnable = false; // 論理演算無効
		blendDesc.SrcBlend = D3D12_BLEND_ONE; // PIXEL_SHADERから出力された色にそのまま加算
		blendDesc.DestBlend = D3D12_BLEND_ZERO; // レンダーターゲットのRGB値に0を乗算
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD; // SrcBlendとDestBlendを加算
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE; // PIXEL_SHADERから出力されたアルファ値にそのまま加算
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO; // レンダーターゲットのアルファ値に0を乗算
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; // SrcBlendAlphaとDestBlendAlphaを加算
		blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP; // レンダーターゲットに対する論理演算を行わない
		blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RGBAすべてのチャンネルへの書き込みを許可

		// ブレンドステート
		D3D12_BLEND_DESC blendState = {};
		blendState.AlphaToCoverageEnable = false; // アルファ値に基づくマルチサンプリングを行わない
		blendState.IndependentBlendEnable = false; // 複数のレンダーターゲットに対して独立したブレンドステートを設定しない
		for (auto i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			blendState.RenderTarget[i] = blendDesc;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// 頂点シェーダをメモリに読み取る
		auto hr = D3DReadFileToBlob(L"SimpleVS.cso", pVSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}
		// ピクセルシェーダをメモリに読み取る
		hr = D3DReadFileToBlob(L"SimplePS.cso", pPSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// グラフィックスパイプラインステートの設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { elementDesc, _countof(elementDesc) }; // 頂点入力レイアウト
		psoDesc.pRootSignature = m_pRootSignature.Get(); // ルートシグネチャ
		psoDesc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() }; // 頂点シェーダ
		psoDesc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() }; // ピクセルシェーダ
		psoDesc.BlendState = blendState; // ブレンドステート
		psoDesc.RasterizerState = rasterizerDesc; // ラスタライザステート
		psoDesc.DepthStencilState.DepthEnable = false; // 深度バッファを使用しない
		psoDesc.DepthStencilState.StencilEnable = false; // ステンシルバッファを使用しない
		psoDesc.SampleMask = UINT_MAX; // 全サンプル有効
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 三角形プリミティブ
		psoDesc.NumRenderTargets = 1; // レンダーターゲットは1つ
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // レンダーターゲットのフォーマット
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN; // 深度ステンシルビューのフォーマット
		psoDesc.SampleDesc.Count = 1; // マルチサンプリングしない
		psoDesc.SampleDesc.Quality = 0; // 標準品質レベル
		psoDesc.NodeMask = 0; // 単一GPUノード

		// パイプラインステートを生成
		hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

	}
	return true;
}

/// <summary>
/// 描画処理
/// </summary>
void App::Render()
{

	// 更新処理
	{
		m_RotateAngle += 0.01f;
		m_CBV[m_FrameIndex].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle);
	}

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
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
		m_pCmdList->SetPipelineState(m_pPSO.Get());
		m_pCmdList->SetDescriptorHeaps(1, m_pHeapCBV.GetAddressOf());
		m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex].Desc.BufferLocation);
		m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
		m_pCmdList->IASetIndexBuffer(&m_IBV);
		m_pCmdList->RSSetScissorRects(1, &m_Scissor);
		m_pCmdList->RSSetViewports(1, &m_Viewport);

		m_pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
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

void App::OnTerm() 
{
	for (auto i = 0; i < FrameCount; i++)
	{
		if (m_pCB[i].Get() != nullptr)
		{
			m_pCB[i]->Unmap(0, nullptr);
			memset(&m_CBV[i].pBuffer, 0, sizeof(m_CBV[i].pBuffer));
		}
		m_pCB[i].Reset();
	}
}
#pragma once

// include
#include<Windows.h>
#include<cstdint>
#include<d3d12.h>
#include<dxgi1_4.h>
#include<wrl/client.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include<iostream>

#include"../shader/SimpleShaderHeader.hlsli"
#include"Mesh.h"

// Linker
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

// Type definition
template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGpu;
	T* pBuffer; // バッファ先頭アドレス
};

// structure definition
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct Transform
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
	char padding[64]; // 256バイトアラインメント用のパディング
};

struct Texture
{
	ComPtr<ID3D12Resource> pResource;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGpu;
};

class App {
public:
	//
	// public variables
	//

	// NOTHING

	// public methods

	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	// private variables
	// ハンドルとは、リソースを参照するための識別子(ポインタなど)
	HINSTANCE m_hInst; // インスタンスハンドル
	HWND m_hWnd;		 // ウィンドウハンドル
	uint32_t m_Width;
	uint32_t m_Height;

	static const uint32_t FrameCount = 2; // フレームバッファ数
	
	ComPtr<ID3D12Device> m_pDevice; // デバイス
	ComPtr<ID3D12CommandQueue> m_pQueue; // コマンドキュー
	ComPtr<IDXGISwapChain3> m_pSwapChain; // スワップチェーン
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount]; // カラーバッファ
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount]; // コマンドアロケータ
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList; // コマンドリスト
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV; // レンダーターゲットビューのディスクリプタヒープ
	ComPtr<ID3D12Fence> m_pFence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount]; // フェンスカウンター
	uint32_t m_FrameIndex; // フレーム番号
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];

	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV_SRV_UAV; // 定数バッファビュー、シェーダリソースビュー、アンオーダードアクセスビューのディスクリプタヒープ
	ComPtr<ID3D12Resource> m_pVB; // 頂点バッファ
	ComPtr<ID3D12Resource> m_pCB[FrameCount]; // 定数バッファ
	ComPtr<ID3D12Resource> m_pIB; // インデックスバッファ
	ComPtr<ID3D12RootSignature> m_pRootSignature; // ルートシグニチャ
	ComPtr<ID3D12PipelineState> m_pPSO; // パイプラインステートオブジェクト
	D3D12_VERTEX_BUFFER_VIEW m_VBV; // 頂点バッファビュー
	ConstantBufferView<Transform> m_CBV[FrameCount * 2]; // 定数バッファビュー
	D3D12_INDEX_BUFFER_VIEW m_IBV; // インデックスバッファビュー
	D3D12_VIEWPORT m_Viewport; // ビューポート
	D3D12_RECT m_Scissor; // シザー矩形
	
	ComPtr<ID3D12Resource> m_pDB; // 深度バッファ
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV; // 深度ステンシルビューのディスクリプタヒープ
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleDSV; // 深度ステンシルビューのハンドル

	Texture m_Texture; // テクスチャ

	float m_RotateAngle;

	static std::vector<Mesh> m_Meshes;
	static std::vector<Material> m_Materials;


	// private methods
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();
	bool OnInit();
	void OnTerm();

	bool InitD3D();
	void TermD3D();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

};

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
#include"PointLight.h"

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

// ワールド変換行列用の定数バッファ構造体
struct WorldCB
{
	DirectX::XMMATRIX World;
};

// 球体インスタンス情報の構造体
struct SphereInstance
{
	DirectX::XMFLOAT3 Position;
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
	static const uint32_t MaxSphereCount = 10; // 最大の球体数
	static const int SphereRowCount = 3; // 球体の行数
	static const int SphereColCount = 5; // 球体の列数
	
	std::vector<SphereInstance> m_SphereInstances; // 球体インスタンス情報
	float m_RotateAngle; // 回転角度

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
	// 例: ヘッダやクラス内
	std::vector<ComPtr<ID3D12Resource>> m_pCB; // 定数バッファリソース (FrameCount * MaxSphereCount)
	std::vector<Transform*> m_pCBMapped;       // マップ先ポインタ (FrameCount * MaxSphereCount)
	ComPtr<ID3D12Resource> m_pIB; // インデックスバッファ
	ComPtr<ID3D12RootSignature> m_pRootSignature; // ルートシグニチャ
	ComPtr<ID3D12PipelineState> m_pPSO; // パイプラインステートオブジェクト
	D3D12_VERTEX_BUFFER_VIEW m_VBV; // 頂点バッファビュー
	std::vector<ConstantBufferView<Transform>> m_CBV; // 定数バッファビュー (FrameCount * MaxSphereCount)
	D3D12_INDEX_BUFFER_VIEW m_IBV; // インデックスバッファビュー
	D3D12_VIEWPORT m_Viewport; // ビューポート
	D3D12_RECT m_Scissor; // シザー矩形
	
	ComPtr<ID3D12Resource> m_pDB; // 深度バッファ
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV; // 深度ステンシルビューのディスクリプタヒープ
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleDSV; // 深度ステンシルビューのハンドル

	Texture m_Texture; // テクスチャ

	static std::vector<Mesh> m_Meshes;
	static std::vector<Material> m_Materials;
	
	PointLight m_PointLight;
	struct PointLightCB {
		DirectX::XMFLOAT3 LightPosition;
		float LightIntensity;
		DirectX::XMFLOAT3 LightColor;
		float LightAttenuation;
	};
	PointLightCB m_PointLightCB;

	ComPtr<ID3D12Resource> m_pPointLightCB;
	UINT8* m_pPointLightCBMapped = nullptr;

	// PBRマテリアルパラメータ用の定数バッファ
	struct PBRMaterialCB {
		float Metallic;     // 金属度（0=非金属、1=金属）
		float Roughness;    // 粗さ（0=鏡面、1=ざらざら）
		DirectX::XMFLOAT3 BaseColor; // ベースカラー
		float AmbientFactor; // 環境光係数
	};
	PBRMaterialCB m_PBRMaterialCB;
	
	ComPtr<ID3D12Resource> m_pPBRMaterialCB;
	UINT8* m_pPBRMaterialCBMapped = nullptr;

	// カメラ情報用定数バッファ
	struct CameraCB {
		DirectX::XMFLOAT3 CamPosition;
		DirectX::XMFLOAT3 CamTarget;
		DirectX::XMFLOAT3 CamUp;
	};
	CameraCB m_CameraCB;
	ComPtr<ID3D12Resource> m_pCameraCB;
	UINT8* m_pCameraCBMapped = nullptr;

	// ワールド変換行列用定数バッファ
	WorldCB m_WorldCB;
	ComPtr<ID3D12Resource> m_pWorldCB;
	UINT8* m_pWorldCBMapped = nullptr;

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

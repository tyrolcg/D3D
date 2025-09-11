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

// Linker
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Type definition
template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
template<typename T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGpu;
	T* pBuffer; // �o�b�t�@�擪�A�h���X
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
	char padding[64]; // 256�o�C�g�A���C�������g�p�̃p�f�B���O
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
	// �n���h���Ƃ́A���\�[�X���Q�Ƃ��邽�߂̎��ʎq(�|�C���^�Ȃ�)
	HINSTANCE m_hInst; // �C���X�^���X�n���h��
	HWND m_hWnd;		 // �E�B���h�E�n���h��
	uint32_t m_Width;
	uint32_t m_Height;

	static const uint32_t FrameCount = 2; // �t���[���o�b�t�@��
	
	ComPtr<ID3D12Device> m_pDevice; // �f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue; // �R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain; // �X���b�v�`�F�[��
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount]; // �J���[�o�b�t�@
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount]; // �R�}���h�A���P�[�^
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList; // �R�}���h���X�g
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV; // �����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Fence> m_pFence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount]; // �t�F���X�J�E���^�[
	uint32_t m_FrameIndex; // �t���[���ԍ�
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];

	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV; // �萔�o�b�t�@�r���[�A�V�F�[�_���\�[�X�r���[�A�A���I�[�_�[�h�A�N�Z�X�r���[�̃f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> m_pVB; // ���_�o�b�t�@
	ComPtr<ID3D12Resource> m_pCB[FrameCount]; // �萔�o�b�t�@
	ComPtr<ID3D12Resource> m_pIB; // �C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12RootSignature> m_pRootSignature; // ���[�g�V�O�j�`��
	ComPtr<ID3D12PipelineState> m_pPSO; // �p�C�v���C���X�e�[�g�I�u�W�F�N�g
	D3D12_VERTEX_BUFFER_VIEW m_VBV; // ���_�o�b�t�@�r���[
	ConstantBufferView<Transform> m_CBV[FrameCount * 2]; // �萔�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW m_IBV; // �C���f�b�N�X�o�b�t�@�r���[
	D3D12_VIEWPORT m_Viewport; // �r���[�|�[�g
	D3D12_RECT m_Scissor; // �V�U�[��`
	
	ComPtr<ID3D12Resource> m_pDB; // �[�x�o�b�t�@
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV; // �[�x�X�e���V���r���[�̃f�B�X�N���v�^�q�[�v
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleDSV; // �[�x�X�e���V���r���[�̃n���h��


	float m_RotateAngle;


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

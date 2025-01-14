#pragma once

// include
#include<Windows.h>
#include<cstdint>
#include<d3d12.h>
#include<dxgi1_4.h>

// Linker
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

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
	
	ID3D12Device* m_pDevice; // �f�o�C�X
	ID3D12CommandQueue* m_pQueue; // �R�}���h�L���[
	IDXGISwapChain* m_pSwapChain; // �X���b�v�`�F�[��
	ID3D12Resource* m_pColorBuffer[FrameCount]; // �J���[�o�b�t�@
	ID3D12CommandAllocator* m_pCmdAllocator[FrameCount]; // �R�}���h�A���P�[�^
	ID3D12GraphicsCommandList* m_pCmdList;
	ID3D12DescriptorHeap* m_pHeapRTV;
	ID3D12Fence* m_pFence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount]; // �t���[���J�E���^�[
	uint32_t m_FrameIndex; // �t���[���ԍ�
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];

	// private methods
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	bool InitD3D();
	void TermD3D();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};

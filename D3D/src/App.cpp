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
	: m_hInst(nullptr) // �������q���X�g �ϐ���(�����l)
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
/// �E�B���h�E�̏���������
///</summary>
bool App::InitWnd() {
	// �C���X�^���X�n���h�����擾
	auto hInst = GetModuleHandle(nullptr); // https://learn.microsoft.com/ja-jp/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea
	if (hInst == nullptr)
	{
		return false;
	}
	// �E�B���h�E�̐ݒ�
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX); // WNDCLASSEX�\���̂̃T�C�Y
	wc.style = CS_HREDRAW  // ���������̃T�C�Y���ύX���ꂽ�Ƃ��ĕ`�悷��
			 | CS_VREDRAW; // ���������̃T�C�Y���ύX���ꂽ�Ƃ��ĕ`�悷��
	wc.lpfnWndProc = WndProc; // �E�B���h�E�ւ̃��b�Z�[�W(�N���b�N�A�T�C�Y�ύX�Ȃ�)�������Ă����Ƃ��ɌĂяo�����R�[���o�b�N�֐�
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION); // �A�C�R���̃n���h��
	wc.hCursor = LoadCursor(hInst, IDC_ARROW); // �J�[�\���̃n���h��
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND); // �w�i�u���V�ւ̃n���h���B�����u���V���F�̒l���w��ł���B�v����ɔw�i�Ƃ��ēh��Ԃ��F���w�肷��B
	wc.lpszMenuName = nullptr; // ���j���[�o�[�ɕ\������郁�j���[�̃��\�[�X��
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);


	// �E�B���h�E�̓o�^
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// �C���X�^���X�n���h���̐ݒ�
	m_hInst = hInst;

	// �E�B���h�E�̃T�C�Y��ݒ�
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);
	
	// �E�B���h�E�̃X�^�C����ݒ�
	// https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-styles
	auto style = WS_OVERLAPPED // �I�[�o�[���b�v�E�B���h�E�ɐݒ�
			   | WS_CAPTION	   // �^�C�g���o�[��t����
			   | WS_SYSMENU;   // �^�C�g���o�[�ɃE�B���h�E���j���[��t����

	// �X�^�C�������ɃT�C�Y����
	AdjustWindowRect(&rc, style, FALSE);

	// �E�B���h�E�𐶐�
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

	// �E�B���h�E�\��
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// �E�B���h�E���X�V
	UpdateWindow(m_hWnd);

	// �E�B���h�E�Ƀt�H�[�J�X��ݒ�
	SetFocus(m_hWnd);

	// ����I��
	return true;
}

///<summary>
/// ���C�����[�v
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
/// �A�v���P�[�V�����̏I������
/// </summary>
void App::TermApp()
{
	// �E�B���h�E���I��
	TermWnd();
}

/// <summary>
/// �E�B���h�E�̏I������
/// </summary>
void App::TermWnd()
{
	// �E�B���h�E�̓o�^������
	if (m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}

	// �n���h���̃N���A
	m_hInst = nullptr;
	m_hWnd = nullptr;
}

/// <summary>
/// �E�B���h�E�v���V�[�W��
/// </summary>
/// <param name="hWnd">�E�B���h�E�n���h��</param>
/// <param name="msg">���b�Z�[�W�R�[�h</param>
/// <param name="wp">�ǉ��p�����[�^</param>
/// <param name="lp">�ǉ��p�����[�^</param>
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
/// Direct3D�̏���������
/// </summary>
/// <returns>����I����</returns>
bool App::InitD3D()
{
	// �f�o�b�O���C���[�̗L����
	// API�̃G���[��x�����o�͂ł���悤�ɂ���
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
	// �f�o�C�X�̍쐬
	// https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
	auto hr = D3D12CreateDevice(
		nullptr, // �g�p����r�f�I�A�_�v�^�ւ̃|�C���^�B����̃A�_�v�^�𗘗p����ꍇ��nullptr��n���B
		D3D_FEATURE_LEVEL_11_0, // �ŏ�D3D_FEATURE_LEVEL
		IID_PPV_ARGS(m_pDevice.GetAddressOf()) // �f�o�C�X�C���^�[�t�F�[�X���󂯎��|�C���^�BIID_PPV_ARGS�}�N�����g�����ƂŁAuuidof�ɂ��GUID�̎擾��void**�ւ̃L���X�g���s���B
	);
	if (FAILED(hr))
	{
		return false;
	}

	// �R�}���h�L���[�̍쐬
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

	// �X���b�v�`�F�C��
	{
		// DXGI�t�@�N�g���̐���
		// DXGI�Ɋւ���I�u�W�F�N�g�𐶐����邽�߂̃t�@�N�g���N���X
		// �X���b�v�`�F�C���̍쐬�ɕK�v
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
		{
			return false;
		}

		// �X���b�v�`�F�C���̐ݒ�
		// https://learn.microsoft.com/ja-jp/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60; // ���t���b�V�����[�g�̕���
		desc.BufferDesc.RefreshRate.Denominator = 1; // ���t���b�V�����[�g�̕��q
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // �������̏������̎w������Ȃ�
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // �X�P�[�����O�̐ݒ�
		// https://learn.microsoft.com/ja-jp/windows-hardware/drivers/display/scaling-the-desktop-image
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �\���t�H�[�}�b�g�̎w��
		// https://learn.microsoft.com/ja-jp/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
		desc.SampleDesc.Quality = 0; // �摜�̕i�����x��
		desc.SampleDesc.Count = 1; // �s�N�Z��������̃}���`�T���v�����O��
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// �X���b�v�`�F�C���̍쐬
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			return false;
		}

		// IDXGISwapChain3���擾
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			SafeRelease(&pSwapChain);
			return false;
		}

		// ���݂̃o�b�N�o�b�t�@�̃C���f�b�N�X���擾
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// �s�v�ɂȂ����̂ŉ������
		SafeRelease(&pFactory);
		SafeRelease(&pSwapChain);


	}

	// �R�}���h�A���P�[�^�̍쐬
	// �R�}���h���X�g�Ɋ��蓖�Ă�ꂽ���������Ǘ�����
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

	// �R�}���h���X�g�̍쐬
	{
		hr = m_pDevice->CreateCommandList(
			0, // ������GPU�m�[�h������ꍇ�Ɏ��ʂ��邽�߂̃r�b�g�}�X�N�BGPU��1�̏ꍇ��0�����蓖�Ă�
			D3D12_COMMAND_LIST_TYPE_DIRECT, // �R�}���h���X�g�̃^�C�v�BDirect�̓R�}���h�L���[�ɒ��ړo�^�\�ȃ��X�g
			m_pCmdAllocator[m_FrameIndex].Get(), // �o�b�N�o�b�t�@�̃A���P�[�^���g��
			nullptr, // �p�C�v���C���X�e�[�g�B��Ŗ����I�ɐݒ肷�邽��nullptr��n���Ă���
			IID_PPV_ARGS(&m_pCmdList) // GUID
		);
		if (FAILED(hr))
		{
			return false;
		}
	}

	// �f�B�X�N���v�^�q�[�v�̍쐬
	// GPU��̃f�B�X�N���v�^��ۑ����邽�߂̔z��
	// ����̓o�b�t�@�����\�[�X�Ƃ���
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FrameCount; // �q�[�v���̃f�B�X�N���v�^�̐��B����̓o�b�t�@�̕������쐬����B
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // _SHADER_VISIBLE�̏ꍇ�̓V�F�[�_����Q�Ƃł���悤�ɂȂ�B
	desc.NodeMask = 0; // GPU�m�[�h�̎���
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �f�B�X�N���v�^�q�[�v�̎�ނ��w��BRTV�̓����_�[�^�[�Q�b�g�r���[�̂��ƁB
	
	hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
	if (FAILED(hr))
	{
		return false;
	}

	// �f�B�X�N���v�^�q�[�v�̐擪�������ʒu���擾
	auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
	// �f�B�X�N���v�^�̃������T�C�Y���擾
	// GetDescriptorHandleIncrementSize(type)�̓f�o�C�X�ˑ��̌Œ�l��Ԃ�
	auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(desc.Type);

	for (auto i = 0u; i < FrameCount; i++)
	{
		// �o�b�N�o�b�t�@���擾
		hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
		if (FAILED(hr))
		{
			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // �ǂ̂悤�Ȏ����Ń��\�[�X�ɃA�N�Z�X���邩�H
		viewDesc.Texture2D.MipSlice = 0; // �~�b�v�}�b�v���x��
		viewDesc.Texture2D.PlaneSlice = 0; // �g�p�e�N�X�`���̕��ʂ̃C���f�b�N�X

		// �����_�[�^�[�Q�b�g�r���[�̍쐬
		m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);
		m_HandleRTV[i] = handle;
		handle.ptr += incrementSize; // ���̃f�B�X�N���v�^�̈ʒu��ݒ�
	}

	// �t�F���X�̍쐬
	// https://glhub.blogspot.com/2016/07/dx12-id3d12fence.html
	{
		// �t�F���X�J�E���^�̏�����
		for (auto i = 0u; i < FrameCount; i++)
		{
			m_FenceCounter[i] = 0;
		}

		// �t�F���X�̍쐬
		// CPU��GPU�̓������Ƃ邽�߂Ɏg��
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE, // ���L�I�v�V����
			IID_PPV_ARGS(&m_pFence)
		);
		if (FAILED(hr))
		{
			return false;
		}

		m_FenceCounter[m_FrameIndex]++;

		// �C�x���g�I�u�W�F�N�g�̍쐬
		// �C�x���g�I�u�W�F�N�g:�V�O�i����ԂƔ�V�O�i����Ԃ����B�������I��������V�O�i����ԂɂȂ�A������擾���ē������Ƃ�B
		// https://goodfuture.candypop.jp/nifty/overlapped.htm
		m_FenceEvent = CreateEvent(
			nullptr, // SECURITY_ATTRIBUTES�\���́B�q�v���Z�X�ł̃n���h���̌p���\�����w�肷��B
			FALSE, // �����̃��Z�b�g�I�u�W�F�N�g���쐬����B
			FALSE, // �C�x���g�I�u�W�F�N�g�̏�����Ԃ��V�O�i����Ԃɂ��邩?
			nullptr // �C�x���g�I�u�W�F�N�g�̖��O
		);
		if (m_FenceEvent == nullptr)
		{
			return false;
		}

		// �R�}���h���X�g�����
		m_pCmdList->Close();
		return true;
	}






}

/// <summary>
/// �`�揈��
/// </summary>
void App::Render()
{
	// �R�}���h�̋L�^���J�n���邽�߂̏���������
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	// ���\�[�X�o���A�̐ݒ�
	// ���p���̃��\�[�X�ւ̊��荞�ݏ�����h��
	// https://learn.microsoft.com/ja-jp/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // �\�����遨�������ށA�̗p�r�̏�ԑJ�ڂ������o���A
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // 
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get(); // �o���A��ݒ肷�郊�\�[�X
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// ���\�[�X�o���A��ǉ�
	m_pCmdList->ResourceBarrier(
		1, // ���\�[�X�o���A�̐�
		&barrier
	);

	// �����_�[�^�[�Q�b�g�̐ݒ�
	m_pCmdList->OMSetRenderTargets(
		1, // �f�B�X�N���v�^�̐�
		&m_HandleRTV[m_FrameIndex],
		FALSE,
		nullptr
	);

	// �N���A�J���[�̐ݒ�
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// �����_�[�^�[�Q�b�g���N���A
	m_pCmdList->ClearRenderTargetView(
		m_HandleRTV[m_FrameIndex],
		clearColor,
		0,
		nullptr
	);

	// �`�揈��
	{
		/* do nothing */
	}

	// ���\�[�X�o���A�̐ݒ�
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// ���\�[�X�o���A��ǉ�
	m_pCmdList->ResourceBarrier(1, &barrier);

	// �R�}���h�̋L�^���I��
	m_pCmdList->Close();

	// �R�}���h�̎��s
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get()};
	m_pQueue->ExecuteCommandLists(
		1, // �R�}���h���X�g�̐�
		ppCmdLists // �R�}���h���X�g�z��̃|�C���^
	);

	// ��ʂɕ\��
	Present(1);
}

/// <summary>
/// ��ʕ\���Ǝ��t���[���̏������s��
/// </summary>
/// <param name="interval">����������</param>
void App::Present(uint32_t interval)
{
	// �\������
	m_pSwapChain->Present(interval, 0);

	// ���݂̃t�F���X�J�E���^�[���擾
	const auto currentFenceValue = m_FenceCounter[m_FrameIndex];
	// �����܂ł�CommandQueue�ɐݒ肳�ꂽ�R�}���h���X�g�����s�����
	// �t�F���X��currentFenceValue�̒l�ɂȂ�
	m_pQueue->Signal(m_pFence.Get(), currentFenceValue);

	// �o�b�N�o�b�t�@�̔ԍ����X�V
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// ���̃t���[��(�o�b�N�o�b�t�@����)���������ł���Αҋ@
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		// �t�F���X�̒l������̒l�ɂȂ����Ƃ��A�C�x���g���V�O�i����Ԃɂ���
		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		// �t�F���X�C�x���g���V�O�i����ԂɂȂ�܂� or �^�C���A�E�g�Ԋu���o�߂���܂őҋ@����
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	// ���̃t���[���̃t�F���X�J�E���^�𑝂₷
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

	// �t�F���X�C�x���g�̔j��
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	m_pFence.Reset();

	// RTV�̔j��
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; i++)
	{
		m_pColorBuffer[i].Reset();
	}

	// �R�}���h���X�g�̔j��
	m_pCmdList.Reset();

	// �R�}���h�A���P�[�^�̔j��
	for (auto i = 0u; i < FrameCount; i++)
	{
		m_pCmdAllocator[i].Reset();
	}

	// �X���b�v�`�F�C���̔j��
	m_pSwapChain.Reset();

	// �R�}���h�L���[�̔j��
	m_pQueue.Reset();

	// �f�o�C�X�̔j��
	m_pDevice.Reset();

}
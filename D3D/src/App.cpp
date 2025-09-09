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
	, m_RotateAngle(0.0f)
	, m_Viewport({ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f }) // �r���[�|�[�g������
	, m_Scissor({ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) }) // �V�U�[��`������
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
		else
		{
			Render();
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
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.Width = m_Width;
		desc.Height = m_Height;
		desc.Scaling = DXGI_SCALING_STRETCH; // �X�P�[�����O�̐ݒ�
		// https://learn.microsoft.com/ja-jp/windows-hardware/drivers/display/scaling-the-desktop-image
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �\���t�H�[�}�b�g�̎w��
		// https://learn.microsoft.com/ja-jp/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
		desc.SampleDesc.Quality = 0; // �摜�̕i�����x��
		desc.SampleDesc.Count = 1; // �s�N�Z��������̃}���`�T���v�����O��
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// �X���b�v�`�F�C���̍쐬
		IDXGISwapChain1* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChainForHwnd(
			m_pQueue.Get(), // �X���b�v�`�F�C�����g�p����R�}���h�L���[
			m_hWnd, // �X���b�v�`�F�C�����֘A�t����E�B���h�E�̃n���h��
			&desc, // �X���b�v�`�F�C���̐ݒ�
			nullptr, // �t���X�N���[�����[�h�̐ݒ�Bnullptr���w�肷��ƁA�E�B���h�E���[�h�ɂȂ�B
			nullptr, // �o�͐���p�̃C���^�[�t�F�[�X�B�ʏ��nullptr�ł悢�B
			&pSwapChain // �������ꂽ�X���b�v�`�F�C���C���^�[�t�F�[�X�ւ̃|�C���^
		);
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			return false;
		}

		// IDXGISwapChain3���擾��
		// �t�B�[���h�Ɋi�[����
		// IDXGISwapChain3 m_pSwapChain;
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

bool App::OnInit()
{
	{

		// ���_�f�[�^
		Vertex vertices[] = {
			{DirectX::XMFLOAT3(-1, 1, 0), DirectX::XMFLOAT4(0,0,1,1)},
			{DirectX::XMFLOAT3(1, 1, 0), DirectX::XMFLOAT4(0,1,0,1)},
			{DirectX::XMFLOAT3(1, -1, 0), DirectX::XMFLOAT4(1,0,0,1)},
			{DirectX::XMFLOAT3(-1, -1, 0), DirectX::XMFLOAT4(1,0,1,1)}
		};

		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU�]�����邽�߂̐錾�BCPU�������݂�1��AGPU�ǂݍ��݂�1��̃f�[�^���K���Ă���
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


		// ���\�[�X�𐶐�
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

		// �}�b�s���O
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}

		// ���_�f�[�^���o�b�t�@�ɃR�s�[
		memcpy(ptr, vertices, sizeof(vertices));
		
		m_pVB->Unmap(0, nullptr);

		// ���_�o�b�t�@�r���[�̐ݒ�
		// �o�b�t�@�͒��_��ێ�����̂��A�萔�o�b�t�@�Ȃ̂��A���g�ł͔��f�ł��Ȃ��B�r���[���K�v�B
		m_VBV = {};
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
	}

	// �C���f�b�N�X�o�b�t�@�r���[
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
		indexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // �s�D��
		indexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// ���\�[�X�𐶐�
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

		// �}�b�s���O
		void* ptr = nullptr;
		hr = m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}

		// �C���f�b�N�X�f�[�^���o�b�t�@�ɃR�s�[
		memcpy(ptr, indices, sizeof(indices));
		m_pIB->Unmap(0, nullptr);
		// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�
		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_IBV.Format = DXGI_FORMAT_R32_UINT; // �C���f�b�N�X�̃t�H�[�}�b�g�Buint32_t�Ȃ̂�R32_UINT
		m_IBV.SizeInBytes = static_cast<UINT>(sizeof(indices));
	}
	// �萔�o�b�t�@�p�̃q�[�v���쐬
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

	// �萔�o�b�t�@�̐���
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		
		// ���\�[�X�ݒ�
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
			cbvDesc.SizeInBytes = (sizeof(Transform) + 255) & ~255; // 256�o�C�g�A���C�������g

			m_CBV[i].HandleCpu = handleCpu;
			m_CBV[i].HandleGpu = handleGpu;
			m_CBV[i].Desc = cbvDesc;

			m_pDevice->CreateConstantBufferView(&m_CBV[i].Desc, m_CBV[i].HandleCpu);

			// �}�b�s���O
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

			// �ϊ��s��ݒ�
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

		// ���[�g�p�����[�^
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // �萔�o�b�t�@�r���[
		param.Descriptor.ShaderRegister = 0; // b0���W�X�^�Ƀo�C���h
		param.Descriptor.RegisterSpace = 0; // ���W�X�^�X�y�[�X0�Ƀo�C���h
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // ���_�V�F�[�_����̂݃A�N�Z�X�\

		// �X�^�e�B�b�N�T���v���[
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 1; // �ٕ����t�B���^�����O���g�p���Ȃ�
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0; // s0���W�X�^�Ƀo�C���h
		samplerDesc.RegisterSpace = 0; // ���W�X�^�X�y�[�X0�Ƀo�C���h
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // �s�N�Z���V�F�[�_����̂݃A�N�Z�X�\
		
		// ���[�g�V�O�l�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.Flags = flag;
		rootDesc.NumParameters = 1; // ���[�g�p�����[�^��1��
		rootDesc.pParameters = &param;
		rootDesc.NumStaticSamplers = 1; // �X�^�e�B�b�N�T���v���[��1��
		rootDesc.pStaticSamplers = &samplerDesc;
		
		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// ���[�g�V�O�l�`���̃V���A���C�Y
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

		// ���[�g�V�O�l�`���̐���
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


	// �p�C�v���C���X�e�[�g�̍쐬
	{
		// ���̓��C�A�E�g
		D3D12_INPUT_ELEMENT_DESC elementDesc[2];
		elementDesc[0].SemanticName = "POSITION"; // ���_�V�F�[�_�Ŏg���ϐ���
		elementDesc[0].SemanticIndex = 0; // �������O�̕ϐ�����������ꍇ�̃C���f�b�N�X
		elementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // �f�[�^�t�H�[�}�b�g
		elementDesc[0].InputSlot = 0; // ���_�o�b�t�@�X���b�g
		elementDesc[0].AlignedByteOffset = 0; // ���_�f�[�^�̃I�t�Z�b�g
		elementDesc[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // ���_�f�[�^�Ƃ��Ĉ���
		elementDesc[0].InstanceDataStepRate = 0; // �C���X�^���X�f�[�^�Ƃ��Ĉ���Ȃ�

		elementDesc[1].SemanticName = "COLOR";
		elementDesc[1].SemanticIndex = 0;
		elementDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elementDesc[1].InputSlot = 0;
		elementDesc[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // �I�t�Z�b�g�������v�Z
		elementDesc[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elementDesc[1].InstanceDataStepRate = 0;

		// ���X�^���C�U�X�e�[�g
		D3D12_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID; // �|���S���̓h��Ԃ����@
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // �w�ʃJ�����O��L���ɂ���
		rasterizerDesc.FrontCounterClockwise = false; // ������W�n�Ŗʂ̐������������
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS; // �[�x�o�C�A�X�l
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP; // �[�x�o�C�A�X�̍ő�l
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS; // �X�΂Ɋ�Â��[�x�o�C�A�X�̒l
		rasterizerDesc.DepthClipEnable = false; // �[�x�N���b�s���O�𖳌��ɂ���
		rasterizerDesc.MultisampleEnable = false; // �}���`�T���v�����O�𖳌��ɂ���
		rasterizerDesc.AntialiasedLineEnable = false; // �A���`�G�C���A�X�𖳌��ɂ���
		rasterizerDesc.ForcedSampleCount = 0; // �����I�ɃT���v������ݒ肵�Ȃ�
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; // �ێ�I���X�^���C�U�𖳌��ɂ���
		// REF: https://learn.microsoft.com/en-us/windows/win32/direct3d12/conservative-rasterization
		
		// �����_�[�^�[�Q�b�g�̃u�����h�X�e�[�g
		D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
		blendDesc.BlendEnable = false; // �u�����h����
		blendDesc.LogicOpEnable = false; // �_�����Z����
		blendDesc.SrcBlend = D3D12_BLEND_ONE; // PIXEL_SHADER����o�͂��ꂽ�F�ɂ��̂܂܉��Z
		blendDesc.DestBlend = D3D12_BLEND_ZERO; // �����_�[�^�[�Q�b�g��RGB�l��0����Z
		blendDesc.BlendOp = D3D12_BLEND_OP_ADD; // SrcBlend��DestBlend�����Z
		blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE; // PIXEL_SHADER����o�͂��ꂽ�A���t�@�l�ɂ��̂܂܉��Z
		blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO; // �����_�[�^�[�Q�b�g�̃A���t�@�l��0����Z
		blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD; // SrcBlendAlpha��DestBlendAlpha�����Z
		blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP; // �����_�[�^�[�Q�b�g�ɑ΂���_�����Z���s��Ȃ�
		blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RGBA���ׂẴ`�����l���ւ̏������݂�����

		// �u�����h�X�e�[�g
		D3D12_BLEND_DESC blendState = {};
		blendState.AlphaToCoverageEnable = false; // �A���t�@�l�Ɋ�Â��}���`�T���v�����O���s��Ȃ�
		blendState.IndependentBlendEnable = false; // �����̃����_�[�^�[�Q�b�g�ɑ΂��ēƗ������u�����h�X�e�[�g��ݒ肵�Ȃ�
		for (auto i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			blendState.RenderTarget[i] = blendDesc;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// ���_�V�F�[�_���������ɓǂݎ��
		auto hr = D3DReadFileToBlob(L"SimpleVS.cso", pVSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}
		// �s�N�Z���V�F�[�_���������ɓǂݎ��
		hr = D3DReadFileToBlob(L"SimplePS.cso", pPSBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// �O���t�B�b�N�X�p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { elementDesc, _countof(elementDesc) }; // ���_���̓��C�A�E�g
		psoDesc.pRootSignature = m_pRootSignature.Get(); // ���[�g�V�O�l�`��
		psoDesc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() }; // ���_�V�F�[�_
		psoDesc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() }; // �s�N�Z���V�F�[�_
		psoDesc.BlendState = blendState; // �u�����h�X�e�[�g
		psoDesc.RasterizerState = rasterizerDesc; // ���X�^���C�U�X�e�[�g
		psoDesc.DepthStencilState.DepthEnable = false; // �[�x�o�b�t�@���g�p���Ȃ�
		psoDesc.DepthStencilState.StencilEnable = false; // �X�e���V���o�b�t�@���g�p���Ȃ�
		psoDesc.SampleMask = UINT_MAX; // �S�T���v���L��
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // �O�p�`�v���~�e�B�u
		psoDesc.NumRenderTargets = 1; // �����_�[�^�[�Q�b�g��1��
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // �����_�[�^�[�Q�b�g�̃t�H�[�}�b�g
		psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN; // �[�x�X�e���V���r���[�̃t�H�[�}�b�g
		psoDesc.SampleDesc.Count = 1; // �}���`�T���v�����O���Ȃ�
		psoDesc.SampleDesc.Quality = 0; // �W���i�����x��
		psoDesc.NodeMask = 0; // �P��GPU�m�[�h

		// �p�C�v���C���X�e�[�g�𐶐�
		hr = m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

	}
	return true;
}

/// <summary>
/// �`�揈��
/// </summary>
void App::Render()
{

	// �X�V����
	{
		m_RotateAngle += 0.01f;
		m_CBV[m_FrameIndex].pBuffer->World = DirectX::XMMatrixRotationY(m_RotateAngle);
	}

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
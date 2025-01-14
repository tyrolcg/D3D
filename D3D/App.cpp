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
	// �f�o�C�X�̍쐬
	// https://learn.microsoft.com/ja-jp/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
	auto hr = D3D12CreateDevice(
		nullptr, // �g�p����r�f�I�A�_�v�^�ւ̃|�C���^�B����̃A�_�v�^�𗘗p����ꍇ��nullptr��n���B
		D3D_FEATURE_LEVEL_11_0, // �ŏ�D3D_FEATURE_LEVEL
		IID_PPV_ARGS(&m_pDevice) // �f�o�C�X�C���^�[�t�F�[�X���󂯎��|�C���^�BIID_PPV_ARGS�}�N�����g�����ƂŁAuuidof�ɂ��GUID�̎擾��void**�ւ̃L���X�g���s���B
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
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60; // ���t���b�V�����[�g�̕���
		desc.BufferDesc.RefreshRate.Denominator = 1; // ���t���b�V�����[�g�̕��q
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

		// �X���b�v�`�F�C���̍쐬
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue, &desc, &pSwapChain);
		if (FAILED(hr))
		{
			SafeRelease(&pFactory);
			return false;
		}
	}

}
#include "App.h"

namespace /* anonymous */
{
	const auto ClassName = TEXT("SampleWindowClass");
}

App::App(uint32_t width, uint32_t height)
	: mhInst(nullptr) // �������q���X�g �ϐ���(�����l)
	, mhWnd(nullptr)
	, mWidth(width)
	, mHeight(height)
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
	mhInst = hInst;

	// �E�B���h�E�̃T�C�Y��ݒ�
	RECT rc = {};
	rc.right = static_cast<LONG>(mWidth);
	rc.bottom = static_cast<LONG>(mHeight);
	
	// �E�B���h�E�̃X�^�C����ݒ�
	// https://learn.microsoft.com/ja-jp/windows/win32/winmsg/window-styles
	auto style = WS_OVERLAPPED // �I�[�o�[���b�v�E�B���h�E�ɐݒ�
			   | WS_CAPTION	   // �^�C�g���o�[��t����
			   | WS_SYSMENU;   // �^�C�g���o�[�ɃE�B���h�E���j���[��t����

	// �X�^�C�������ɃT�C�Y����
	AdjustWindowRect(&rc, style, FALSE);

	// �E�B���h�E�𐶐�
	mhWnd = CreateWindowEx(
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
		mhInst,
		nullptr
	);
	if (mhWnd == nullptr)
	{
		return false;
	}

	// �E�B���h�E�\��
	ShowWindow(mhWnd, SW_SHOWNORMAL);

	// �E�B���h�E���X�V
	UpdateWindow(mhWnd);

	// �E�B���h�E�Ƀt�H�[�J�X��ݒ�
	SetFocus(mhWnd);

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
	if (mhInst != nullptr)
	{
		UnregisterClass(ClassName, mhInst);
	}

	// �n���h���̃N���A
	mhInst = nullptr;
	mhWnd = nullptr;
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
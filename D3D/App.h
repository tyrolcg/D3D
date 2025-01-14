#pragma once

#include<Windows.h>
#include<cstdint>


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
	HINSTANCE mhInst; // �C���X�^���X�n���h��
	HWND mhWnd;		 // �E�B���h�E�n���h��
	uint32_t mWidth;
	uint32_t mHeight;

	// private methods
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};

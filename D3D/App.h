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
	// ハンドルとは、リソースを参照するための識別子(ポインタなど)
	HINSTANCE mhInst; // インスタンスハンドル
	HWND mhWnd;		 // ウィンドウハンドル
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

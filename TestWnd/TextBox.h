#pragma once
#include <xstring>
#include <iostream>
#include <wtypes.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")
#include "resource.h"

class TextBox
{
public:
	HWND m_hWnd;
	std::wstring m_Text;
	std::wstring m_CompText;
	RECT m_rectComp;
	std::shared_ptr<std::wstring[]> Candidates;
	LONG Count;

	LONG m_CaretPos;

	TextBox(HWND hWnd);

	VOID Draw(HWND hwnd, HDC hdc, PAINTSTRUCT* ps);
	//For IME CandidateWnd
	VOID GetCompExt(RECT* rect);

	VOID onChar(WPARAM wParam, LPARAM lParam);
	VOID onKeyDown(WPARAM wParam, LPARAM lParam);
	VOID onKeyUp(WPARAM wParam, LPARAM lParam);
};

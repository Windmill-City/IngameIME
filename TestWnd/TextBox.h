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
	HWND									m_hWnd;
	std::wstring							m_wstrTextContent;
	std::wstring							m_wstrComposition;
	RECT									m_rectComp;
	std::shared_ptr<std::wstring[]>			m_pCandidates;
	LONG									m_lPageCount;
	LONG									m_lCaretPos;

	TextBox(HWND hWnd);

	VOID Draw(HWND hwnd, HDC hdc, PAINTSTRUCT* ps);
	//For IME CandidateWnd
	VOID GetCompExt(RECT* rect);

	VOID onChar(WPARAM wParam, LPARAM lParam);
	VOID onKeyDown(WPARAM wParam, LPARAM lParam);
	VOID onKeyUp(WPARAM wParam, LPARAM lParam);
};

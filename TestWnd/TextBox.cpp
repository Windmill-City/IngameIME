#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#include "TextBox.h"
TextBox::TextBox(HWND hWnd)
{
	m_rectComp = *new RECT();
	m_lCaretPos = 0;
	m_lPageCount = 0;
	m_hWnd = hWnd;
}

VOID TextBox::Draw(HWND hwnd, HDC hdc, PAINTSTRUCT* ps)
{
	using namespace Gdiplus;
	Gdiplus::Font fontText(L"Microsoft Yahei", 12);
	Graphics graphics(hdc);
	PointF origin(8.0f, 8.0f);
	m_rectComp.top = origin.Y;
	//should use Font height, because some IME draw CompStr themselves, when CompStr is Empty
	//so the candidate window wont cover the text
	m_rectComp.bottom = m_rectComp.top + fontText.GetHeight(&graphics);

	Gdiplus::SolidBrush BrushFront(Color(255, 34, 142, 230));
	Gdiplus::StringFormat format = Gdiplus::StringFormat::GenericTypographic();
	//Draw Text
	graphics.DrawString(m_wstrTextContent.c_str(), m_wstrTextContent.size(), &fontText, origin, &format, &BrushFront);
	//Measure Text
	RectF rectfText;
	graphics.MeasureString(m_wstrTextContent.c_str(), (INT)m_wstrTextContent.size(), &fontText, origin, &rectfText);
	//Map rect Comp
	m_rectComp.left = rectfText.X + rectfText.Width;
	//Draw Comp
	PointF originComp(m_rectComp.left, origin.Y);
	graphics.DrawString(m_wstrComposition.c_str(), m_wstrComposition.size(), &fontText, originComp, &format, &BrushFront);
	//Measure Comp
	RectF rectfComp;
	graphics.MeasureString(m_wstrComposition.c_str(), (INT)m_wstrComposition.size(), &fontText, originComp, &rectfComp);
	//Map Comp
	m_rectComp.right = m_rectComp.left + rectfComp.Width;
	RectF rectfCaret;
	std::wstring toMeasure = m_wstrComposition.substr(m_lCaretPos);
	graphics.MeasureString(toMeasure.c_str(), (INT)toMeasure.size(), &fontText, originComp, &rectfCaret);
	//Draw Caret
	graphics.FillRectangle(&BrushFront, (int)(m_rectComp.right - rectfCaret.Width), m_rectComp.top, 2, (int)fontText.GetHeight(&graphics));
	//Draw Candidates
	int yOffset = fontText.GetHeight(&graphics);
	for (size_t i = 0; i < m_lPageCount; i++)
	{
		std::wstring str = m_pCandidates[i];
		int size = str.size();
		origin.Y += yOffset;
		graphics.DrawString(str.c_str(), str.size(), &fontText, origin, &format, &BrushFront);
	}
}

VOID TextBox::GetCompExt(RECT* rect)
{
	InvalidateRect(m_hWnd, NULL, NULL);
	SetRect(rect, m_rectComp.left, m_rectComp.top, m_rectComp.right, m_rectComp.bottom);
}

VOID TextBox::onChar(WPARAM wParam, LPARAM lParam)
{
	WCHAR ch = wParam;
	if (ch > 255//WCHAR
		|| !iscntrl(ch))
		m_wstrTextContent += ch;
}

VOID TextBox::onKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_BACK && m_wstrTextContent.size() > 0)
		m_wstrTextContent.erase(m_wstrTextContent.size() - 1);
	InvalidateRect(m_hWnd, NULL, NULL);
}

VOID TextBox::onKeyUp(WPARAM wParam, LPARAM lParam)
{
	InvalidateRect(m_hWnd, NULL, NULL);
}
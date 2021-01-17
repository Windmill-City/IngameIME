// TestWnd.cpp : 定义应用程序的入口点。
//
#include "../IngameIME/IMM.cpp"
#include "../IngameIME/TSF.cpp"
#include "framework.h"
#include "TestWnd.h"
#include "TextBox.h"
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)

#define BOOLSTR(x) (x == 1 ? L"TRUE" : L"FALSE")
#define DrawStr(x) graphics.DrawString(x.c_str(), x.size(), &fontText, origin, &format, &BrushFront);origin.Y += yOffset;

#define MAX_LOADSTRING 100

// 全局变量:
HWND hWnd;
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
TextBox* textBox;
IngameIME::BaseIME* api = 
//new IngameIME::IMM();
new IngameIME::TSF();

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void CALLBACK onCandidateList(libtf::CandidateList* list) {
	textBox->m_lPageCount = list->m_lPageSize;
	textBox->m_pCandidates = list->m_pCandidates;
}

void CALLBACK onComposition(libtf::CompositionEventArgs* args) {
	switch (args->m_state)
	{
	case libtf::CompositionState::StartComposition:
	case libtf::CompositionState::EndComposition:
		textBox->m_wstrComposition.clear();
		textBox->m_lCaretPos = 0;
		break;
	case libtf::CompositionState::Commit:
		textBox->m_wstrTextContent += args->m_strCommit;
		break;
	case libtf::CompositionState::Composing:
		textBox->m_wstrComposition = args->m_strComposition;
		textBox->m_lCaretPos = args->m_lCaretPos;
		break;
	default:
		break;
	}
}

void CALLBACK onGetTextExt(PRECT prect) {
	textBox->GetCompExt(prect);//Pos the CandidateList window, should return a bounding box of the composition string
}

void CALLBACK onAlphaMode(BOOL isAlphaMode) {
	//notify if ime in Alphanumeric input mode
	InvalidateRect(hWnd, NULL, NULL);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	//Must on STA for TSF
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr))
	{
		return FALSE;
	}
	//Reg callbacks
	api->m_sigAlphaMode = onAlphaMode;
	api->m_sigComposition = onComposition;
	api->m_sigCandidateList = onCandidateList;
	api->m_sigGetTextExt = onGetTextExt;

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_TESTWND, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTWND));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	CoUninitialize();
	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTWND));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TESTWND);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ULONG_PTR gdiplustoken;
	Gdiplus::GdiplusStartupInput gdiplusstartupinput;
	Gdiplus::GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, NULL);

	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	//Init here
	api->Initialize(hWnd);
	textBox = new TextBox(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		int scrWidth, scrHeight;
		RECT rect;
		//获得屏幕尺寸
		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 500, 450, SWP_SHOWWINDOW);
		//取得窗口尺寸
		GetWindowRect(hWnd, &rect);
		//重新设置rect里的值
		rect.left = (scrWidth - rect.right) / 2;
		rect.top = (scrHeight - rect.bottom) / 2;
		//移动窗口到指定的位置
		SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_STATE:
			api->setState(!api->State());
			break;
		case IDM_FULLSC:
			api->setFullScreen(!api->FullScreen());
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		InvalidateRect(hWnd, NULL, NULL);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		using namespace Gdiplus;
		RECT rect;
		GetClientRect(hWnd, &rect);
		Graphics graphics(hdc);
		SolidBrush brush_white(Color(255, 255, 255, 255));

		graphics.FillRectangle(&brush_white, 0, 0, rect.right, rect.bottom);
		if (textBox) textBox->Draw(hWnd, hdc, &ps);
		//Draw State
		std::wstring IMEState = L"IMEState:";
		IMEState += BOOLSTR(api->State());
		std::wstring FullScreen = L"FullScreen:";
		FullScreen += BOOLSTR(api->FullScreen());
		std::wstring AlphaMode = L"AlphaMode:";
		AlphaMode += BOOLSTR(api->m_alphaMode);

		using namespace Gdiplus;
		Gdiplus::Font fontText(L"Microsoft Yahei", 12);
		Gdiplus::SolidBrush BrushFront(Color(255, 34, 142, 230));
		Gdiplus::StringFormat format = Gdiplus::StringFormat::GenericTypographic();
		PointF origin(8.0f, 300.0f);
		int yOffset = fontText.GetHeight(&graphics);
		DrawStr(IMEState);
		DrawStr(FullScreen);
		DrawStr(AlphaMode);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CHAR:
		if (textBox) textBox->onChar(wParam, lParam);
		break;
	case WM_KEYDOWN:
		if (textBox) textBox->onKeyDown(wParam, lParam);
		break;
	case WM_KEYUP:
		if (textBox) textBox->onKeyUp(wParam, lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

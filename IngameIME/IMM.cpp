#include "pch.h"
#include <Imm.h>
#pragma comment(lib, "imm32.lib")
#include <functional>
#include "BaseIME.h"

namespace IngameIME {
	class __declspec(dllexport) IMM : public BaseIME
	{
		std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> m_fhandleWndMsg = std::bind(&IMM::handleWndMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		WNDPROC m_prevWndProc = NULL;
		HIMC m_context = NULL;
		BOOL m_enabled = FALSE;

		static LRESULT CALLBACK handleWndMsg_CStyle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			return getInstance()->m_fhandleWndMsg(hwnd, msg, wparam, lparam);
		}

		void handleComposition(WPARAM compFlag, LPARAM genrealFlag)
		{
			if (((GCS_COMPSTR | GCS_COMPATTR | GCS_COMPCLAUSE | GCS_COMPREADATTR | GCS_COMPREADCLAUSE | GCS_COMPREADSTR) & compFlag) && (m_fullscreen || m_handleCompStr))//Comp String/Sel Changed
			{
				//CompStr
				int size = ImmGetCompositionString(m_context, GCS_COMPSTR, NULL, 0) + sizeof(WCHAR);
				WCHAR* buf = new WCHAR[size / sizeof(WCHAR)];
				ZeroMemory(buf, size);
				ImmGetCompositionString(m_context, GCS_COMPSTR, buf, size);
				//CompSel
				int sel = ImmGetCompositionString(m_context, GCS_CURSORPOS, NULL, 0);
				onComposition((PWCH)buf, TRUE, sel);
			}
			if (GCS_RESULTSTR & genrealFlag)//Has commited
			{
				int size = ImmGetCompositionString(m_context, GCS_RESULTSTR, NULL, 0) + sizeof(WCHAR);
				WCHAR* buf = new WCHAR[size / sizeof(WCHAR)];
				ZeroMemory(buf, size);
				ImmGetCompositionString(m_context, GCS_RESULTSTR, buf, size);
				onComposition((PWCH)buf, FALSE, 0);
			}
		}

		void posCandWnd()
		{
			RECT* rect = new RECT();
			onGetCompExt(rect);
			POINT* InsertPos = new POINT();
			InsertPos->x = rect->left;
			InsertPos->y = rect->top;
			//Jap and Korea IME may use this, keep it
			CANDIDATEFORM* cand = new CANDIDATEFORM();
			cand->dwIndex = 0;
			cand->dwStyle = CFS_EXCLUDE;
			cand->ptCurrentPos = *InsertPos;
			cand->rcArea = *rect;
			ImmSetCandidateWindow(m_context, cand);
			COMPOSITIONFORM* comp = new COMPOSITIONFORM();
			comp->dwStyle = CFS_POINT;//If use CFS_RECT, Chinese IME wont set comp text for some start words
			comp->ptCurrentPos = *InsertPos;
			ImmSetCompositionWindow(m_context, comp);
		}

		void handleCandlist()
		{
			size_t size = ImmGetCandidateList(m_context, 0, NULL, 0);

			DWORD pageSize = 0;
			WCHAR* candStr = NULL;
			std::wstring* cand = NULL;

			if (size) {
				LPCANDIDATELIST candlist = (LPCANDIDATELIST)new byte[size];
				size = ImmGetCandidateList(m_context, 0, candlist, size);

				pageSize = candlist->dwPageSize;
				DWORD candCount = candlist->dwCount;//if greater than one, then contain vaild strings

				if (pageSize > 0 && candCount > 1) {
					DWORD pageStart = candlist->dwPageStart;
					DWORD pageEnd = pageStart + pageSize;

					cand = new std::wstring[pageSize];

					for (size_t i = 0; i < pageSize; i++)
					{
						LONG_PTR pStrStart = (LONG_PTR)candlist + candlist->dwOffset[i];
						LONG_PTR pStrEnd = (LONG_PTR)candlist + (i + 1 < candCount ? candlist->dwOffset[i + 1] : size);
						auto len = pStrEnd - pStrStart;

						candStr = (WCHAR*)new byte[len];
						memcpy(candStr, (void*)pStrStart, len);

						cand[i] = candStr;
					}
				}
			}
			onCandidateList(cand, pageSize);
		}

		LRESULT handleWndMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			switch (msg)
			{
			case WM_GETDLGCODE:
				//Return this to make IME get the keys
				return DLGC_WANTALLKEYS;
				//When init window, we will recive this Msg
				//If we want IME input, we need to AccrociateContext
			case WM_INPUTLANGCHANGE:
				ImmAssociateContext(hwnd, m_context);
				goto Handled;

				//SETCONTEXT should be handled by IME, we can control IMEs UI(Candidate Window, etc) by changing the lParam
				//See:https://docs.microsoft.com/en-us/windows/win32/intl/wm-ime-setcontext
				//we need to associatecontext, for activate IME
			case WM_IME_SETCONTEXT:
				if (wparam && State())//wParam == 1 means window active otherwise not
					ImmAssociateContext(hwnd, m_context);

				if (m_fullscreen)
				{
					lparam &= ~ISC_SHOWUICANDIDATEWINDOW;
				}
				if (m_handleCompStr || m_fullscreen) {
					lparam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
				}
				break;

			case WM_IME_COMPOSITION:
				handleComposition(wparam, lparam);
				if (!m_fullscreen) posCandWnd(); else handleCandlist();//partly commit will missing a IMN_CHANGECANDIDATE msg, so we fetch here
				if (!m_handleCompStr && !m_fullscreen) break;
				goto Handled;

			case WM_IME_STARTCOMPOSITION:
				if (!m_fullscreen || !m_handleCompStr) break;
				onComposition(NULL, TRUE, 0);
				goto Handled;
			case WM_IME_ENDCOMPOSITION:
				if (!m_fullscreen || !m_handleCompStr) break;
				//Clear CompStr
				onComposition(NULL, FALSE, 0);
				//Clear candidates
				onCandidateList(NULL, NULL);
				//if we handle and draw the comp text
				//we dont pass this msg to the next
				goto Handled;
			case WM_IME_CHAR:
				//in case duplicate commit
				goto Handled;

			case WM_IME_NOTIFY:
				switch (wparam)
				{
				case IMN_CHANGECANDIDATE:
					if (!m_fullscreen) break;
					handleCandlist();
					goto Handled;
				case IMN_OPENCANDIDATE:
				case IMN_CLOSECANDIDATE:
					if (!m_fullscreen) break;
					onCandidateList(NULL, NULL);
					goto Handled;
				case IMN_SETCONVERSIONMODE:
					DWORD dwConversion;
					ImmGetConversionStatus(m_context, &dwConversion, NULL);
					m_alphaMode = !(dwConversion & IME_CMODE_NATIVE);
					onAlphaMode(m_alphaMode);
				default:
					break;
				}

			default:
				break;
			}
			return CallWindowProc(m_prevWndProc, hwnd, msg, wparam, lparam);
		Handled:
			return NULL;
		}

		IMM() {}
	public:
		static IMM* getInstance() {
			static IMM _instance;
			return &_instance;
		}

		void Initialize(HWND hWnd)
		{
			m_hWnd = hWnd;
			m_context = ImmGetContext(hWnd);
			if (!m_context) m_context = ImmCreateContext();
			m_prevWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)handleWndMsg_CStyle);
			setState(m_enabled);
			m_initialized = TRUE;
		}

		LONG_PTR Uninitialize()
		{
			if (m_initialized) {
				setState(FALSE);
				ImmDestroyContext(m_context);
				m_initialized = FALSE;
				return SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_prevWndProc);
			}
			return NULL;
		}

		void setState(BOOL enable)
		{
			m_enabled = enable;
			if (m_enabled)
				ImmAssociateContext(m_hWnd, m_context);
			else
				ImmAssociateContext(m_hWnd, NULL);
		}

		virtual BOOL State() override
		{
			return m_enabled;
		}
	};
}
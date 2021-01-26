#include "pch.h"
#include <Imm.h>
#pragma comment(lib, "imm32.lib")
#include "BaseIME.h"
namespace IngameIME {
	std::map<HWND, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>> g_hWndProcs;
	LRESULT g_handleWndMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		auto iter = g_hWndProcs.find(hwnd);
		if (iter != g_hWndProcs.end()) {
			return iter->second(hwnd, msg, wparam, lparam);
		}
		return NULL;
	}
	class IngameIME_API IMM : public BaseIME
	{
		HIMC												m_context = NULL;
		BOOL												m_enabled = FALSE;
		BOOL												m_handleCompStr = TRUE;
		BOOL												m_fullscreen = FALSE;
		libtf::CandidateList*								m_CandidateList = new libtf::CandidateList();

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
				m_sigComposition(new libtf::CompositionEventArgs(buf, sel));
			}
			if (GCS_RESULTSTR & genrealFlag)//Has commited
			{
				int size = ImmGetCompositionString(m_context, GCS_RESULTSTR, NULL, 0) + sizeof(WCHAR);
				WCHAR* buf = new WCHAR[size / sizeof(WCHAR)];
				ZeroMemory(buf, size);
				ImmGetCompositionString(m_context, GCS_RESULTSTR, buf, size);
				m_sigComposition(new libtf::CompositionEventArgs(buf));
			}
		}

		void posCandWnd()
		{
			RECT* rect = new RECT();
			m_sigGetTextExt(rect);
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
			m_CandidateList->Reset();
			DWORD size = ImmGetCandidateList(m_context, 0, NULL, 0);

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
						LONG_PTR pStrStart = (LONG_PTR)candlist + candlist->dwOffset[i + pageStart];
						LONG_PTR pStrEnd = (LONG_PTR)candlist + (i + pageStart + 1 < candCount ? candlist->dwOffset[i + pageStart + 1] : size);
						auto len = pStrEnd - pStrStart;

						candStr = (WCHAR*)new byte[len];
						memcpy(candStr, (void*)pStrStart, len);

						cand[i] = candStr;
					}
					m_CandidateList->m_lPageSize = pageSize;
					m_CandidateList->m_pCandidates.reset(cand);
				}
			}
			m_sigCandidateList(m_CandidateList);
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
				m_sigComposition(new libtf::CompositionEventArgs(libtf::CompositionState::StartComposition));
				goto Handled;
			case WM_IME_ENDCOMPOSITION:
				if (!m_fullscreen || !m_handleCompStr) break;
				//Clear CompStr
				m_sigComposition(new libtf::CompositionEventArgs(libtf::CompositionState::StartComposition));
				//Clear candidates
				m_CandidateList->Reset();
				m_sigCandidateList(m_CandidateList);
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
					m_CandidateList->Reset();
					m_sigCandidateList(m_CandidateList);
					goto Handled;
				case IMN_SETCONVERSIONMODE:
					DWORD dwConversion;
					ImmGetConversionStatus(m_context, &dwConversion, NULL);
					m_alphaMode = !(dwConversion & IME_CMODE_NATIVE);
					m_sigAlphaMode(m_alphaMode);
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
	public:
		void Initialize(HWND hWnd)
		{
			m_hWnd = hWnd;
			m_context = ImmGetContext(hWnd);
			if (!m_context) m_context = ImmCreateContext();
			g_hWndProcs[hWnd] = m_fhandleWndMsg;
			m_prevWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)g_handleWndMsg);
			setState(m_enabled);
			m_initialized = TRUE;
		}

		LONG_PTR Uninitialize()
		{
			if (m_initialized) {
				setState(FALSE);
				ImmDestroyContext(m_context);
				g_hWndProcs.erase(m_hWnd);
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

		BOOL State() override
		{
			return m_enabled;
		}

		void setFullScreen(BOOL fullscreen) override
		{
			m_fullscreen = fullscreen;
			if (m_enabled) {
				setState(FALSE);
				setState(TRUE);
			}
		}

		BOOL FullScreen() override
		{
			return m_fullscreen;
		}
};
}
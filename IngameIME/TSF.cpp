#include "pch.h"
#include "BaseIME.h"
#include "../libtf/libtf/Application.hpp"
#include "../libtf/libtf/Document.hpp"
namespace IngameIME {
	std::map<HWND, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>> g_tf_hWndProcs;
	LRESULT g_tf_handleWndMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		auto iter = g_tf_hWndProcs.find(hwnd);
		if (iter != g_tf_hWndProcs.end()) {
			return iter->second(hwnd, msg, wparam, lparam);
		}
		return NULL;
	}
	class IngameIME_API TSF : public BaseIME
	{
		CComPtr<libtf::Application>							m_Application;
		CComPtr<libtf::Document>							m_Document;

		LRESULT handleWndMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			switch (msg)
			{
			case WM_SETFOCUS:
				m_Document->Focus();
				break;
			default:
				break;
			}
			return CallWindowProc(m_prevWndProc, hwnd, msg, wparam, lparam);
		}
	public:
		void Initialize(HWND hWnd) override
		{
			m_Application = new libtf::Application();
			m_Application->Initialize();
			m_Document = new libtf::Document(m_Application->m_pThreadMgr, m_Application->m_ClientId, hWnd);
			m_Document->Focus();

			m_Application->m_sigAlphaMode = [this](BOOL alphaMode) { 
				this->m_alphaMode = alphaMode;
				this->m_sigAlphaMode(m_alphaMode);
			};
			m_Document->m_sigComposition = [this](libtf::CompositionEventArgs* args) {
				this->m_sigComposition(args);
			};
			m_Document->m_sigGetTextExt = [this](LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) {
				this->m_sigGetTextExt(prc);
			};
			m_Application->m_pCandidateListHandler->m_sigCandidateList = [this](libtf::CandidateList* list) {
				this->m_sigCandidateList(list);
			};

			setState(FALSE);

			g_tf_hWndProcs[hWnd] = m_fhandleWndMsg;
			m_prevWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)g_tf_handleWndMsg);
			m_initialized = TRUE;
		}

		LONG_PTR Uninitialize() override
		{
			if (m_initialized) {
				m_Document.Release();
				m_Application.Release();
				g_tf_hWndProcs.erase(m_hWnd);
				m_initialized = FALSE;
				return SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_prevWndProc);
			}
			return NULL;
		}

		void setState(BOOL state) override
		{
			m_Application->setKeyStrokeFeedState(state);
			if (!state) m_Document->TerminateComposition();
		}

		BOOL State() override
		{
			return m_Application->KeyStrokeFeedState();
		}

		void setFullScreen(BOOL fullscreen) override
		{
			m_Application->m_pCandidateListHandler->m_fhandleCandidate = fullscreen;
		}

		BOOL FullScreen() override
		{
			return m_Application->m_pCandidateListHandler->m_fhandleCandidate;
		}
	};
}

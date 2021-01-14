#include "pch.h"
#include "BaseIME.h"
#include "../libtf/libtf/Application.hpp"
#include "../libtf/libtf/Document.hpp"
namespace IngameIME {
	class IngameIME_API TSF : public BaseIME
	{
		CComPtr<libtf::Application> m_Application;
		CComPtr<libtf::Document>	m_Document;
	public:
		void Initialize(HWND hWnd) override
		{
			m_Application = new libtf::Application();
			m_Application->Initialize();
			m_Document = new libtf::Document(m_Application->m_pThreadMgr, m_Application->m_ClientId, hWnd);

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
		}

		LONG_PTR Uninitialize() override
		{
			m_Document.Release();
			m_Application.Release();
			return NULL;
		}

		void setState(BOOL state) override
		{
			m_Document->SetKeyboardState(state);
		}

		BOOL State() override
		{
			return m_Document->KeyboardState();
		}

		void setFullScreen(BOOL fullscreen) override
		{
			m_Application->m_pCandidateListHandler->m_handleCandidate = fullscreen;
		}

		BOOL FullScreen() override
		{
			return m_Application->m_pCandidateListHandler->m_handleCandidate;
		}
	};
}

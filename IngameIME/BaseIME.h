#pragma once
#include <wtypes.h>
#include <xstring>

namespace IngameIME {
	typedef void (CALLBACK* CANDPROC)(byte*, DWORD*, size_t);
	typedef void (CALLBACK* COMPPROC)(PWCHAR, BOOL, INT);//BOOL->TRUE = START/UPDATE FALSE = END/COMMIT,INT->Caret Pos
	typedef void(CALLBACK* COMPEXTPROC)(PRECT);//CandidateWnd's pos
	class __declspec(dllexport) BaseIME
	{
	public:
		BOOL m_initialized = FALSE;
		HWND m_hWnd = NULL;
		BOOL m_fullscreen = FALSE;
		BOOL m_handleCompStr = FALSE;

		CANDPROC onCandidateList = NULL;
		COMPPROC onComposition = NULL;
		COMPEXTPROC onGetCompExt = NULL;

		virtual void Initialize(HWND) = 0;
		virtual LONG Uninitialize() = 0;
		virtual void setState(BOOL) = 0;
		virtual BOOL State() = 0;
	};
}


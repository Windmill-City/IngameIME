#pragma once
#include <wtypes.h>
#include <xstring>

namespace IngameIME {
	typedef void (CALLBACK* CANDPROC)(byte*, DWORD*, size_t);
	typedef void (CALLBACK* COMPPROC)(PWCHAR, BOOL, INT);//BOOL->TRUE = START/UPDATE FALSE = END/COMMIT,INT->Caret Pos
	typedef void (CALLBACK* COMPEXTPROC)(PRECT);//Composition's bounding box, use to pos candidate window by ime
	typedef void (CALLBACK* ALPHAMODEPROC)(BOOL);//isAlphaMode, indicate if in Alphanumeric input mode
	class __declspec(dllexport) BaseIME
	{
	public:
		BOOL m_initialized = FALSE;
		HWND m_hWnd = NULL;
		BOOL m_fullscreen = FALSE;
		BOOL m_handleCompStr = FALSE;
		BOOL m_alphaMode = FALSE;

		CANDPROC onCandidateList = NULL;
		COMPPROC onComposition = NULL;
		COMPEXTPROC onGetCompExt = NULL;
		ALPHAMODEPROC onAlphaMode = NULL;

		virtual void Initialize(HWND) = 0;
		virtual LONG_PTR Uninitialize() = 0;
		virtual void setState(BOOL) = 0;
		virtual BOOL State() = 0;
	};
}


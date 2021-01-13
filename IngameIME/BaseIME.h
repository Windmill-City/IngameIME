#pragma once
#include <functional>
#include <wtypes.h>
#include <xstring>

namespace IngameIME {
	typedef std::function<void(std::wstring*, size_t)> CANDPROC;
	typedef std::function<void(PWCHAR, BOOL, INT)> COMPPROC;//BOOL->TRUE = START/UPDATE FALSE = END/COMMIT,INT->Caret Pos
	typedef std::function<void(PRECT)> COMPEXTPROC;//Composition's bounding box, use to pos candidate window by ime
	typedef std::function<void(BOOL)> ALPHAMODEPROC;//isAlphaMode, indicate if in Alphanumeric input mode
	class __declspec(dllexport) BaseIME
	{
	public:
		BOOL m_initialized = FALSE;
		HWND m_hWnd = NULL;
		BOOL m_fullscreen = FALSE;
		BOOL m_handleCompStr = FALSE;
		BOOL m_alphaMode = FALSE;

		CANDPROC onCandidateList = [](std::wstring*, size_t) {};
		COMPPROC onComposition = [](PWCHAR, BOOL, INT) {};
		COMPEXTPROC onGetCompExt = [](PRECT) {};
		ALPHAMODEPROC onAlphaMode = [](BOOL) {};

		virtual void Initialize(HWND) = 0;
		virtual LONG_PTR Uninitialize() = 0;
		virtual void setState(BOOL) = 0;
		virtual BOOL State() = 0;
	};
}


#pragma once
#include <wtypes.h>
#include <xstring>

namespace IngameIME {
	typedef VOID (CALLBACK* CANDPROC)(byte*, DWORD*, size_t);
	typedef VOID (CALLBACK* COMPPROC)(PWCHAR, BOOL, INT);//BOOL->TRUE = START/UPDATE FALSE = END/COMMIT,INT->Caret Pos
	typedef VOID(CALLBACK* COMPEXTPROC)(PRECT);//CandidateWnd's pos
	class __declspec(dllexport) BaseIME
	{
	public:
		BOOL m_initialized = FALSE;
		HWND m_hWnd = NULL;
		BOOL m_fullscreen = FALSE;
		BOOL m_enabled = FALSE;
		BOOL m_handleCompStr = FALSE;

		CANDPROC onCandidateList = NULL;
		COMPPROC onComposition = NULL;
		COMPEXTPROC onGetCompExt = NULL;

		virtual VOID Initialize(HWND) = 0;
		virtual LONG Uninitialize() = 0;
		virtual VOID setState(BOOL) = 0;
		virtual BOOL State() {
			return m_enabled;
		}
		virtual CANDPROC setCandProc(CANDPROC proc) {
			CANDPROC tmp = onCandidateList;
			onCandidateList = proc;
			return tmp;
		}
		virtual COMPPROC setCompProc(COMPPROC proc) {
			COMPPROC tmp = onComposition;
			onComposition = proc;
			return tmp;
		}
		virtual COMPEXTPROC setCompExtProc(COMPEXTPROC proc) {
			COMPEXTPROC tmp = onGetCompExt;
			onGetCompExt = proc;
			return tmp;
		}
	};
}


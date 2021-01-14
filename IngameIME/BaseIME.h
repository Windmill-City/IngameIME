#pragma once
#include <functional>
#include <wtypes.h>
#include <xstring>
#include "../libtf/libtf/CompositionEventArgs.hpp"
#include "../libtf/libtf/CandidateListHandler.hpp"
#define IngameIME_API __declspec(dllexport)
namespace IngameIME {
	typedef std::function < VOID(libtf::CompositionEventArgs*)>		sig_Composition;
	typedef std::function <VOID(RECT* prc)>							sig_GetTextExt;
	typedef std::function<VOID(libtf::CandidateList* list)>			sig_CandidateList;
	typedef std::function<VOID(BOOL)>								sig_AlphaMode;
	class IngameIME_API BaseIME
	{
	public:
		BOOL												m_initialized = FALSE;
		HWND												m_hWnd = NULL;
		BOOL												m_alphaMode = FALSE;

		sig_CandidateList									m_sigCandidateList = [](libtf::CandidateList* list) {};
		sig_Composition										m_sigComposition = [](libtf::CompositionEventArgs*) {};
		sig_GetTextExt										m_sigGetTextExt = [](RECT* prc) {};
		sig_AlphaMode										m_sigAlphaMode = [](BOOL) {};

		virtual void Initialize(HWND) = 0;
		virtual LONG_PTR Uninitialize() = 0;
		virtual void setState(BOOL) = 0;
		virtual BOOL State() = 0;
		virtual void setFullScreen(BOOL) = 0;
		virtual BOOL FullScreen() = 0;
	};
}


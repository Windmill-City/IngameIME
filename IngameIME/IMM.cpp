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

        static LRESULT handleWndMsg_CStyle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
            return getInstance()->m_fhandleWndMsg(hwnd, msg, wparam, lparam);
        }

        void handleComposition(LONG compFlag, LONG genrealFlag)
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
                if (onComposition) onComposition((PWCH)buf, TRUE, sel);
            }
            if (GCS_RESULTSTR & genrealFlag)//Has commited
            {
                int size = ImmGetCompositionString(m_context, GCS_RESULTSTR, NULL, 0) + sizeof(WCHAR);
                WCHAR* buf = new WCHAR[size / sizeof(WCHAR)];
                ZeroMemory(buf, size);
                ImmGetCompositionString(m_context, GCS_RESULTSTR, buf, size);
                if (onComposition) onComposition((PWCH)buf, FALSE, 0);
            }
        }

        void posCandWnd()
        {
            RECT* rect = new RECT();
            if (onGetCompExt) onGetCompExt(rect);
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
            LPCANDIDATELIST candlist = (LPCANDIDATELIST)new byte[size];
            size = ImmGetCandidateList(m_context, 0, candlist, size);

            DWORD pageSize = candlist->dwPageSize;

            byte* candStr = NULL;
            DWORD* candStrLen = NULL;
            if (pageSize > 0) {
                DWORD candCount = candlist->dwCount;
                DWORD pageStart = candlist->dwPageStart;
                DWORD pageEnd = pageStart + pageSize;

                DWORD pStrStart = (DWORD)candlist + (DWORD)candlist->dwOffset[pageStart];
                DWORD pStrEnd = (DWORD)candlist + (pageEnd < candCount ? candlist->dwOffset[pageEnd] : size);

                auto len = pStrEnd - pStrStart;
                candStr = new byte[len];
                candStrLen = new DWORD[pageSize];
                memcpy(candStr, (void*)pStrStart, len);
                for (size_t i = 0; i < pageSize; i++)
                {
                    auto offStart = candlist->dwOffset[pageStart + i];
                    auto offEnd = pageStart + i + 1 < candCount ? candlist->dwOffset[pageStart + i + 1] : size;
                    candStrLen[i] = offEnd - offStart;
                }
            }
            if (onCandidateList) onCandidateList(candStr, candStrLen, pageSize);
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

                ImmSetOpenStatus(m_context, m_enabled);

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
                if (!m_fullscreen) posCandWnd();
                if (!m_handleCompStr && !m_fullscreen) break;
                goto Handled;

            case WM_IME_STARTCOMPOSITION:
                if (!m_fullscreen || !m_handleCompStr) break;
                if (onComposition) onComposition(NULL, TRUE, 0);
                goto Handled;
            case WM_IME_ENDCOMPOSITION:
                if (!m_fullscreen || !m_handleCompStr) break;
                //Clear CompStr
                if (onComposition) onComposition(NULL, FALSE, 0);
                //Clear candidates
                if (onCandidateList) onCandidateList(NULL, NULL, 0);
                //if we handle and draw the comp text
                //we dont pass this msg to the next
                goto Handled;
            case WM_IME_CHAR:
                //in case duplicate commit
                goto Handled;

            case WM_IME_NOTIFY:
                if (!m_fullscreen) break;
                switch (wparam)
                {
                case IMN_CHANGECANDIDATE:
                    handleCandlist();
                    break;
                case IMN_OPENCANDIDATE:
                case IMN_CLOSECANDIDATE:
                    if (onCandidateList) onCandidateList(NULL, NULL, 0);
                    break;
                default:
                    break;
                }
                goto Handled;

            default:
                break;
            }
            return CallWindowProc(m_prevWndProc, hwnd, msg, wparam, lparam);
        Handled:
            return NULL;
        }

        IMM(){}
    public:
        static IMM* getInstance() {
            static IMM _instance;
            return &_instance;
        }

        void Initialize(HWND hWnd)
        {
            m_hWnd = hWnd;
            m_context = ImmGetContext(hWnd);
            m_prevWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)handleWndMsg_CStyle);
            setState(m_enabled);
            m_initialized = TRUE;
        }

        LONG Uninitialize()
        {
            if (m_initialized) {
                setState(FALSE);
                ImmDestroyContext(m_context);
                return SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG)m_prevWndProc);
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
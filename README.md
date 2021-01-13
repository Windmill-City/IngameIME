# IngameIME
Enable IME in FullScreen games
# Example
![Change IME State](https://github.com/Windmill-City/IngameIME/blob/master/Docs/IMEState.gif)
![Handle Composition String](https://github.com/Windmill-City/IngameIME/blob/master/Docs/CompStr.gif)
![FullScreen Mode](https://github.com/Windmill-City/IngameIME/blob/master/Docs/FullSC.gif)
![Alpha Mode](https://github.com/Windmill-City/IngameIME/blob/master/Docs/AlphaMode.gif)
# How to use
Get an api instance first
```c++
IngameIME::BaseIME* api = IngameIME::IMM::getInstance();
```
Set up callbacks
```c++
void CALLBACK onCandidateList(std::wstring* candStr ,size_t size) {
	textBox->Count = size;
	textBox->Candidates.reset(candStr);
}

void CALLBACK onComposition(PWCHAR pstr, BOOL state, INT caret) {
    if (state && pstr) {//Update Composition String
        textBox->m_CompText = pstr;
        textBox->m_CaretPos = caret;
    }else if(!state && pstr)//Commit String
        textBox->m_Text += pstr;
    else//End Composition
    {
        textBox->m_CompText.clear();
        textBox->m_CaretPos = 0;
    }
}

void CALLBACK onGetCompExt(PRECT prect) {
    textBox->GetCompExt(prect);//Pos the CandidateList window, should return a bounding box of the composition string
}

void CALLBACK onAlphaMode(BOOL isAlphaMode) {
	//notify if ime in Alphanumeric input mode
	InvalidateRect(hWnd, NULL, NULL);
}
```
Register in the api
```c++
   api->onCandidateList = onCandidateList;
   api->onComposition = onComposition;
   api->onGetCompExt = onGetCompExt;
   api->onAlphaMode = onAlphaMode;
```
Then init by passing a HWND ptr
```c++
   api->Initialize(hWnd);
```
Change IME State
```c++
api->setState(TRUE/FALSE);
api->m_fullscreen = TRUE/FALSE;//Change FullScreen mode need to setState(FALSE) then setState(TRUE) to refresh it
api->m_handleCompStr = TRUE/FALSE;
```

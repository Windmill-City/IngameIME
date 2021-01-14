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
void CALLBACK onCandidateList(libtf::CandidateList* list) {
	textBox->m_lPageCount = list->m_lPageSize;
	textBox->m_pCandidates = list->m_pCandidates;
}

void CALLBACK onComposition(libtf::CompositionEventArgs* args) {
	switch (args->m_state)
	{
	case libtf::CompositionState::StartComposition:
	case libtf::CompositionState::EndComposition:
		textBox->m_wstrComposition.clear();
		textBox->m_lCaretPos = 0;
		break;
	case libtf::CompositionState::Commit:
		textBox->m_wstrTextContent += args->m_strCommit;
		break;
	case libtf::CompositionState::Composing:
		textBox->m_wstrComposition = args->m_strComposition;
		textBox->m_lCaretPos = args->m_lCaretPos;
		break;
	default:
		break;
	}
}

void CALLBACK onGetTextExt(PRECT prect) {
	textBox->GetCompExt(prect);//Pos the CandidateList window, should return a bounding box of the composition string
}

void CALLBACK onAlphaMode(BOOL isAlphaMode) {
	//notify if ime in Alphanumeric input mode
	InvalidateRect(hWnd, NULL, NULL);
}
```
Register in the api
```c++
	api->m_sigAlphaMode = onAlphaMode;
	api->m_sigComposition = onComposition;
	api->m_sigCandidateList = onCandidateList;
	api->m_sigGetTextExt = onGetTextExt;
```
Then init by passing a HWND ptr
```c++
   api->Initialize(hWnd);
```
Change IME State
```c++
api->setState(TRUE/FALSE);
api->setFullScreen(TRUE/FALSE);
```

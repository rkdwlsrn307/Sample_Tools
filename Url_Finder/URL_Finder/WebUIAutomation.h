#pragma once

class WebUIAutomtaion
{
private:
	IUIAutomation* m_automation;
	IUIAutomationCondition* m_docCondition;
	IUIAutomationCondition* m_addressAccCondition;
	IUIAutomationCondition* m_addressAccelateCondition;
	IUIAutomationCondition* m_addressAccAllCondition;

private:
	HWINEVENTHOOK m_hook;
	BOOL m_proc_init_thread;
	BOOL m_event_init_thread;
	DWORD m_pid;
	std::map<std::wstring, DWORD>m_browser_map;
	std::map<std::wstring, HWINEVENTHOOK>m_hook_map;

	typedef struct _WIDGET_CHILD_PARAM
	{
		WebUIAutomtaion* _this;
		std::vector<HWND>* renderList;
	} WIDGET_CHILD_PARAM, * PWIDGET_CHILD_PARAM;

private:
	void Finalize();
	void InitializeAutomation();
	void FinalizeAutomation();
	void ResetEventHook();
	void InitEventHook();
	void ProcessInitThread();
	void DoProcessUpdateThread();
	void DoWinEventThread();
	void DoWinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime);
	void SetPid();

	BOOL IsFilterEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime);
	void CheckUrlEvent(IAccessible* acc, DWORD event, VARIANT child, HWND hwnd);
	BOOL ChromeRenderEvent(VARIANT role, VARIANT parentrole, std::string renderurl);
	BOOL UpdateParentRole(IAccessible* acc, VARIANT child, VARIANT &parentrole);
	BOOL ChromeLinkEvent(HWND hwnd);
	BOOL ChromeWidgetEvent(HWND hwnd);
	BOOL GetRenderFromWidget(HWND widget, std::vector<HWND>& renderList);
	BOOL DoEnumWidgetChild(HWND hwnd, std::vector<HWND>& list);
	BOOL UpdateWidgetEvent(std::vector<HWND>& renderHandle);
	std::string GetUrlFromHwnd(HWND hwnd);
	std::string GetAddressEdit(HWND hwnd);
	std::string GetAddressEditFromCondition(IUIAutomationElement* target,
		IUIAutomationCondition* condition);

private:
	static void WINAPI ProcessUpdateThread();
	static void CALLBACK WinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime);
	static void WINAPI WinEventThread();
	static BOOL CALLBACK EnumWidgetChild(HWND hwnd, LPARAM param);

public:
	void Initialize();
	DWORD GetPid() { return m_pid; }

	WebUIAutomtaion();
	~WebUIAutomtaion();

public:
	static WebUIAutomtaion& Get()
	{
		static WebUIAutomtaion instance;

		return instance;
	}
};
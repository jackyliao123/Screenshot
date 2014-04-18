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
			if (selectRect.valid){
				selectRect.valid = false;
			}
			else{
				DeleteObject(hbitmap);
				hbitmap = NULL;
				delete[] capturePixels;
				DeleteObject(buffer);
				ShowWindow(hwnd, SW_HIDE);
			}
		}
		break;
	}
}

void paintToBuffer(){
	memcpy(pixels, capturePixels, bufferWidth * bufferHeight * 4);
	for (unsigned int i = 0; i < bufferWidth * bufferHeight; i++){
		pixels[i].r >>= 1;
		pixels[i].g >>= 1;
		pixels[i].b >>= 1;
	}
	if (selectRect.valid){
		for (unsigned int i = selectRect.y; i < selectRect.y + selectRect.height; i++){
			memcpy(&pixels[i * bufferWidth + selectRect.x], &capturePixels[i * bufferWidth + selectRect.x], selectRect.width * 4);
		}
		for (int i = selectRect.x - 1; i < selectRect.x + selectRect.width + 1; i++){
			setPixel(i, selectRect.y - 1, SELECTION_COLOR);
			setPixel(i, selectRect.y, SELECTION_COLOR);
			setPixel(i, selectRect.y + 1, SELECTION_COLOR);
			setPixel(i, selectRect.y + selectRect.height - 1, SELECTION_COLOR);
			setPixel(i, selectRect.y + selectRect.height, SELECTION_COLOR);
			setPixel(i, selectRect.y + selectRect.height + 1, SELECTION_COLOR);
		}
		for (int i = selectRect.y - 1; i < selectRect.y + selectRect.height + 1; i++){
			setPixel(selectRect.x - 1, i, SELECTION_COLOR);
			setPixel(selectRect.x, i, SELECTION_COLOR);
			setPixel(selectRect.x + 1, i, SELECTION_COLOR);
			setPixel(selectRect.x + selectRect.width - 1, i, SELECTION_COLOR);
			setPixel(selectRect.x + selectRect.width, i, SELECTION_COLOR);
			setPixel(selectRect.x + selectRect.width + 1, i, SELECTION_COLOR);
		}
	}
}

void setPixel(int x, int y, int color){
	if (x >= 0 && x < bufferWidth && y >= 0 && y < bufferHeight) {
		pixels[x + y * bufferWidth].val = color;
	}
}

void createBuffer(){
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biWidth = rect.right - rect.left;
	bmi.bmiHeader.biHeight = rect.top - rect.bottom;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(hwnd);
	bufferWidth = rect.right - rect.left;
	bufferHeight = rect.bottom - rect.top;
	buffer = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
	
	capturePixels = new pixel[bufferWidth * bufferHeight];
	GetDIBits(hdc, hbitmap, 0, bufferHeight, capturePixels, &bmi, 0);

	DeleteDC(hdc);
}

void paintWindow(){
	if (hbitmap != NULL){
		PAINTSTRUCT 	ps;
		HDC 			hdc;
		BITMAP 			bitmap;
		HDC 			hdcMem;
		HGDIOBJ 		oldBitmap;

		hdc = BeginPaint(hwnd, &ps);

		paintToBuffer();

		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, buffer);

		GetObject(buffer, sizeof(bitmap), &bitmap);
		BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, oldBitmap);

		DeleteDC(hdcMem);
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
			createBuffer();
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
	case WM_LBUTTONDOWN:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		mousePressX = mouseX;
		mousePressY = mouseY;
		if (!selectRect.valid)
			selectionType = SELECTION;
		break;
	case WM_LBUTTONUP:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		selectionType = NOTHING;
		break;
	case WM_MOUSEMOVE:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		if (selectionType == SELECTION){
			selectRect = Selection(mousePressX, mousePressY, mouseX, mouseY);
		}
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
		InvalidateRect(hwnd, 0, true);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

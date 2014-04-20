#include "Window.h"

bool registerHotkey(){
	return RegisterHotKey(hwnd, 0, HOTKEY_MOD, HOTKEY_VK) != 0;
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

	return RegisterClassEx(&wc) != 0;
}

bool createWindow(){
	HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &rect);
	hwnd = CreateWindow(CLASS_NAME, "", WS_POPUP, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	//SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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

void disposeWindow(){
	DeleteObject(hbitmap);
	hbitmap = NULL;
	selectRect.valid = false;
	delete[] capturePixels;
	DeleteObject(buffer);
	ShowWindow(hwnd, SW_HIDE);
}

void sendToClipboard(){
	if (selectRect.valid){
		OpenClipboard(hwnd);
		EmptyClipboard();

		BITMAPINFOHEADER bmih;
		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biWidth = selectRect.width;
		bmih.biHeight = -selectRect.height;
		bmih.biPlanes = 1;
		bmih.biBitCount = 32;
		bmih.biCompression = BI_RGB;

		BITMAPINFO dbmi;
		dbmi.bmiHeader = bmih;

		HDC hdc = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP capture = CreateCompatibleBitmap(hdc, selectRect.width, selectRect.height);
		bufferWidth = rect.right - rect.left;
		bufferHeight = rect.bottom - rect.top;
		pixel *ps = new pixel[selectRect.width * selectRect.height];

		HGDIOBJ old = SelectObject(hdcMem, hbitmap);

		for (int i = 0; i < selectRect.height; i++){
			memcpy(&ps[i * selectRect.width], &capturePixels[(i + selectRect.y) * bufferWidth + selectRect.x], selectRect.width * 4);
		}

		SetDIBits(hdc, capture, 0, selectRect.height, ps, &dbmi, 0);

		SetClipboardData(CF_BITMAP, capture);

		SelectObject(hdc, old);

		CloseClipboard();

		ReleaseDC(NULL, hdc);
		DeleteDC(hdcMem);
		DeleteObject(capture);
		delete[] ps;
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
				selectRect.valid = false;
				delete[] capturePixels;
				DeleteObject(buffer);
				ShowWindow(hwnd, SW_HIDE);
			}
		}
		break;
	case VK_RETURN:
		sendToClipboard();
		disposeWindow();
		break;
	case 'C':
		if (ctrlPressed)
			sendToClipboard();
		break;
	case 'X':
		if (ctrlPressed){
			sendToClipboard();
			disposeWindow();
		}
		break;
	case 'S':
		if (ctrlPressed){
			OPENFILENAMEW openFile = { 0 };
			wchar_t filePath[261];
			memset(&filePath, 0, 261);
			openFile.lStructSize = sizeof(OPENFILENAMEA);
			openFile.hwndOwner = hwnd;
			openFile.lpstrFile = filePath;
			openFile.nMaxFile = 261;
			openFile.Flags = OFN_EXPLORER;
			unsigned int num = 0, size = 0;
			GetImageEncodersSize(&num, &size);
			ImageCodecInfo *pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
			GetImageEncoders(num, size, pImageCodecInfo);

			wstring s;

			for (unsigned int i = 0; i < num; ++i){
				const wchar_t *format = pImageCodecInfo[i].FormatDescription;
				const wchar_t *filename = pImageCodecInfo[i].FilenameExtension;
				wstring fileLower(filename);
				transform(fileLower.begin(), fileLower.end(), fileLower.begin(), tolower);
				s = s + wstring(format, wcslen(format)) + wstring(L" (") + fileLower + wstring(L")", 2) + wstring(filename, wcslen(filename) + 1);
			}
			s = s + wstring(L"\0", 1);

			openFile.lpstrFilter = s.c_str();

			openFile.lpstrCustomFilter = NULL;
			openFile.nFilterIndex = 0;
			openFile.lpstrFileTitle = NULL;
			openFile.lpstrInitialDir = NULL;
			openFile.lpstrTitle = L"Save Capture As";
			if (!GetSaveFileNameW(&openFile)){
				return;
			}

			BITMAPINFOHEADER bmih;
			bmih.biSize = sizeof(BITMAPINFOHEADER);
			bmih.biWidth = selectRect.width;
			bmih.biHeight = -selectRect.height;
			bmih.biPlanes = 1;
			bmih.biBitCount = 32;
			bmih.biCompression = BI_RGB;

			BITMAPINFO dbmi;
			dbmi.bmiHeader = bmih;

			HDC hdc = GetDC(NULL);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP capture = CreateCompatibleBitmap(hdc, selectRect.width, selectRect.height);
			bufferWidth = rect.right - rect.left;
			bufferHeight = rect.bottom - rect.top;
			pixel *ps = new pixel[selectRect.width * selectRect.height];

			HGDIOBJ old = SelectObject(hdcMem, hbitmap);

			for (int i = 0; i < selectRect.height; i++){
				memcpy(&ps[i * selectRect.width], &capturePixels[(i + selectRect.y) * bufferWidth + selectRect.x], selectRect.width * 4);
			}

			SetDIBits(hdc, capture, 0, selectRect.height, ps, &dbmi, 0);

			ImageCodecInfo info = pImageCodecInfo[openFile.nFilterIndex - 1];

			wchar_t *fne = new wchar_t[wcslen(info.FilenameExtension) + 1];
			wcscpy_s(fne, wcslen(info.FilenameExtension) + 1, info.FilenameExtension);

			wchar_t *context = NULL;
			wchar_t *c = wcstok_s(fne, L";", &context);
			wchar_t *first = c;

			bool found = false;

			while (c != NULL){
				wstring str(filePath);
				transform(str.begin(), str.end(), str.begin(), toupper);
				wstring suffix(c);
				suffix = suffix.substr(1);
				if (str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0){
					found = true;
				}
				c = wcstok_s(NULL, L";", &context);
			}

			wstring path(filePath);

			if (!found){
				wstring extension = wstring(first).substr(1);
				transform(extension.begin(), extension.end(), extension.begin(), tolower);
				path = path + extension;
			}

			Bitmap bitmap(capture, NULL);
			bitmap.Save(path.c_str(), &(info.Clsid));

			delete[] fne;

			SelectObject(hdc, old);

			ReleaseDC(NULL, hdc);
			DeleteDC(hdcMem);
			DeleteObject(capture);
			delete[] ps;
			delete[] pImageCodecInfo;
		}
		break;
	}
}

void drawRect(int x1, int y1, int x2, int y2, pixel color, short thickness){
	for (char j = -thickness; j <= thickness; j++){
		for (int i = x1 - thickness; i < x2 + thickness + 1; i++){
			setPixel(i, y1 + j, color);
			setPixel(i, y2 + j, color);
		}
		for (int i = y1 + thickness + 1; i < y2 - thickness; i++){
			setPixel(x1 + j, i, color);
			setPixel(x2 + j, i, color);
		}
	}
}

void fillRect(int x1, int y1, int x2, int y2, pixel color){
	for (int x = x1; x < x2; x++){
		for (int y = y1; y < y2; y++){
			setPixel(x, y, color);
		}
	}
}

void paintToBuffer(){
	memcpy(pixels, capturePixels, bufferWidth * bufferHeight * 4);
	for (int i = 0; i < bufferWidth * bufferHeight; i++){
		pixels[i].r -= pixels[i].r >> 2;
		pixels[i].g -= pixels[i].g >> 2;
		pixels[i].b -= pixels[i].b >> 2;
	}
	if (selectRect.valid){
		for (int i = selectRect.y; i < selectRect.y + selectRect.height; i++){
			memcpy(&pixels[i * bufferWidth + selectRect.x], &capturePixels[i * bufferWidth + selectRect.x], selectRect.width * 4);
		}
		drawRect(selectRect.x, selectRect.y, selectRect.x + selectRect.width, selectRect.y + selectRect.height, SELECTION_COLOR, SELECTION_WIDTH);
		drawText(selectRect);
	}
	else{
		RECT rect;
		POINT p;
		GetCursorPos(&p);
		transparent = true;
		GetWindowRect(WindowFromPoint(p), &rect);
		if (rect.left < 0)
			rect.left = 0;
		if (rect.right > bufferWidth)
			rect.right = bufferWidth;
		if (rect.top < 0)
			rect.top = 0;
		if (rect.bottom > bufferHeight)
			rect.bottom = bufferHeight;
		for (int i = rect.top; i < rect.bottom; i++){
			memcpy(&pixels[i * bufferWidth + rect.left], &capturePixels[i * bufferWidth + rect.left], (rect.right - rect.left) * 4);
		}
		drawRect(rect.left, rect.top, rect.right, rect.bottom, WINDOW_SELECT_COLOR, WINDOW_SELECT_WIDTH);
		drawText(Selection(rect.left, rect.top, rect.right, rect.bottom));
	}
	drawZoom();
}

void drawZoom(){
	int realx = mouseX + 20 > bufferWidth - 129 ? bufferWidth - 129 : mouseX + 20;
	int realy = mouseY + 20 > bufferHeight - 190 ? bufferHeight - 190 : mouseY + 20;

	fillRect(realx + 4, realy + 5, 130 + realx + 4, 190 + realy + 5, pixel(0xC0808080));
	fillRect(realx - 1, realy - 1, 130 + realx - 1, 190 + realy - 1, pixel(0xC0000000));

	for (int i = 0; i < 128; i++){
		for (int j = 0; j < 128; j++){
			double calcZoom = 64 / zoom;
			double shift = zoom / 2.0;

			setPixel(i + realx, j + realy, getPixel((int)((i + shift) / zoom + mouseX - calcZoom), (int)((j + shift) / zoom + mouseY - calcZoom)));
		}
	}
	fillRect(realx + 63, realy, realx + 65, realy + 128, pixel(0x80000000));
	fillRect(realx, realy + 63, realx + 128, realy + 65, pixel(0x80000000));

	HDC dcWnd = GetDC(hwnd);
	HDC hdc = CreateCompatibleDC(dcWnd);
	HFONT font = CreateFont(17, 0, 0, 0, FW_LIGHT, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	SelectObject(hdc, font);
	HBITMAP hBmpOld = (HBITMAP)SelectObject(hdc, buffer);

	pixel p = getPixel(mouseX, mouseY);

	fillRect(realx + 81, realy + 130, realx + 127, realy + 147, p);

	ostringstream s;
	s << "0x" << setw(6) << setfill('0') << uppercase << hex << (p.val & 0xFFFFFF) << dec << "\nRGB: " << (unsigned short)p.r << " " << (unsigned short)p.g << " " << (unsigned short)p.b << "\nx: " << mouseX << ", y: " << mouseY;
	string txt = s.str();

	RECT rect;
	rect.left = realx + 3;
	rect.top = realy + 132;
	rect.bottom = realy + 190;
	rect.right = realx + 128;
	SetBkMode(hdc, TRANSPARENT);

	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, txt.c_str(), txt.length(), &rect, DT_LEFT | DT_WORDBREAK);

	SelectObject(hdc, hBmpOld);
	ReleaseDC(hwnd, dcWnd);
	DeleteDC(hdc);
	DeleteObject(font);

}
void drawText(Selection sel){
	ostringstream s;
	s << sel.width << " x " << sel.height;
	string txt = s.str();
	HDC dcWnd = GetDC(hwnd);
	HDC hdc = CreateCompatibleDC(dcWnd);
	HFONT font = CreateFont(15, 0, 0, 0, FW_LIGHT, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	SelectObject(hdc, font);
	SIZE size;
	GetTextExtentPoint32(hdc, txt.c_str(), txt.length(), &size);
	int width = size.cx;
	int height = size.cy;
	int x = sel.x;
	int y = sel.y;
	if (y - height - 2 < 0){
		y = height + 2;
	}
	if (x + width + 6 > bufferWidth){
		x = bufferWidth - width - 6;
	}
	fillRect(x, y - height - 2, width + 6 + x, height + 3 + y - height - 2, pixel(0xC0000000));
	HBITMAP hBmpOld = (HBITMAP)SelectObject(hdc, buffer);

	RECT rect;
	rect.left = x + 3;
	rect.top = y - height;
	rect.bottom = y;
	rect.right = x + 3 + width;
	SetBkMode(hdc, TRANSPARENT);
	
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, txt.c_str(), txt.length(), &rect, DT_LEFT | DT_WORDBREAK);

	SelectObject(hdc, hBmpOld);
	ReleaseDC(hwnd, dcWnd);
	DeleteDC(hdc);
	DeleteObject(font);
}

pixel getPixel(int x, int y){
	if (x >= 0 && x < bufferWidth && y >= 0 && y < bufferHeight) {
		pixel p = capturePixels[x + y * bufferWidth];
		p.a = 255;
		return p;
	}
	return pixel();
}

void setPixel(int x, int y, pixel color){
	if (x >= 0 && x < bufferWidth && y >= 0 && y < bufferHeight) {
		unsigned char alpha = color.a;
		if (alpha < 255){
			pixel *original = &pixels[x + y * bufferWidth];
			original->r = (original->r * (255 - alpha) + color.r * alpha) >> 8;
			original->g = (original->g * (255 - alpha) + color.g * alpha) >> 8;
			original->b = (original->b * (255 - alpha) + color.b * alpha) >> 8;
		}
		else
			pixels[x + y * bufferWidth] = color;
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
		if (!selectRect.valid){
			RECT rect;
			POINT p;
			GetCursorPos(&p);
			transparent = true;
			GetWindowRect(WindowFromPoint(p), &rect);
			if (rect.left < 0)
				rect.left = 0;
			if (rect.right > bufferWidth)
				rect.right = bufferWidth;
			if (rect.top < 0)
				rect.top = 0;
			if (rect.bottom > bufferHeight)
				rect.bottom = bufferHeight;
			selectRect = Selection(rect.left, rect.top, rect.right, rect.bottom);
			InvalidateRect(hwnd, 0, false);
		}
		break;
	case WM_MOUSEMOVE:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		if (selectionType == SELECTION){
			selectRect = Selection(mousePressX, mousePressY, mouseX, mouseY);
			selectRect.expand(1, 1);
		}
		InvalidateRect(hwnd, 0, false);
		break;
	case WM_MOUSEWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0){
			zoom += 1;
		}
		if (GET_WHEEL_DELTA_WPARAM(wParam) < 0){
			zoom -= 1;
		}
		if (zoom < 1)
			zoom = 1;
		if (zoom > 32)
			zoom = 32;
		InvalidateRect(hwnd, 0, false);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_CONTROL){
			ctrlPressed = true;
		}
		keyPressed(wParam);
		InvalidateRect(hwnd, 0, true);
		break;
	case WM_KEYUP:
		if (wParam == VK_CONTROL){
			ctrlPressed = false;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

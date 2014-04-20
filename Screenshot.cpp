#include "Window.h"

/*int WINAPI WinMain(HINSTANCE hi, HINSTANCE hpi, LPSTR lcl, int ncs){
	hInstance = hi;
	hPrevInstance = hpi;
	lpCmdLine = lcl;*/

int main(){
	hInstance = 0;
	hPrevInstance = 0;
	lpCmdLine = 0;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	/*unsigned int num = 0, size = 0;
	GetImageEncodersSize(&num, &size);
	ImageCodecInfo *pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
	GetImageEncoders(num, size, pImageCodecInfo);
	for (unsigned int j = 0; j < num; ++j){
		wcout << pImageCodecInfo[j].MimeType << endl;
	}*/

	if (!registerWindowClass()){
		MessageBox(NULL, "Unable to register window class", "Error", MB_ICONERROR);
	}
	if (!createWindow()){
		MessageBox(NULL, "Unable to create the window", "Error", MB_ICONERROR);
	}
	if (!registerHotkey()){
		MessageBox(NULL, "Unable to register hotkey", "Error", MB_ICONERROR);
		exit(-1);
	}
	startMessageLoop();

	system("pause");
}
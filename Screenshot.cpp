#include "Window.h"

/*int WINAPI WinMain(HINSTANCE hi, HINSTANCE hpi, LPSTR lcl, int ncs){
	hInstance = hi;
	hPrevInstance = hpi;
	lpCmdLine = lcl;
	*/
int main(){
	hInstance = 0;
	hPrevInstance = 0;
	lpCmdLine = 0;

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
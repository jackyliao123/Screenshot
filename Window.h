#ifndef Window_h
#define Window_h

#include <iostream>
using namespace std;

#include "Constants.h"
#include "Selection.h"

HBITMAP takeScreenshot();
bool registerWindowClass();
bool createWindow();
void startMessageLoop();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void keyPressed(int vk);
bool registerHotkey();
void createBuffer();
void paintWindow();
void paintToBuffer();

static HINSTANCE hInstance;
static HINSTANCE hPrevInstance;
static LPSTR lpCmdLine;
static HBITMAP hbitmap;
static HBITMAP buffer;

static HWND hwnd;
static RECT rect;
static Selection selectRect;

static bool transparent;

struct pixel {
	union {
		struct {
			unsigned char b, g, r, a;
		};
		unsigned int val;
	};
	pixel() {
		val = 0;
	}
};

static pixel *pixels;
static pixel *capturePixels;

static char selectionType;

static WORD mousePressX;
static WORD mousePressY;

static WORD mouseX;
static WORD mouseY;

static unsigned int bufferWidth;
static unsigned int bufferHeight;

#endif

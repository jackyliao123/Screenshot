#ifndef Window_h
#define Window_h

#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

#include "Constants.h"
#include "Selection.h"

#include <GdiPlus.h>
using namespace Gdiplus;

#include <algorithm>

#pragma comment (lib, "Gdiplus.lib")

static ULONG_PTR gdiplusToken;
static GdiplusStartupInput gdiplusStartupInput;

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
void drawZoom();
void drawText(Selection sel);
void sendToClipboard();
void disposeWindow();

static HINSTANCE hInstance;
static HINSTANCE hPrevInstance;
static LPSTR lpCmdLine;
static HBITMAP hbitmap;
static HBITMAP buffer;

static HWND hwnd;
static RECT rect;
static Selection selectRect;

static bool transparent;
static double zoom = 2;
static bool ctrlPressed = false;

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
	pixel(unsigned int v) {
		val = v;
	}
};

void setPixel(int x, int y, pixel color);
pixel getPixel(int x, int y);
void drawRect(int x1, int y1, int x2, int y2, pixel color, short thickness);

static pixel *pixels;
static pixel *capturePixels;

static char selectionType;

static WORD mousePressX;
static WORD mousePressY;

static WORD mouseX;
static WORD mouseY;

static int bufferWidth;
static int bufferHeight;

#endif

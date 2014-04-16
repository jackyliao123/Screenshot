#ifndef Window_h
#define Window_h

#include <iostream>
using namespace std;

#include "Constants.h"

HBITMAP takeScreenshot();
bool registerWindowClass();
bool createWindow();
void startMessageLoop();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void keyPressed(int vk);
bool registerHotkey();
void paintWindow();

static HINSTANCE hInstance;
static HINSTANCE hPrevInstance;
static LPSTR lpCmdLine;
static HBITMAP hbitmap;

static HWND hwnd;

static bool transparent;

#endif

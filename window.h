#ifndef WINDOW_DEF
#define WINDOW_DEF
#include <string>
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include "ymath.h"

std::string Window_ExecuteCommand(const std::string &cmd);
Vector2 Window_GetCursorPos();
void Window_SetLockCursor(bool lock);
int Window_Open(const std::string &title, int posx, int posy, int width, int height, int fs);
void Window_MainLoop(void(*Update)(), void(*Event)(SDL_Event), bool(*Draw)(), void(*Focused)(), void(*OnResize)(), int display_fps, int stretch, float lockRes);
void Window_CleanUp();
void Window_Close();
void Window_WarpMouse(int x, int y);
int Window_GetDeltaTime();
int Window_GetTicks();
int Window_MaxDeltaTime();
int Window_GetWindowWidth();
int Window_GetWindowHeight();
int Window_IsKeyDown(const std::string &name);
int Window_GetWidth();
int Window_GetHeight();
int Window_GetViewportWidth();
int Window_GetViewportHeight();
void Window_SetViewportWidth(int w);
void Window_SetViewportHeight(int h);
void Window_SetMousePos(int x, int y);
HWND Window_GetWMWindow();
HDC Window_GetWMDC();
std::string Window_GetCopiedText();

#endif
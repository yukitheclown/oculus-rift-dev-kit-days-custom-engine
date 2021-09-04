#undef __STRICT_ANSI__
#include "stdafx.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_syswm.h"
#include "window.h"
#include <stdio.h>
#include <math.h>

static SDL_Window *window;
static SDL_GLContext context;
static int windowWidth = 0, windowHeight = 0;
static int viewportWidth = 0, viewportHeight = 0;
static int breakLoop = 0;
static int deltaTime = 0;
static int maxDeltaTime = 100;
static bool lockCursor = false;
static Vector2 cursorPos;

int Window_Open(const std::string &title, int posx, int posy, int width, int height, int fs){
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow(
		title.c_str(),
		posx,
		posy,
		width,
		height,
		SDL_WINDOW_OPENGL | fs
		);
	context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(0);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("Glew Init Failed\n");
		return 0;
	}

	windowWidth = width;
	windowHeight = height;
	if (viewportWidth <= 0) viewportWidth = width;
	if (viewportHeight <= 0) viewportHeight = height;

	SDL_ShowCursor(0);

	return 1;
}

void Window_WarpMouse(int x, int y){
	SDL_WarpMouseInWindow(window, x, y);
}

void Window_CleanUp(){
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window_Close(){
	Window_CleanUp();
	breakLoop = 0;
}

int Window_GetDeltaTime(){
	return deltaTime;
}

int Window_GetTicks(){

	return SDL_GetTicks();
}

void Window_SetMousePos(int x, int y){
	SDL_WarpMouseInWindow(window, x, y);
}

Vector2 Window_GetCursorPos(){
	return cursorPos;
}

void Window_SetLockCursor(bool lock){
	lockCursor = lock;
}

HWND Window_GetWMWindow(){

	SDL_SysWMinfo info;
	SDL_GetWindowWMInfo(window, &info);

	return info.info.win.window;
}

HDC Window_GetWMDC(){
	return 0;
	// SDL_SysWMinfo info;
	// SDL_GetWindowWMInfo(window, &info);

	// return info.info.win.hd;
}

std::string Window_ExecuteCommand(const std::string &cmd){

	FILE* pipe = _popen(cmd.c_str(), "r");

	if (!pipe) return "";

	char buffer[128];

	std::string result = "";

	while (!feof(pipe)){
		memset(buffer, 0, sizeof(buffer));
		if (fgets(buffer, 128, pipe) != NULL){
			result += std::string(buffer);
		}
	}

	_pclose(pipe);

	return result;
}

std::string Window_GetCopiedText(){

	 char *text = SDL_GetClipboardText();

	 std::string ret = "";

	if(text) ret = text;

	SDL_free(text);

	return ret;
}

int Window_MaxDeltaTime(){ return maxDeltaTime; }
int Window_GetWindowHeight(){ return windowHeight; }
int Window_GetWindowWidth(){ return windowWidth; }
int Window_GetViewportWidth() { return viewportWidth; }
int Window_GetViewportHeight() { return viewportHeight; }
void Window_SetViewportWidth(int w) { viewportWidth = w; glViewport(0, 0, viewportWidth, viewportHeight); }
void Window_SetViewportHeight(int h) { viewportHeight = h; glViewport(0, 0, viewportWidth, viewportHeight); }

void Window_MainLoop(void(*Update)(), void(*Event)(SDL_Event), bool(*Draw)(), void(*Focused)(), void(*OnResize)(),
	int display_fps, int stretch, float lockRes){

	SDL_Event event;
	int prevDisplayFPS = SDL_GetTicks();
	int lastTime = SDL_GetTicks();;
	int frames = 0;

	breakLoop = 1;

	while (breakLoop){

		while (SDL_PollEvent(&event)){

			if (event.type == SDL_WINDOWEVENT){
				if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
					Focused();
				}
			}
			else if (event.type == SDL_QUIT){
				Window_Close();
				break;
			}
			else if (event.type == SDL_MOUSEMOTION && lockCursor){

				Vector2 cPos = { (float)(event.motion.x - (Window_GetWindowWidth() / 2)),
					(float)(event.motion.y - (Window_GetWindowHeight() / 2)) };

				cursorPos += cPos*0.2f;
				Window_SetMousePos((int)Window_GetWindowWidth() / 2, (int)Window_GetWindowHeight() / 2);

				if (cursorPos.x < 0) cursorPos.x = 0;
				if (cursorPos.y < 0) cursorPos.y = 0;

				if (cursorPos.x > Window_GetViewportWidth()) cursorPos.x = (float)Window_GetViewportWidth();
				if (cursorPos.y > Window_GetViewportHeight()) cursorPos.y = (float)Window_GetViewportHeight();
			}

			if (stretch && event.type == SDL_WINDOWEVENT){
				if (event.window.event == SDL_WINDOWEVENT_RESIZED){
					windowWidth = event.window.data1;
					windowHeight = event.window.data2;

					if (!lockRes)
						glViewport(0, 0, windowWidth, windowHeight);
					else
						glViewport(0, (event.window.data2 - (int)round(event.window.data1 * lockRes)) / 2, event.window.data1,
						(int)round(event.window.data1 * lockRes));

					OnResize();
				}
			}
			Event(event);
		}

		if (!breakLoop) break;

		int currTime = SDL_GetTicks();
		deltaTime = currTime - lastTime;
		lastTime = currTime;

		Update();
		if (Draw()) SDL_GL_SwapWindow(window);

		frames++;

		if (display_fps){
			int currTime = SDL_GetTicks();
			if (currTime - prevDisplayFPS > 1000){
				printf("%f ms\n", ((float)currTime - (float)prevDisplayFPS) / frames);
				printf("%i fps\n", frames);
				prevDisplayFPS = currTime;
				frames = 0;
			}
		}
	}
}
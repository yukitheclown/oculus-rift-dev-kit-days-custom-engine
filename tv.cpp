#include "tv.h"
#include "stdafx.h"
#include "text.h"
#include "window.h"
#include <windows.h>
#include <GL/glew.h>
#include <shlobj.h>

void TvMenu::RefreshScreen(){

	font.Clear();

	const int textOffsetX = 100;
	const int textOffsetY = 100;
	const int textOffsetPerLine = 60;

	for(int k = scroll; k < (int)onScreen->options.size(); k++){

		std::string option = onScreen->options[k].text;
		if(k == selection)
			font.AddText((char)COLOR_CHAR5 + option, textOffsetX, textOffsetY + ((k-scroll) * textOffsetPerLine), 0, 1, 1);
		else
			font.AddText(option, textOffsetX, textOffsetY + ((k-scroll) * textOffsetPerLine), 0, 1, 1);
	}
}

void TvMenu::Draw(){

	Shaders_UseProgram(TEXT_2D_SHADER);
	Shaders_SetUniformColor({1,1,1,1});
	font.DrawAllText();
}

void TvMenu::SelectionUp(){

	if(!onScreen) return;

	if(selection-1 < 0)
		return;

	if(--selection-scroll < 0)
		--scroll;

	RefreshScreen();
}

void FileBrowser::GetContents(const std::wstring &inDir){

	inFiles.clear();
	inDirectories.clear();

	inDirectory = inDir;

	std::wstring dir = inDir + L"*";

    WIN32_FIND_DATA ffd;
    HANDLE find = FindFirstFile((LPCWSTR)dir.c_str(), &ffd);

    if(INVALID_HANDLE_VALUE == find)
        return;

	while (FindNextFile(find, &ffd)){

        if(ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;

    	std::wstring temp = inDir + ffd.cFileName;

        if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){

        	// if(strcmp((char *)ffd.cFileName, ".") == 0) continue;

        	inDirectories.resize(inDirectories.size()+1);
            inDirectories[inDirectories.size()-1].path = std::wstring(temp) + L"\\";
            std::wstring name = std::wstring(ffd.cFileName);
            inDirectories[inDirectories.size()-1].name = std::string(name.begin(), name.end());
        }

        else {

        	inFiles.resize(inFiles.size()+1);
	        inFiles[inFiles.size()-1].path = temp;
            std::wstring name = std::wstring(ffd.cFileName);
            inFiles[inFiles.size()-1].name = std::string(name.begin(), name.end());
        }

    }

    FindClose(find);
}

void TvMenu::RefreshFileOptions(bool upDir){

	if(!onScreen) return;

	std::shared_ptr<FileBrowser> fileBrowser = onScreen->fileBrowser;

	if(!fileBrowser) return;

	std::wstring directory = homeDirectory;// + "/Videos/";

	if(upDir){

		directory = fileBrowser->GetDirectory();

    	for(int f = directory.size()-1; f >= 0; f--){
    		if(directory[f] == '\\') {
    			directory.resize(f);
    			break;
    		}
    	}

	} else {

		if(selection < (int)onScreen->options.size() && onScreen->options[selection].dirPath.size() > 0)
			directory = onScreen->options[selection].dirPath;
	}

	onScreen->options.clear();

	fileBrowser->GetContents(directory);

	for(int k = 0; k < (int)fileBrowser->inDirectories.size(); k++){
		onScreen->options.push_back({nullptr, fileBrowser->inDirectories[k].name, L"", fileBrowser->inDirectories[k].path});
	}

	for(int k = 0; k < (int)fileBrowser->inFiles.size(); k++){
		onScreen->options.push_back({nullptr, fileBrowser->inFiles[k].name, fileBrowser->inFiles[k].path, L""});
	}
}

void TvMenu::SelectionDown(){

	if(!onScreen) return;

	if(selection+1 >= (int)onScreen->options.size())
		return;

	if(++selection-scroll > maxLines)
		++scroll;

	RefreshScreen();
}

void TvMenu::Select(TvScreen *parent){
	if(!parent || !onScreen || selection >= (int)onScreen->options.size()) return;

	std::wstring videoPath = L"";

	if(onScreen->fileBrowser){

		if(selection < (int)onScreen->fileBrowser->inDirectories.size())
			RefreshFileOptions(false);
		else if(selection < (int)onScreen->options.size())
			videoPath = onScreen->options[selection].path;

	} else if(onScreen->options[selection].next) {

		onScreen = onScreen->options[selection].next;
		RefreshFileOptions(false);

	}

	if(videoPath.size() && onScreen != youtubeScreenBest && onScreen != youtubeScreenWorst &&
			onScreen != youtubeScreenStandard && onScreen != streamScreen){

		parent->videoLoading = true;

		if(videoPath.length()) parent->PlayVideo(videoPath);

	} else if((onScreen == youtubeScreenBest || onScreen == youtubeScreenStandard ||
			onScreen == youtubeScreenWorst || onScreen == streamScreen) && onScreen->options[0].path.length()) {

		parent->videoLoading = true;

		onScreen->options[0].text = (char)COLOR_CHAR7 + std::string("Loading...");

		if (onScreen == youtubeScreenBest){

			parent->PlayVideo(L"", true, "youtube-dl --no-check-certificate -f best -g --cookies " +
				VideoState::CookiesFile + " " + std::string(onScreen->options[0].path.begin(), onScreen->options[0].path.end()));

		} else if (onScreen == youtubeScreenStandard){

			parent->PlayVideo(L"", true, "youtube-dl --no-check-certificate -f 18 -g --cookies " +
				VideoState::CookiesFile + " " + std::string(onScreen->options[0].path.begin(), onScreen->options[0].path.end()));

		} else if (onScreen == youtubeScreenWorst){

			parent->PlayVideo(L"", true, "youtube-dl --no-check-certificate -f worst -g --cookies " +
				VideoState::CookiesFile + " " + std::string(onScreen->options[0].path.begin(), onScreen->options[0].path.end()));

		} else {

			parent->PlayVideo(onScreen->options[0].path, true);
		}
	}

	selection = scroll = 0;
	RefreshScreen();
}

void TvMenu::BackScreen(){
	if(!onScreen) return;


	if(onScreen->fileBrowser){

		RefreshFileOptions(true);

	} else if(onScreen->prev){

		onScreen = onScreen->prev;
		RefreshFileOptions(true);
	}

	selection = scroll = 0;
	RefreshScreen();
}

void TvMenu::Init(){

	font.LoadFont("Resources/DejaVuSansMono.ttf");
	font.SetSize(32);

	std::shared_ptr<FileBrowser> fileBrowser = std::shared_ptr<FileBrowser>(new FileBrowser());

	PWSTR path;

	SHGetKnownFolderPath (FOLDERID_Videos, 0, NULL, &path);

	homeDirectory = std::wstring((WCHAR *)path) + L"\\";// getpwuid(getuid())->pw_dir;

	CoTaskMemFree(path);

	startScreen = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(nullptr, nullptr));
	creditsScreen = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, nullptr));
	playScreen = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, fileBrowser));
	youtubeScreenBest = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, nullptr));
	youtubeScreenStandard = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, nullptr));
	youtubeScreenWorst = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, nullptr));
	streamScreen = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, nullptr));
	// cheatsScreen = std::shared_ptr<TvMenuScreen>(new TvMenuScreen(startScreen, nullptr));

	startScreen->options.push_back({playScreen,"Play", L"", L""});
	startScreen->options.push_back({youtubeScreenBest,"Youtube Best Quality", L"", L""});
	startScreen->options.push_back({youtubeScreenStandard,"Youtube Standard Quality", L"", L""});
	startScreen->options.push_back({youtubeScreenWorst,"Youtube Worst Quality", L"", L""});
	startScreen->options.push_back({streamScreen,"Stream", L"", L""});
	startScreen->options.push_back({creditsScreen,"Credits",L"", L""});
	// startScreen->options.push_back({cheatsScreen,"Secrets",L"", L""});

	youtubeScreenBest->options.push_back({nullptr, std::string((char)COLOR_CHAR7 + std::string("PRESS v TO PASTE YOUTUBE URL THEN PRESS ENTER. "
		"(PRESS v AGAIN IF YOU WANT TO OVERRIDE)")),L"", L""});
	youtubeScreenWorst->options.push_back({nullptr, std::string((char)COLOR_CHAR7 + std::string("PRESS v TO PASTE YOUTUBE URL THEN PRESS ENTER. "
		"(PRESS v AGAIN IF YOU WANT TO OVERRIDE)")),L"", L""});
	youtubeScreenStandard->options.push_back({nullptr, std::string((char)COLOR_CHAR7 + std::string("PRESS v TO PASTE YOUTUBE URL THEN PRESS ENTER. "
		"(PRESS v AGAIN IF YOU WANT TO OVERRIDE)")),L"", L""});
	streamScreen->options.push_back({nullptr, std::string((char)COLOR_CHAR7 + std::string("PRESS v TO PASTE STREAM URL THEN PRESS ENTER. "
		"(PRESS v AGAIN IF YOU WANT TO OVERRIDE)")),L"", L""});
	creditsScreen->options.push_back({nullptr,"YUKIZINI | PROGRAMMING | YUKIZINI.TUMBLR.COM",L"", L""});
	creditsScreen->options.push_back({nullptr,"CHENDEV | ANIMATIONS | HONKDEV.TUMBLR.COM",L"", L""});
	creditsScreen->options.push_back({nullptr,"WHOEVER MADE THESE MODELS I FOUND ONLINE",L"", L""});
	creditsScreen->options.push_back({nullptr,"AND ALL MY FRIENDS ON AGDG",L"", L""});
	// creditsScreen->options.push_back({nullptr,"--------------------BLACKLIST-----------------",L"", L""});
	// creditsScreen->options.push_back({nullptr,"IF YOUR NAME IS SAID PLEASE CLOSE THE GAME NOW",L"", L""});
	// creditsScreen->options.push_back({nullptr,"----------------------------------------------",L"", L""});
	// creditsScreen->options.push_back({nullptr,"MURF",L"", L""});
	// creditsScreen->options.push_back({nullptr,"ANYONE FROM THE STEAMCHAT",L"", L""});
	// creditsScreen->options.push_back({nullptr,"GOOGUM",L"", L""});

	// cheatsScreen->options.push_back({nullptr,"IF YOU FIND THE ANSWER TO ANY OF THESE RIDDLES",L"", L""});
	// cheatsScreen->options.push_back({nullptr,"EMAIL ME ASAP YUKIZINI@YUKIZINI.COM",L"", L""});
	// cheatsScreen->options.push_back({nullptr,"AND I WILL GIVE PRIZES TO THE FIRST TO FIND THEM",L"", L""});
	// cheatsScreen->options.push_back({nullptr,"-----------------------------------",L"", L""});
	// cheatsScreen->options.push_back({nullptr,"#1 : 6d656d6520646f7420656d206b617920766565",L"", L""});
	// cheatsScreen->options.push_back({nullptr,"#2 : .lua lua()",L"", L""});

	onScreen = startScreen;

	RefreshScreen();
}

void TvMenu::Event(const SDL_Event &ev, TvScreen *parent){

	if(ev.type == SDL_KEYUP){
		if(ev.key.keysym.sym == SDLK_UP)
			SelectionUp();
		else if(ev.key.keysym.sym == SDLK_DOWN)
			SelectionDown();
		else if(ev.key.keysym.sym == SDLK_BACKSPACE)
			BackScreen();
		else if(ev.key.keysym.sym == SDLK_v){

			if(onScreen == youtubeScreenStandard || onScreen == youtubeScreenBest ||
				onScreen == youtubeScreenWorst || onScreen == streamScreen){

				std::string path = Window_GetCopiedText();

				std::string ret = "";

				for(int k = 0; k < (int)path.size(); k++)
					if((path[k] >= 32 && path[k] <= 126))
						ret += path[k];

				onScreen->options[0].text = (char)COLOR_CHAR7 + ret;
				onScreen->options[0].path = std::wstring(ret.begin(), ret.end());
				RefreshScreen();
			}
		} else if(ev.key.keysym.sym == SDLK_RETURN){
			Select(parent);
		}
	}
}

void TvScreen::Event(const SDL_Event &ev){

	if(videoLoading) return;

	if(!videoPlaying){

		tvMenu.Event(ev, this);

	} else {

		if(ev.type == SDL_KEYUP){
			if(ev.key.keysym.sym == SDLK_RIGHT)
				videoState.Seek(10);
			else if(ev.key.keysym.sym == SDLK_LEFT)
				videoState.Seek(-10);
		}
	}
}

void TvScreen::PlayVideo(const std::wstring &path, bool isStream, const std::string &command, const std::string &cookies){

	videoState.Init(std::string(path.begin(), path.end()).c_str(), isStream, command, cookies);
}

void TvScreen::Init(){

	font.LoadFont("Resources/DejaVuSansMono.ttf");
	font.SetSize(48);

	tvMenu.Init();
	videoPlane.Create(TEXTURED_2D_SHADER);
}

void TvScreen::Close(){
	videoState.Close();
	glDeleteTextures(1, &videoGlTexture);
}

void TvScreen::Update(){

	videoState.Update();

	if(videoPlaying){

		videoState.VideoRefreshTimer(videoGlTexture, subtitleText);
	}
}

Vector3 TvScreen::GetAverageFrameColor(){
	if (videoState.opened){
		return videoState.avgFrameColor;
	}
	else {
		return Vector3( 0.1, 0.3, 0.3) *255;
	}
}

void TvScreen::Draw(){

	float proj[16];
	Math_Ortho(proj, 0, 1920, 0, 1080, 0.0f, 10.0f);
	Shaders_SetProjectionMatrix(proj);
	Shaders_SetViewMatrix(Matrix4().m);
	Shaders_SetModelMatrix(Matrix4().m);

    glDisable(GL_DEPTH_TEST);
	glViewport(0,0,1920, 1080);

	glClearColor(0.1,0.3,0.3,1);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	if(!videoPlaying && videoState.opened){

		videoLoading = false;
		videoPlaying = true;

		glDeleteTextures(1, &videoGlTexture);
		glGenTextures(1, &videoGlTexture);
		glBindTexture(GL_TEXTURE_2D, videoGlTexture);

		int w = videoState.width;
		int h = videoState.height;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if(!videoState.opened) videoPlaying = false;

	if(videoPlaying){

		glBindTexture(GL_TEXTURE_2D, videoGlTexture);
		videoPlane.Draw({(1920/2),(1080/2),0},{-1920,1080,0});

		Shaders_UseProgram(TEXT_2D_SHADER);
		Shaders_SetUniformColor({1,1,1,1});
		font.Clear();

        if(subtitleText.size()){
			font.AddText(subtitleText, 0, 1080-100, 0, 1, 1, 1920, 1080, 15);
			font.DrawAllText();
		}

	} else {

		tvMenu.Draw();
	}

    glEnable(GL_DEPTH_TEST);
}
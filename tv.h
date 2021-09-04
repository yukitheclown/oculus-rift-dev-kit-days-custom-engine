#ifndef TV_DEF
#define TV_DEF

#include "video_state.h"
#include "window.h"
#include "draw_plane.h"
#include "text.h"
#include <vector>
#include <string>

class FileBrowser {

public:

	FileBrowser(){}

	void GetContents(const std::wstring &dir);
	std::wstring GetDirectory() const {return inDirectory;}

	struct FileDir {
		std::string name;
		std::wstring path;
	};

	std::vector<FileDir> inDirectories;
	std::vector<FileDir> inFiles;

private:
	std::wstring inDirectory;
};

class TvMenuScreen {
public:

	TvMenuScreen(std::shared_ptr<TvMenuScreen> p, std::shared_ptr<FileBrowser> fb): prev(p), fileBrowser(fb){}

	struct Option {
		std::shared_ptr<TvMenuScreen> next;
		std::string text;
		std::wstring path;
		std::wstring dirPath;
	};

	std::vector<Option> options;
	std::shared_ptr<TvMenuScreen> prev;
	std::shared_ptr<FileBrowser> fileBrowser;
};

class TvScreen;

class TvMenu {

public:

	TvMenu():selection(0),scroll(0),maxLines(16){}
	void Draw();
	void SelectionUp();
	void SelectionDown();
	void Select(TvScreen *parent);
	void BackScreen();
	void Init();
	void RefreshFileOptions(bool );
	void Event(const SDL_Event &ev, TvScreen *parent);
private:

	void RefreshScreen();

	std::wstring homeDirectory;
	int selection;
	int scroll;
	int maxLines;

	std::shared_ptr<TvMenuScreen> onScreen;
	std::shared_ptr<TvMenuScreen> youtubeScreenBest;
	std::shared_ptr<TvMenuScreen> youtubeScreenStandard;
	std::shared_ptr<TvMenuScreen> youtubeScreenWorst;
	std::shared_ptr<TvMenuScreen> streamScreen;
	std::shared_ptr<TvMenuScreen> startScreen;
	std::shared_ptr<TvMenuScreen> creditsScreen;
	std::shared_ptr<TvMenuScreen> playScreen;
	std::shared_ptr<TvMenuScreen> cheatsScreen;

	FontFace font;
};

class TvScreen {

public:
	TvScreen():videoLoading(false), lastUpdateTime(-1), videoGlTexture(0), videoPlaying(false){}
	void Init();
	void Draw();
	void PlayVideo(const std::wstring &path, bool isStream = false, const std::string &command = "", const std::string &cookies = "");
	void Close();
	void Update();
	Vector3 GetAverageFrameColor();
	void Event(const SDL_Event &ev);

	bool videoLoading;
private:
	int lastUpdateTime;
	FontFace font;
	unsigned int videoGlTexture;
	VideoState videoState;
	std::string subtitleText;
	TvMenu tvMenu;
	bool videoPlaying;
	DrawPlane videoPlane;
};


#endif
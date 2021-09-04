#ifndef TEXT_DEF
#define TEXT_DEF

#include "ymath.h"
#include <string>
#include <vector>
extern "C"{
#include <ft2build.h>
#include <freetype2/freetype.h>
#include <freetype2/ftglyph.h>
#include <freetype2/ftoutln.h>
#include <freetype2/fttrigon.h>
#define FT_FREETYPE_H
}

#define COLOR_CHAR1 240
#define COLOR_CHAR2 241
#define COLOR_CHAR3 242
#define COLOR_CHAR4 243
#define COLOR_CHAR5 244
#define COLOR_CHAR6 239
#define COLOR_CHAR7 238
#define COLOR_CHAR8 237
#define TAB_SPACING 4

typedef struct {
    float ax;
    float ay;
    float bw;
    float bh;
    float bl;
    float bt;
    float tx;
} FontCharacter;

class FontFace {

public:

	FontFace(){}
	int DrawAllText();
	void Clear();
	int SetSize(int size);
	int LoadFont(const char *path);
	void AddText( const std::string &text, float x, float y, float z, float sx, float sy);
	void AddText( const std::string &text, float x, float y, float z, float sx, float sy, float maxWidth, float maxHeight, float padding = -1);
	float GetTextWidth(const std::string &text, float sx);
	float GetTextHeight(const std::string &text, float sy);
	std::string GetFittingString(const std::string &text, float sx, float x, float maxWidth);

private:

	struct Data {
		
		Data(){}
		~Data();
		Data(const Data&) = delete;
		Data(Data &&) = delete;
		Data &operator=(const Data&) = delete;
		Data &operator=(Data&&) = delete;
		
		unsigned int     fontTexture;
		FT_Face          fontFace;
		int              atlasWidth;
		int              atlasHeight;
		int              fontSize;
		int              fontHeight;
		FontCharacter    fontCharacters[128];

		std::vector<Vector2> texCoords;
		std::vector<Vector3> verts;
		std::vector<Vector4> colors;
	};

	std::shared_ptr<Data> data;
};

class Text {
public:

	static Text *Instance(){
		static Text t;
		return &t;
	}

	FT_Library GetFTLibrary(){ return ftLibrary; }

private:
	void Init();
	Text(){ Init(); }
	~Text();
	FT_Library  ftLibrary;
};

#endif
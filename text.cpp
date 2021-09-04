#pragma once
#include <GL/glew.h>
#include "math.h"
#include "shaders.h"
#include <stdio.h>
#include "stdafx.h"
#include <vector>
#include "text.h"

extern "C" {
#include <ft2build.h>
#include <freetype2/freetype.h>
#include <freetype2/ftglyph.h>
#include <freetype2/ftoutln.h>
#include <freetype2/fttrigon.h>
}

static float rgbToGL(float rgb){ return rgb / 255.0; }

int FontFace::LoadFont(const char *path){
    if(data || !Text::Instance()->GetFTLibrary()) return 0;

    data = std::shared_ptr<Data>(new Data);
    data->fontFace = nullptr;
    data->fontTexture = 0;

    if(FT_New_Face(Text::Instance()->GetFTLibrary(), path, 0, &data->fontFace)){
        printf("FontFace::LoadFont: Could not load font.\n");
        data = nullptr;
        return 0;
    }

    return 1;
}

int FontFace::SetSize(int size){
    if(!data || !data->fontFace) return 0;

    data->fontSize = size;

    FT_Set_Pixel_Sizes(data->fontFace, 0, size);
    glGenTextures(1, &data->fontTexture);
    glBindTexture(GL_TEXTURE_2D, data->fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    int w = 0, h = 0, i = 0, x = 0;

    FT_GlyphSlot g = data->fontFace->glyph;

    for(i = 32; i < 128; i++){
        if(FT_Load_Char(data->fontFace, i, FT_LOAD_RENDER)){
            printf("FontFace::SetFontSize: Error \n");
            continue;
        }
        w += g->bitmap.width;
        h  = h > g->bitmap.rows ? h : g->bitmap.rows;
    }

    data->fontHeight = h;

    data->atlasWidth  = w;
    data->atlasHeight = h;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,w,h,0,GL_ALPHA,GL_UNSIGNED_BYTE,0);

    for(i = 32; i < 128; i++){

        if(FT_Load_Char(data->fontFace, i, FT_LOAD_RENDER)){
            printf("FontFace::SetFontSize: Error \n");
            continue;
        }

        // unsigned char buffer[g->bitmap.width * g->bitmap.rows];
        // int bufferIndex = 0;

        // int bitWidth = g->bitmap.width;
        // int bitHeight = g->bitmap.rows;

        // int padding = 8 - (bitWidth % 8);

        // for(int y = 0; y < bitHeight; y++){
        //     for(int x = 0; x < bitWidth; x++){

        //         int index = (y * (bitWidth + padding)) + x;
        //         int onBit = 8 - ((index % 8) + 1);
        //         unsigned char bit = ((g->bitmap.buffer[(int)floor(index / 8.0f)] >> onBit) & 0x01) * 255;

        //         buffer[bufferIndex++] = bit;
        //     }
        // }

        glTexSubImage2D(GL_TEXTURE_2D,0,x,0,g->bitmap.width,g->bitmap.rows,GL_ALPHA,GL_UNSIGNED_BYTE,g->bitmap.buffer);
        data->fontCharacters[i].ax = g->advance.x >> 6;
        data->fontCharacters[i].ay = g->advance.y >> 6;
        data->fontCharacters[i].bw = g->bitmap.width;
        data->fontCharacters[i].bh = g->bitmap.rows;
        data->fontCharacters[i].bl = g->bitmap_left;
        data->fontCharacters[i].bt = g->bitmap_top;
        data->fontCharacters[i].tx = ((float)x / (float)w);
        x += g->bitmap.width;
    }
    return 1;
}

int FontFace::DrawAllText(){
    if(!data || !data->fontFace) return 0;

    Text2DShader::Instance()->UseAndBind();
    Shaders_UpdateProjectionMatrix();
    Shaders_UpdateViewMatrix();
    Shaders_UpdateModelMatrix();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->fontTexture);

    if(data->verts.size() > 0){

        Text2DShader::Instance()->GetPosVbo().Bind();
        Text2DShader::Instance()->GetPosVbo().SetData(data->verts.size()*sizeof(Vector3), &data->verts[0], GL_STATIC_DRAW);

        Text2DShader::Instance()->GetCoordVbo().Bind();
        Text2DShader::Instance()->GetCoordVbo().SetData(data->verts.size()*sizeof(Vector2), &data->texCoords[0], GL_STATIC_DRAW);

        Text2DShader::Instance()->GetColorVbo().Bind();
        Text2DShader::Instance()->GetColorVbo().SetData(data->verts.size()*sizeof(Vector4), &data->colors[0], GL_STATIC_DRAW);

        glCullFace(GL_BACK);
        glDrawArrays(GL_TRIANGLES, 0, data->verts.size());
        glCullFace(GL_FRONT);
    }

    return 1;

}

std::string FontFace::GetFittingString(const std::string &text, float sx, float x, float maxWidth){

    float width = 0;

    std::string ret, lastWord;

    for(int k = 0; k < (int)text.size(); k++){

        width += data->fontCharacters[(int)text[k]].ax * sx;

        if(width + x > maxWidth) return ret;

        lastWord += text[k];

        if(text[k] == ' ' || k == (int)text.size()-1){
            ret += lastWord;
            lastWord.clear();
            continue;
        }
    }

    if(ret.size() > 0 && ret[ret.size()-1] == ' ')
        ret.pop_back();

    return ret;
}

float FontFace::GetTextWidth(const std::string &text, float sx){
    float width = 0;
    for(int k = 0; k < (int)text.size(); k++){
        width += data->fontCharacters[(int)text[k]].ax * sx;
    }
    return width;
}

float FontFace::GetTextHeight(const std::string &text, float sy){
    float height = 0;
    for(int k = 0; k < (int)text.size(); k++){
        float h = (data->fontCharacters[(int)text[k]].bh * sy) + (data->fontCharacters[(int)text[k]].ay * sy);
        height = h > height ? h : height;
    }
    return height;
}

void FontFace::AddText( const std::string &text, float x, float y, float z, float sx, float sy,
    float maxWidth, float maxHeight, float padding){

    float marginX = maxWidth - x;

    float width = 0;

    if((width = GetTextWidth(text.c_str(), sx)) + x < maxWidth){
        float center = ((marginX / 2.0f) + x) - (width/2.0f);
        AddText(text, center, y, z, sx, sy);
        return;
    }

    struct SizeStr {
        float width;
        float height;
        std::string str;
    };

    std::vector<SizeStr> strsToAdd;

    float height = 0;

    unsigned int index = 0;

    while(index < text.size()){

        std::string fitting = GetFittingString(&text.c_str()[index], sx, x, maxWidth);

        width = GetTextWidth(fitting.c_str(), sx);

        float h = GetTextHeight(fitting.c_str(), sy) + padding;

        strsToAdd.push_back({width, h, fitting});

        height += h;

        index += fitting.size();
    }

    float topTextY = y + height/2.0 > maxHeight ? maxHeight - height - padding : y - (height/2.0);

    for(int k = 0; k < (int)strsToAdd.size(); k++){

        float center = ((marginX / 2.0f) + x) - (strsToAdd[k].width/2.0f);

        AddText(strsToAdd[k].str, center, topTextY, z, sx, sy);
        topTextY += strsToAdd[k].height + padding;
    }
}

void FontFace::AddText( const std::string &text, float x, float y, float z, float sx, float sy){
    if(!data) return;

    float startX = x;

    Vector4 currColor = Vector4(1,1,1,1);

    for(int m = 0; m < (int)text.size(); m++){

        char p = text[m];

        if(p == 0) continue;

        if(p == '\n'){
            y += data->fontSize*sy;
            x = startX;
            continue;
        }

        if(p == '\t' ){
            x += data->fontCharacters[(int)' '].ax * sx * TAB_SPACING;
            continue;
        }

        else if(p == (char)COLOR_CHAR1) { currColor = Vector4(rgbToGL(253),rgbToGL(253),rgbToGL(253),1); continue; }
        else if(p == (char)COLOR_CHAR2) { currColor = Vector4(rgbToGL(255),rgbToGL(131),rgbToGL(0), 1); continue; }
        else if(p == (char)COLOR_CHAR3) { currColor = Vector4(rgbToGL(26),rgbToGL(255),rgbToGL(0),1); continue; }
        else if(p == (char)COLOR_CHAR4) { currColor = Vector4(rgbToGL(0),rgbToGL(255),rgbToGL(247),1); continue; }
        else if(p == (char)COLOR_CHAR5) { currColor = Vector4(rgbToGL(255),rgbToGL(0),rgbToGL(23),1); continue; }
        else if(p == (char)COLOR_CHAR6) { currColor = Vector4(rgbToGL(150),rgbToGL(150),rgbToGL(150),1); continue; }
        else if(p == (char)COLOR_CHAR7) { currColor = Vector4(rgbToGL(255),rgbToGL(230),rgbToGL(0),1); continue; }
        else if(p == (char)COLOR_CHAR8) { currColor = Vector4(rgbToGL(0),rgbToGL(0),rgbToGL(0),1); continue; }

        float x2 =  x + data->fontCharacters[(int)p].bl * sx;
        float y2 = -y + data->fontCharacters[(int)p].bt * sy;
        float w = data->fontCharacters[(int)p].bw * sx;
        float h = data->fontCharacters[(int)p].bh * sy;

        x += data->fontCharacters[(int)p].ax * sx;
        y += data->fontCharacters[(int)p].ay * sy;

        if(!w || !h) continue;

        for(int f = 0; f < 6; f++) data->colors.push_back(currColor);


        data->verts.push_back({ x2,     -y2    , z});
        data->verts.push_back({ x2,     -y2 + h, z});
        data->verts.push_back({ x2 + w, -y2    , z});
        data->verts.push_back({ x2 + w, -y2    , z});
        data->verts.push_back({ x2,     -y2 + h, z});
        data->verts.push_back({ x2 + w, -y2 + h, z});
        data->texCoords.push_back({ data->fontCharacters[(int)p].tx, 0});
        data->texCoords.push_back({ data->fontCharacters[(int)p].tx,  (data->fontCharacters[(int)p].bh / data->atlasHeight) });
        data->texCoords.push_back({ data->fontCharacters[(int)p].tx + (data->fontCharacters[(int)p].bw / data->atlasWidth),  0 });
        data->texCoords.push_back({ data->fontCharacters[(int)p].tx + (data->fontCharacters[(int)p].bw / data->atlasWidth),  0 });
        data->texCoords.push_back({ data->fontCharacters[(int)p].tx,  (data->fontCharacters[(int)p].bh / data->atlasHeight) });
        data->texCoords.push_back({ data->fontCharacters[(int)p].tx + (data->fontCharacters[(int)p].bw / data->atlasWidth),
            (data->fontCharacters[(int)p].bh / data->atlasHeight)});
    }
}

void FontFace::Clear(){
    if(!data) return;
    data->verts.clear();
    data->texCoords.clear();
    data->colors.clear();
}

FontFace::Data::~Data(){
    if(fontTexture != 0) glDeleteTextures(1,&fontTexture);
    if(fontFace && Text::Instance()->GetFTLibrary()) FT_Done_Face(fontFace);
}

void Text::Init(){
    if(FT_Init_FreeType(&ftLibrary)) printf("Could not Init Freetype.\n");

}

Text::~Text(){

    if(ftLibrary) FT_Done_FreeType(ftLibrary);
    ftLibrary = nullptr;
}

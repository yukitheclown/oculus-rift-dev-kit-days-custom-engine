#include "stdafx.h"
#include "image_loader.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <png.h>

Image::Data::Data() : w(0), h(0), glTexture(-1) {}
Image::Data::~Data(){
    if(!glTexture) return;
    glDeleteTextures(1, &glTexture);
}

void Image::LoadPNG(const std::string &path, bool loadMipMaps){

    Delete();

	FILE *fp;
	fopen_s(&fp, path.c_str(), "rb");

    if( fp == NULL ){
        printf("Error loading PNG %s: No such file.\n", path.c_str());
        return;
    }

    unsigned char header[8];
    fread(header, 1, 8, fp);
    int ispng = !png_sig_cmp(header, 0, 8);

    if(!ispng){
        fclose(fp);
        printf("Not png %s\n", path.c_str());
        return;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        printf("Error loading %s. png_create_read_struct failed.\n",path.c_str() );
        fclose(fp);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr){
        printf("Error loading %s. png_create_info_struct failed.\n",path.c_str() );
        fclose(fp);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    if(setjmp(png_jmpbuf(png_ptr))){
        printf("Error loading %s. setjmp failed.\n",path.c_str() );
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return;
    }

    this->data = std::shared_ptr<Data>(new Data());
    glGenTextures(1, &data->glTexture);
	
	png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_uint_32 twidth, theight;

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if(bit_depth < 8)
        png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);

    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

    this->data->w = twidth;
    this->data->h = theight;

    png_read_update_info(png_ptr, info_ptr);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    png_byte *image_data = (png_byte *)malloc(sizeof(png_byte) * rowbytes * this->data->h);
    if(!image_data){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return;
    }

    png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * this->data->h);
    if(!row_pointers){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        free(image_data);
        return;
    }


    for(int i = 0; i < this->data->h; ++i)
        row_pointers[this->data->h - 1 - i] = image_data + i * rowbytes;

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    free(row_pointers);
    fclose(fp);

    glBindTexture(GL_TEXTURE_2D, data->glTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->data->w, this->data->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);


    if(loadMipMaps){
        glGenerateMipmap(GL_TEXTURE_2D);
        SetParameters(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
    }

    free(image_data);
}

void Image::SetParameters(const int &minFilter, const int &magFilter, const int &glTextureWrapS, const int &glTextureWrapT){
    if(!data) return;

    glBindTexture(GL_TEXTURE_2D, data->glTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,glTextureWrapS);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,glTextureWrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Image::Bind(){
    glBindTexture(GL_TEXTURE_2D, data->glTexture);
}

void Image::Delete(){
    if(!data) return;
    glDeleteTextures(1, &data->glTexture);
    data->glTexture = 0;
}

// #include "stdafx.h"
// #include "image_loader.h"
// #define GLEW_STATIC
// #include <GL/glew.h>
// #include <stdio.h>
// #include <iostream>
// #include <stdlib.h>
// extern "C" {
// #include <libavutil/imgutils.h>
// #include <libswscale/swscale.h>
// #include <libavformat/avformat.h>
//     // #include <png.h>
// }
// Image::Data::Data() : w(0), h(0), glTexture(-1) {}
// Image::Data::~Data(){
//     if(!glTexture) return;
//     glDeleteTextures(1, &glTexture);
// }

// void Image::LoadPNG(const std::string &path, bool loadMipMaps){

//     Delete();

//     this->data = std::shared_ptr<Data>(new Data());
//     glGenTextures(1, &data->glTexture);

//     AVInputFormat *iformat;
//     struct SwsContext *swsCtx = NULL;
//     AVFormatContext *formatCtx = NULL;
//     AVFrame *frame = NULL;
//     int frameDecoded = 0;
//     AVPacket pkt;
//     AVCodecContext *codecCtx = NULL;
//     AVCodec *codec = NULL;
//     uint8_t *imageData = NULL;
//     AVFrame *pFrameRGB = NULL;

//     iformat = av_find_input_format("image2");
//     if(avformat_open_input(&formatCtx, path.c_str(), iformat, nullptr) < 0){
//         printf("Error loading %s.\n",path.c_str() );
//         goto end;
//     }

//     codecCtx = formatCtx->streams[0]->codec;
//     codec = avcodec_find_decoder(codecCtx->codec_id);
//     if(!codec){
//         printf("Error loading %s\n.",path.c_str() );
//         goto end;
//     }

//     if(avcodec_open2(codecCtx, codec, nullptr) < 0){
//         printf("Error loading %s\n.",path.c_str() );
//         goto end;
//     }

//     if(!(frame == av_frame_alloc())){
//         printf("Error loading %s\n.",path.c_str() );
//         goto end;
//     }

//     if(av_read_frame(formatCtx, &pkt) < 0){
//         printf("Error loading %s\n.",path.c_str() );
//         goto end;
//     }

//     int ret = avcodec_decode_video2(codecCtx, frame, &frameDecoded, &pkt);

//     if(!frameDecoded || ret < 0){
//         printf("Error loading %s.\n",path.c_str() );
//         goto end;
//     }

//     swsCtx = sws_getContext(codecCtx->width, codecCtx->height,
//         codecCtx->pix_fmt, codecCtx->width, codecCtx->height,
//         PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

//     data->w = frame->width;
//     data->h = frame->height;

//     pFrameRGB = av_frame_alloc();

//     int numBytes = avpicture_get_size(PIX_FMT_RGB24, frame->width, frame->height);

//     imageData = (uint8_t *)av_malloc(numBytes);

//     sws_scale(swsCtx, (uint8_t const * const *)frame->data, frame->linesize, 0, frame->height,
//         pFrameRGB->data, pFrameRGB->linesize);

//     memmove(imageData, pFrameRGB->data[0], numBytes);

//     glBindTexture(GL_TEXTURE_2D, data->glTexture);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->data->w, this->data->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);


//     if(loadMipMaps){
//         glGenerateMipmap(GL_TEXTURE_2D);
//         SetParameters(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
//     }


//     end:

//     if(codecCtx) avcodec_close(codecCtx);
//     if(formatCtx) avformat_close_input(&formatCtx);
//     if(frame) av_freep(&frame);
//     if (pFrameRGB) av_free(pFrameRGB);
//     if (swsCtx) sws_freeContext(swsCtx);
//     av_free_packet(&pkt);

//     if(imageData) free(imageData);
// }

// void Image::SetParameters(const int &minFilter, const int &magFilter, const int &glTextureWrapS, const int &glTextureWrapT){
//     if(!data) return;

//     glBindTexture(GL_TEXTURE_2D, data->glTexture);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
//     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,glTextureWrapS);
//     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,glTextureWrapT);
//     glBindTexture(GL_TEXTURE_2D, 0);
// }

// void Image::Bind(){
//     glBindTexture(GL_TEXTURE_2D, data->glTexture);
// }

// void Image::Delete(){
//     if(!data) return;
//     glDeleteTextures(1, &data->glTexture);
//     data->glTexture = 0;
// }
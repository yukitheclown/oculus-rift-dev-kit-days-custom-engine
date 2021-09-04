#define GLEW_STATIC
#include "stdafx.h"
#include "stdafx.h"
#include <GL/glew.h>
#include <memory>
#include "framebuffers.h"
#include <stdio.h>

DepthBuffer::Data::Data() : width(0), height(0), depthTexture(0), fb(0) {}
DepthBuffer::Data::~Data(){
	if(depthTexture) glDeleteTextures(1, &depthTexture);
	if(fb) glDeleteFramebuffers(1, &fb);
}
ColorBuffer::Data::Data() : width(0), height(0), colorTexture(0), fb(0) {}
ColorBuffer::Data::~Data(){
	if(colorTexture) glDeleteTextures(1, &colorTexture);
	if(renderBuffer) glDeleteTextures(1, &renderBuffer);
	if(fb) glDeleteFramebuffers(1, &fb);
}
GBuffer::Data::Data() : width(0), height(0), textures(), fb(0) {}
GBuffer::Data::~Data(){
	if(fb) glDeleteFramebuffers(1, &fb);
	if(depthTexture) glDeleteTextures(1, &depthTexture);
	for(int k = 0; k < (int)textures.size(); k++) glDeleteTextures(1, &textures[k]);
}

static void GenerateDepthTex(int w, int h, GLuint *tex, int filterType){
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0); // OR GL_DEPTH_COMPONENT16
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType); // GL_NEAREST for SSR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType); // GL_NEAREST for SSR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
}


static void GenerateColorTex(int w, int h, GLuint *tex, int filterType){
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void DepthBuffer::Initialize(int w, int h){

	data = std::shared_ptr<Data>(new Data());

	data->width = w;
	data->height = h;

	glGenFramebuffers(1,&data->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, data->fb);    

	data->depthTexture = 0;
	GenerateDepthTex(w, h, &data->depthTexture, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, data->depthTexture, 0);
	
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		Destroy();
		printf("FrameBuffers_Init: Error creating depth buffer. \n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthBuffer::Destroy(){
	if(!data) return;
	glDeleteTextures(1, &data->depthTexture);
	glDeleteFramebuffers(1, &data->fb);
	data->depthTexture = data->fb = 0;
}

void ColorBuffer::Initialize(int w, int h){

	data = std::shared_ptr<Data>(new Data());

	data->width = w;
	data->height = h;
	GenerateColorTex(w, h, &data->colorTexture, GL_LINEAR);
	glGenRenderbuffers(1, &data->renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, data->renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	glGenFramebuffers(1,&data->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, data->fb);    
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, data->renderBuffer);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		Destroy();
		printf("FrameBuffers_Init: Error creating color buffer. \n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ColorBuffer::Destroy(){
	if(!data) return;
	glDeleteTextures(1, &data->colorTexture);
	glDeleteFramebuffers(1, &data->fb);
	glDeleteRenderbuffers(1, &data->renderBuffer);
	data->colorTexture = data->fb = data->renderBuffer = 0;
}

void GBuffer::Initialize(int w, int h, int num){

	if(num <= 0) return;

	data = std::shared_ptr<Data>(new Data());

	data->textures.resize(num);

	numTextures = num;
	data->width = w;
	data->height = h;


	glGenFramebuffers(1,&data->fb);
	glBindFramebuffer(GL_FRAMEBUFFER, data->fb);    

	GLuint *drawBuffers = new GLuint[num];
	for(int k = 0; k < numTextures; k++){
		GenerateColorTex(w, h, &data->textures[k], GL_NEAREST);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k, GL_TEXTURE_2D, data->textures[k], 0);
		drawBuffers[k] = GL_COLOR_ATTACHMENT0 + k;
	}

	glDrawBuffers(numTextures, drawBuffers);

	delete[] drawBuffers;

	GenerateDepthTex(w, h, &data->depthTexture, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, data->depthTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		Destroy();
		printf("FrameBuffers_Init: Error creating color buffer. \n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Destroy(){
	if(!data) return;
	glDeleteFramebuffers(1, &data->fb);
	glDeleteTextures(1, &data->depthTexture);
	for(int k = 0; k < (int)data->textures.size(); k++) glDeleteTextures(1, &data->textures[k]);
}
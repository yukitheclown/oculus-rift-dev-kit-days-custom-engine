#ifndef FRAMEBUFFERS_DEF
#define FRAMEBUFFERS_DEF

#include <stdio.h>
#include <string.h>
#include <vector>

class DepthBuffer {
public:
	
	DepthBuffer(){}
	void Destroy();
	void Initialize(int w, int h);
	unsigned int GetDepthTexture() const { if(!data) return 0; return data->depthTexture; }
	unsigned int GetFrameBuffer() const { if(!data) return 0; return data->fb; }
	int GetWidth() const { if(!data) return 0; return data->width; }
	int GetHeight() const { if(!data) return 0; return data->height; }
private:
	struct Data {
		Data();
		~Data();
		Data(const Data&) = delete;
		Data(Data &&) = delete;
		Data &operator=(const Data&) = delete;
		Data &operator=(Data&&) = delete;
		int width, height;
		unsigned int depthTexture;
		unsigned int fb;
	};

	std::shared_ptr<Data> data;
};

class ColorBuffer {
public:

	ColorBuffer(){}
	void Destroy();
	void Initialize(int w, int h);
	unsigned int GetColorTexture() const { if(!data) return 0; return data->colorTexture; }
	unsigned int GetFrameBuffer() const { if(!data) return 0; return data->fb; }
	int GetWidth() const { if(!data) return 0; return data->width; }
	int GetHeight() const { if(!data) return 0; return data->height; }
private:
	struct Data {
		Data();
		~Data();
		Data(const Data&) = delete;
		Data(Data &&) = delete;
		Data &operator=(const Data&) = delete;
		Data &operator=(Data&&) = delete;
		int width, height;
		unsigned int colorTexture;
		unsigned int renderBuffer;
		unsigned int fb;
	};

	std::shared_ptr<Data> data;
};

class GBuffer {
public:

	GBuffer(){}
	void Destroy();
	void Initialize(int w, int h, int numTextures );
	unsigned int GetDepthTexture() const { if(!data) return 0; return data->depthTexture; }
	unsigned int GetColorTexture() const { if(data && numTextures > 0) return data->textures[0]; else return 0; }
	unsigned int GetNormalTexture() const { if(data && numTextures > 1) return data->textures[1]; else return 0; }
	unsigned int GetPositionTexture() const { if(data && numTextures > 2) return data->textures[2]; else return 0; }
	unsigned int GetAttributeTexture() const { if(data && numTextures > 3) return data->textures[3]; else return 0; }
	unsigned int GetTexture(int t) const { if(!data && t < numTextures) return data->textures[t]; else return 0; }
	unsigned int GetFrameBuffer() const { if(!data) return 0; return data->fb; }
	int GetWidth() const { if(!data) return 0; return data->width; }
	int GetHeight() const { if(!data) return 0; return data->height; }
private:
	struct Data {
		Data();
		~Data();
		Data(const Data&) = delete;
		Data(Data &&) = delete;
		Data &operator=(const Data&) = delete;
		Data &operator=(Data&&) = delete;
		int width, height;
		std::vector<unsigned int> textures;
		unsigned int depthTexture;
		unsigned int fb;
	};
	int numTextures = 0;
	std::shared_ptr<Data> data;
};

#endif
#pragma once
#undef __STRICT_ANSI__
#include "ymath.h"
#include "OVR_CAPI_GL.h"
#include "OVR.h"
#include <GL/glew.h>

struct DepthFrameBuffer {
    GLuint        texId;

	DepthFrameBuffer(OVR::Sizei size);
};

//--------------------------------------------------------------------------
struct TextureBuffer {
    ovrSwapTextureSet* TextureSet;
    GLuint        texId;
    GLuint        fboId;
    OVR::Sizei    texSize;

	TextureBuffer(ovrHmd hmd, bool rendertarget, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data);
    OVR::Sizei GetSize(void) const{ return texSize; }
	void SetAndClearRenderSurface(DepthFrameBuffer * dbuffer);
	void UnsetRenderSurface();
};

class OculusHandler {
public:
	int CreateOVRWindow(bool renderForOculus);
	void Draw(void (*DrawFunc)(float *projMatrix, float *viewMatrix, int eyeIndex, Rect &viewport), int clearFlags);
	void GetProjViewMatricies(float *projMatrix, float *viewMatrix, int eye);
	int GetWindowWidth() const { return windowWidth; }
	int GetWindowHeight() const { return windowHeight; }
	int GetEyeWidth() const { return eyeWidth; }
	int GetEyeHeight() const { return eyeHeight; }
	Vector3 GetPosition() const { return position; }
	Vector3 GetUp() const { return up; }
	Vector3 GetForward() const { return forward; }
	Vector3 GetPlayerPosition() const { return playerPosition; }
	void SetPosition(const Vector3 &pos){ playerPosition = pos; }
	void SetRotation(const Quaternion &rot){ playerRotation = rot; }
	static OculusHandler &Instance() { static OculusHandler ins; return ins; }
	void GetMatricies(ovrPosef *eyeRenderPose, OVR::Matrix4f *view, OVR::Matrix4f *proj, int eye);
	Quaternion playerRotation;
	Vector3 playerPosition;
private:
	OculusHandler() : windowOpened(false) {}
	OculusHandler(const OculusHandler &o) = delete;
	OculusHandler(OculusHandler &&o) = delete;
	OculusHandler &operator = (OculusHandler &&o) = delete;
	OculusHandler &operator = (const OculusHandler &o) = delete;
	~OculusHandler(){ try { Destroy(); } catch(...){ printf("Failed to Destroy OculusHandler.\n"); } }
	int windowWidth, windowHeight;
	int eyeWidth, eyeHeight;
	ovrRecti eyeRenderViewport[2];
	ovrEyeRenderDesc eyeRenderDesc[2];
	bool renderingForOculus, windowOpened;
	ovrHmd hmd;
	Vector3 up, forward, position;
	OVR::Matrix4f eyeProjections[2];
    ovrGLTexture* mirrorTexture;
    GLuint mirrorFBO;
	TextureBuffer * eyeRenderTexture[2];
	DepthFrameBuffer * eyeDepthBuffer[2];

	void Destroy();
};
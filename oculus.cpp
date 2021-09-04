#include <GL/glew.h>
#include <vector>
#include "oculus.h"
#include "window.h"
#include "ymath.h"

DepthFrameBuffer::DepthFrameBuffer(OVR::Sizei size){

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLenum internalFormat = GL_DEPTH_COMPONENT24;
	GLenum type = GL_UNSIGNED_INT;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
}


TextureBuffer::TextureBuffer(ovrHmd hmd, bool rendertarget, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data){

	texSize = size;

	if (displayableOnHmd) {
		// This texture isn't necessarily going to be a rendertarget, but it usually is.
		ovrHmd_CreateSwapTextureSetGL(hmd, GL_RGBA, size.w, size.h, &TextureSet);
		for (int i = 0; i < TextureSet->TextureCount; ++i){
			ovrGLTexture* tex = (ovrGLTexture*)&TextureSet->Textures[i];
			glBindTexture(GL_TEXTURE_2D, tex->OGL.TexId);

			if (rendertarget){
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
		}
	}
	else{
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);

		if (rendertarget){
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.w, texSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	if (mipLevels > 1)
		glGenerateMipmap(GL_TEXTURE_2D);

	glGenFramebuffers(1, &fboId);
}

void TextureBuffer::SetAndClearRenderSurface(DepthFrameBuffer * dbuffer){
	ovrGLTexture* tex = (ovrGLTexture*)&TextureSet->Textures[TextureSet->CurrentIndex];

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0);

	glViewport(0, 0, texSize.w, texSize.h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TextureBuffer::UnsetRenderSurface(){
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

int OculusHandler::CreateOVRWindow(bool renderingForOculus){

	if(windowOpened) return 1;

	if (ovr_Initialize(nullptr) != ovrSuccess) { printf("ovr_Initialize failed.\n"); }

	ovrResult result = ovrHmd_Create(0, &hmd);

	if(!OVR_SUCCESS(result)) {
		ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
	}

	if (!OVR_SUCCESS(result)) {
		ovr_Shutdown();
		return 0;
	}

	windowWidth = eyeWidth = hmd->Resolution.w;
	windowHeight = eyeHeight = hmd->Resolution.h;

	//windowWidth /= 2;
	//windowHeight /= 2;

	this->renderingForOculus = renderingForOculus;

	if(!Window_Open("Rift", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0)){
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
		return 1;
	}

	if(renderingForOculus){
		OVR::Sizei recommendedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0);
		OVR::Sizei recommendedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0);

		OVR::Sizei renderTargetSize = OVR::Sizei(
			recommendedTex0Size.w + recommendedTex1Size.w,
			recommendedTex0Size.h > recommendedTex1Size.h ? recommendedTex0Size.h : recommendedTex1Size.h
		);

		eyeWidth = renderTargetSize.w/2;
		eyeHeight = renderTargetSize.h;

		eyeRenderTexture[0] = new TextureBuffer(hmd, true, true, recommendedTex0Size, 1, NULL);
		eyeDepthBuffer[0] = new DepthFrameBuffer(recommendedTex0Size);
		eyeRenderTexture[1] = new TextureBuffer(hmd, true, true, recommendedTex1Size, 1, NULL);
		eyeDepthBuffer[1] = new DepthFrameBuffer(recommendedTex1Size);

		eyeRenderViewport[0].Pos = OVR::Vector2i(0, 0);
		eyeRenderViewport[1].Pos = OVR::Vector2i((renderTargetSize.w + 1) / 2, 0);
		eyeRenderViewport[1].Size = OVR::Sizei(renderTargetSize.w/2, renderTargetSize.h);
		eyeRenderViewport[0].Size = OVR::Sizei(renderTargetSize.w/2, renderTargetSize.h);

        ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence|ovrHmdCap_DynamicPrediction);

	    ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation|ovrTrackingCap_MagYawCorrection|
	                                  ovrTrackingCap_Position, 0);

	    ovrHmd_CreateMirrorTextureGL(hmd, GL_RGBA, windowWidth, windowHeight, (ovrTexture**)&mirrorTexture);

	    glGenFramebuffers(1, &mirrorFBO);
	    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTexture->OGL.TexId, 0);
	    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		eyeRenderDesc[0] = ovrHmd_GetRenderDesc(hmd, ovrEye_Left, hmd->DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovrHmd_GetRenderDesc(hmd, ovrEye_Right, hmd->DefaultEyeFov[1]);

		eyeProjections[0] = ovrMatrix4f_Projection(eyeRenderDesc[0].Fov, 0.1f, 1000.0f, true);
		eyeProjections[1] = ovrMatrix4f_Projection(eyeRenderDesc[1].Fov, 0.1f, 1000.0f, true);
	}

	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
	windowOpened = true;
	return 0;
}

void OculusHandler::GetMatricies(ovrPosef *eyeRenderPose, OVR::Matrix4f *view, OVR::Matrix4f *proj, int eye){
	// eyeRenderPose->Position = OVR::Vector3f(eyeRenderPose->Position) * 10;
	// eyeRenderPose->Position = OVR::Vector3f(eyeRenderPose->Position);
    // eyeRenderPose->Position = OVR::Vector3f(eyeRenderPose->Position) * 1.5;
    eyeRenderPose->Position = OVR::Vector3f(eyeRenderPose->Position) * 2;
//
	OVR::Posef pose = OVR::Posef(*eyeRenderPose);

	OVR::Quatf baseQ = OVR::Quatf(playerRotation.x, playerRotation.y, playerRotation.z, playerRotation.w);
	pose.Rotation = baseQ * pose.Rotation;
    OVR::Vector3f forward = pose.Rotation.Rotate(OVR::Vector3f(0, 0,-1));
    OVR::Vector3f up = pose.Rotation.Rotate(OVR::Vector3f(0,1,0));
    pose.Translation = baseQ.Rotate(pose.Translation);
    pose.Translation.x += playerPosition.x;
    pose.Translation.y += playerPosition.y;
    pose.Translation.z += playerPosition.z;

    this->position = { pose.Translation.x, pose.Translation.y, pose.Translation.z};
    this->forward = { forward.x, forward.y, forward.z};
    this->up = {up.x, up.y, up.z};

	*view = OVR::Matrix4f::LookAtRH(pose.Translation, pose.Translation + forward, up);

	// if(renderingForOculus){
		*proj = eyeProjections[eye];
	// 	return;
	// }

	// Matrix4 tempProj;
	// tempProj.Perspective(60.0f*(.1415/180), (float)hmd->Resolution.w / (float)hmd->Resolution.h, 0.1f, 1000.0f);
	// memcpy(&proj->M[0][0], &tempProj.m[0], sizeof(float)*16);
}

void OculusHandler::GetProjViewMatricies(float *projMatrix, float *viewMatrix, int e){
	ovrEyeType eye = hmd->EyeRenderOrder[e];

	ovrPosef headPose[2];
    ovrVector3f hmdToEyeViewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset };
    ovrTrackingState hmdState;
    ovrHmd_GetEyePoses(hmd, 0, hmdToEyeViewOffset, headPose, &hmdState);

	ovrPosef eyeRenderPose = headPose[eye];

	OVR::Matrix4f view, proj;
	GetMatricies(&eyeRenderPose, &view, &proj, eye);

	memcpy(viewMatrix, &view.M[0][0], sizeof(float)*16);
	memcpy(projMatrix, &proj.M[0][0], sizeof(float)*16);
}

void OculusHandler::Draw(void (*DrawFunc)(float *projMatrix, float *viewMatrix, int eyeIndex, Rect &viewport), int clearFlags){

    ovrVector3f               ViewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset,
                                                eyeRenderDesc[1].HmdToEyeViewOffset };
    ovrPosef                  headPose[2];

    ovrFrameTiming   ftiming = ovrHmd_GetFrameTiming(hmd, 0);
    ovrTrackingState hmdState = ovrHmd_GetTrackingState(hmd, ftiming.DisplayMidpointSeconds);
    ovr_CalcEyePoses(hmdState.HeadPose.ThePose, ViewOffset, headPose);

    for (int eye = 0; eye<2; eye++){

		OVR::Matrix4f view, proj;
		GetMatricies(&headPose[eye], &view, &proj, eye);

        eyeRenderTexture[eye]->TextureSet->CurrentIndex =
            (eyeRenderTexture[eye]->TextureSet->CurrentIndex + 1) % eyeRenderTexture[eye]->TextureSet->TextureCount;

        eyeRenderTexture[eye]->SetAndClearRenderSurface(eyeDepthBuffer[eye]);

        float w = eyeRenderTexture[eye]->texSize.w;
        float h = eyeRenderTexture[eye]->texSize.h;

        Rect viewport = {0,0,0,w,h};

        DrawFunc(&proj.M[0][0], &view.M[0][0], eye, viewport);

        eyeRenderTexture[eye]->UnsetRenderSurface();
    }

    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = ViewOffset[0];
    viewScaleDesc.HmdToEyeViewOffset[1] = ViewOffset[1];

    ovrLayerEyeFov ld;
    ld.Header.Type  = ovrLayerType_EyeFov;
    ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    for (int eye = 0; eye < 2; eye++){
        ld.ColorTexture[eye] = eyeRenderTexture[eye]->TextureSet;
        ld.Viewport[eye]     = OVR::Recti(eyeRenderTexture[eye]->GetSize());
        ld.Fov[eye]          = hmd->DefaultEyeFov[eye];
        ld.RenderPose[eye]   = headPose[eye];
    }

    ovrLayerHeader* layers = &ld.Header;
    ovrHmd_SubmitFrame(hmd, 0, &viewScaleDesc, &layers, 1);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLint w = mirrorTexture->OGL.Header.TextureSize.w;
    GLint h = mirrorTexture->OGL.Header.TextureSize.h;
    glBlitFramebuffer(0, h, w, 0,
                      0, 0, w, h,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void OculusHandler::Destroy(){
	if(!windowOpened) return;
    glDeleteFramebuffers(1, &mirrorFBO);
    ovrHmd_DestroyMirrorTexture(hmd, (ovrTexture*)mirrorTexture);
    ovrHmd_DestroySwapTextureSet(hmd, eyeRenderTexture[0]->TextureSet);
    ovrHmd_DestroySwapTextureSet(hmd, eyeRenderTexture[1]->TextureSet);
	ovrHmd_Destroy(hmd);
	Window_Close();
	ovr_Shutdown();
	windowOpened = false;
}
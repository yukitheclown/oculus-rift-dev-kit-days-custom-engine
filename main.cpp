#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <memory>
#include "oculus.h"
#include "waifu.h"
#include "skeletal_animation.h"
#include "window.h"
#include "tv.h"
#include "shaders.h"
#include "framebuffers.h"
#include "draw_plane.h"
#include "video_state.h"

static OculusHandler *oHandler;

static DrawPlane tvPlane, watermarkPlane;
static TvScreen tvScreen;
//
static MeshInstance couchMeshInstance;
static Mesh couchMesh;

static ColorBuffer cBuffer;
static DepthBuffer dBuffer;

static Image watermarkImage;

static Vector3 tvPos;
static Vector3 lightDir = {0,0,-1};

static float tvRatio = 1920.0f/1080;
static Vector3 tvSize = {1*tvRatio,1,0};

static NeptuneWaifu waifu;

static Vector3 goal;

static float lightViewMatrix[16];
static float lightProj[16];

typedef struct {
	bool movingLeft;
	bool movingRight;
	bool movingForward;
	bool movingBackward;
	bool movingUp;
	bool movingDown;
	float theta;
} MovingDirs;

static MovingDirs movingDirs;

void Update(){

	tvScreen.Update();

	waifu.Update();

	static int lastTime = Window_GetTicks();
	int dt = Window_GetTicks() - lastTime;
	lastTime = Window_GetTicks();

	Vector3 vel = {0,0,0};

	float speed = 0.001;

	if(movingDirs.movingLeft) vel.x += dt*speed;
	if(movingDirs.movingRight) vel.x -= dt*speed;
	if(movingDirs.movingForward) vel.z += dt*speed;
	if(movingDirs.movingBackward) vel.z -= dt*speed;
	if(movingDirs.movingUp) vel.y -= dt*speed;
	if(movingDirs.movingDown) vel.y += dt*speed;

	Vector3 forward = oHandler->GetForward().normalize();

	Vector3 axis = forward.cross({0,0,1}).normalize();

	float theta = acos(forward.dot({0,0,1}));

	Vector3 b = axis.cross({0,0,1}).normalize();

	if (b.dot(forward) < 0) theta = -theta;

	vel = Quaternion(axis, theta) * vel;

	oHandler->SetRotation(Quaternion({0,1,0}, movingDirs.theta));

	oHandler->SetPosition(oHandler->GetPlayerPosition() + vel);
}

void Event(SDL_Event ev){

	static int lastTime = Window_GetTicks();
	int dt = Window_GetTicks() - lastTime;
	lastTime = Window_GetTicks();

	if(ev.type == SDL_KEYDOWN){
		if(ev.key.keysym.sym == SDLK_a) movingDirs.movingLeft = true;
		if(ev.key.keysym.sym == SDLK_d) movingDirs.movingRight = true;
		if(ev.key.keysym.sym == SDLK_w) movingDirs.movingForward = true;
		if(ev.key.keysym.sym == SDLK_s) movingDirs.movingBackward = true;
		if(ev.key.keysym.sym == SDLK_q) movingDirs.movingUp = true;
		if(ev.key.keysym.sym == SDLK_e) movingDirs.movingDown = true;
	}

	if(ev.type == SDL_KEYUP){
		if(ev.key.keysym.sym == SDLK_a) movingDirs.movingLeft = false;
		if(ev.key.keysym.sym == SDLK_d) movingDirs.movingRight = false;
		if(ev.key.keysym.sym == SDLK_w) movingDirs.movingForward = false;
		if(ev.key.keysym.sym == SDLK_s) movingDirs.movingBackward = false;
		if(ev.key.keysym.sym == SDLK_q) movingDirs.movingUp = false;
		if(ev.key.keysym.sym == SDLK_e) movingDirs.movingDown = false;
	}

	if(ev.type == SDL_MOUSEMOTION){
		movingDirs.theta -= ev.motion.xrel*0.000025*dt;
		Window_WarpMouse(Window_GetWindowWidth()/2.0, Window_GetWindowHeight()/2.0);
	}

	if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
	 	Window_Close();

	tvScreen.Event(ev);
}

void Quit(){}

void DrawTvScreen(){

	glBindFramebuffer(GL_FRAMEBUFFER, cBuffer.GetFrameBuffer());
	tvScreen.Draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderWorld(bool shadowPass){

	Shaders_UseProgram(TEXTURED_SHADER);

	Shaders_UpdateViewMatrix();
	Shaders_UpdateProjectionMatrix();
	Shaders_SetLightInvDir({-lightDir.x, -lightDir.y, -lightDir.z});
	Shaders_UpdateDepthMvpMatrix();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cBuffer.GetColorTexture());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dBuffer.GetDepthTexture());

	float trans[16];
	Math_TranslateMatrix(trans, {0,0,0});
	Shaders_SetModelMatrix(trans);
	Shaders_UpdateModelMatrix();

	Shaders_SetUniformColor({0,0,0,1});

	Shaders_DisableShadows();

	glCullFace(GL_FRONT);
	tvPlane.Draw(tvPos, tvSize);
	glCullFace(GL_BACK);

	Vector3 c = tvScreen.GetAverageFrameColor() / 800;
	Shaders_SetUniformColor({c.x,c.y,c.z,0.5});

	if(!shadowPass) Shaders_EnableShadows();

	couchMeshInstance.Translate(Vector3(0.25f,-1.1f,-0.1f));
	couchMeshInstance.UpdateMatrix();
	couchMeshInstance.Draw();

	waifu.color = {c.x, c.y, c.z, 1};
	waifu.Draw(lightDir, shadowPass);
}

void RenderShadowMap(){

	Shaders_SetViewMatrix(lightViewMatrix);
	Shaders_SetProjectionMatrix(lightProj);

	glBindFramebuffer(GL_FRAMEBUFFER, dBuffer.GetFrameBuffer());
	glViewport(0,0,dBuffer.GetWidth(),dBuffer.GetHeight());

	glClear(GL_DEPTH_BUFFER_BIT);

	RenderWorld(true);

	float depthMvp[16];
	Math_MatrixMatrixMult(depthMvp, lightProj, lightViewMatrix);
	Shaders_SetDepthMvpMatrix(depthMvp);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Draw(float *proj, float *view, int eyeIndex, Rect &viewport){

	Shaders_SetProjectionMatrix(proj);
	Shaders_SetViewMatrix(view);

	RenderWorld(false);

// 	float w = oHandler->GetEyeWidth();
// 	float h = oHandler->GetEyeHeight();

// 	float ortho[16];
// 	Math_Ortho(ortho, 0, w, 0, h, 0.0f, 10.0f);
// 	Shaders_SetProjectionMatrix(ortho);
// 	Shaders_SetViewMatrix(Matrix4().m);
// 	Shaders_SetModelMatrix(Matrix4().m);

//     glDisable(GL_DEPTH_TEST);
// 	glViewport(viewport.x,viewport.y,viewport.width, viewport.height);

// 	glActiveTexture(GL_TEXTURE0);

// 	glBindTexture(GL_TEXTURE_2D, watermarkImage.GetTexture());

// 	glDisable(GL_CULL_FACE);
// 	watermarkPlane.Draw({w/2,h/2,0}, {-w, -h,0});
// 	glEnable(GL_CULL_FACE);

// 	glEnable(GL_DEPTH_TEST);
}

bool Render(){

	DrawTvScreen();
	RenderShadowMap();

	Vector3 c = tvScreen.GetAverageFrameColor() / 500;
	glClearColor(c.x,c.y,c.z,1);

	oHandler->Draw(Draw, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

void OnResize(){

}

void Focused(){

}

int main(int, char**){

	av_register_all();
	avformat_network_init();

	memset(&movingDirs, 0, sizeof(MovingDirs));

	oHandler = &OculusHandler::Instance();

	if(oHandler->CreateOVRWindow(true)) return 0;

	oHandler->SetRotation(Math_Quat({0.0f,1.0f,0.0f},(float)PI));

	movingDirs.theta = (float)PI;

	cBuffer.Initialize(1920, 1080);
	dBuffer.Initialize(1024, 1024);

    glDisable(GL_DITHER);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	Shaders_Init();

	tvScreen.Init();

	waifu.Init(oHandler);

	waifu.waifuPos = Vector3(0.25f,-1.1f,-0.1f);

	Vector3 headPos = waifu.GetBonePos("Head") + Vector3(-0.75f, 0.2f, 0.7f);
	// Vector3 headPos = Vector3(0.25f,-1.1f,-0.1f) + Vector3(-0.75f, 0.2f, 0.7f);
	oHandler->SetPosition(headPos);
	tvPos = headPos + Vector3((waifu.waifuPos.x - headPos.x)/3.0f,0,1.15f);
	// tvPos = headPos + Vector3(0,0,1.15f);

	tvPlane.Create(TEXTURED_SHADER);
	// watermarkPlane.Create(TEXTURED_SHADER);

	// watermarkImage.LoadPNG("Resources/watermark.png");
	couchMesh.LoadObjFile("Resources/Couch/couch.yuk");
	couchMesh.InitializeDrawing("", TEXTURED_SHADER);
	couchMeshInstance = MeshInstance(&couchMesh);

	// tvScreen.PlayVideo("/home/yuki/Desktop/Fireplace, Soft Jazz & Rain - Romantic Date Night Inside.mp4");

	float theta = 80.0f*(3.1415/180);
	// Vector3 pos = Vector3(tvPos.x, tvPos.y, tvPos.z+0.6);
	Vector3 pos = Vector3(tvPos.x, tvPos.y, tvPos.z+1.5);
	Math_LookAt(lightViewMatrix,pos, pos + lightDir, Vector3(0, 1, 0));
	Math_Perspective(lightProj, theta, tvRatio, 0.1, 200);

	// tvScreen.PlayVideo(result, true);

	// tvScreen.PlayVideo("test.mkv");

	// std::string path = "https://www.youtube.com/watch?v=nuHfVn_cfHU";

	// tvScreen.PlayVideo("", true, "youtube-dl -f 18/17/22 -g --cookies " +
	// 			VideoState::CookiesFile + " " + path);

	Window_MainLoop(Update, Event, Render, Focused, OnResize, 1, 0, 0);

	tvScreen.Close();

	Shaders_Close();
	return 0;
}
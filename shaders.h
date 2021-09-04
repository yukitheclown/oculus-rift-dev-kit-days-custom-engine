#ifndef SHADERS_DEF
#define SHADERS_DEF
#include "ymath.h"
#include "vao_vbo.h"

#define SHADERS_POSITION_ATTRIB "pos"
#define SHADERS_NORM_ATTRIB "norm"
#define SHADERS_COLOR_ATTRIB "color"
#define SHADERS_COORD_ATTRIB "coord"


class Text2DShader {
public:
	void UseAndBind();

	static Text2DShader *Instance(){
		static Text2DShader t;
		return &t;
	}

	Vao GetVao() const { return vao; }
	Vbo GetPosVbo() const { return posVbo; }
	Vbo GetCoordVbo() const { return coordVbo; }
	Vbo GetColorVbo() const { return colorVbo; }
private:
	void Init();
	Text2DShader(){ Init(); }
	Vao vao;
	Vbo posVbo, colorVbo, coordVbo;
};

class PostProcessingShader {
public:
	void UseAndBind();

	static PostProcessingShader *Instance(){
		static PostProcessingShader t;
		return &t;
	}

	void Render();

private:
	void Init();
	PostProcessingShader(){ Init(); }
	Vao vao;
	Vbo posVbo;
};

enum {
    TEXTURELESS_SHADER = 0,
    TEXTURED_SHADER,
    SKELETAL_ANIMATION_SHADER,
    PARTICLE_SHADER,
    WATER_SHADER,
	TEXT_2D_SHADER,
	TEXT_3D_SHADER,
	TEXTURELESS_2D_SHADER,
	POST_PROCESSING_SHADER,
	TEXTURED_2D_SHADER,
};

void Shaders_SetLightPos(const Vector3 &pos);
void Shaders_DisableShadows();
void Shaders_EnableShadows();
void Shaders_Init();
unsigned int Shaders_GetProgram(int program);
void Shaders_Close();
void Shaders_SetViewMatrix(float *matrix);
void Shaders_UpdateViewMatrix();
void Shaders_GetViewMatrix(float *matrix);
void Shaders_SetProjectionMatrix(float *matrix);
void Shaders_UpdateProjectionMatrix();
void Shaders_GetProjectionMatrix(float *matrix);
void Shaders_SetModelMatrix(float *matrix);
void Shaders_UpdateModelMatrix();
void Shaders_GetModelMatrix(float *matrix);
void Shaders_SetDepthMvpMatrix(float *matrix);
void Shaders_UpdateDepthMvpMatrix();
void Shaders_GetDepthMvpMatrix(float *matrix);
void Shaders_UseProgram(int program);
void Shaders_GetInvViewMatrix(float *matrix);
void Shaders_SetUniformColor(Vector4 color);
void Shaders_SetCameraClipMatrix(float *matrix);
void Shaders_UpdateCameraClipMatrix();
void Shaders_GetCameraClipMatrix(float *matrix);
void Shaders_DisableColorAttrib();
void Shaders_EnableColorAttrib();
void Shaders_SetRgbSplitAmount(float a);
void Shaders_SetBlurAmount(float a);
void Shaders_SetReflectionAmount(float a);
void Shaders_SetGlowAmount(float a);
void Shaders_SetLightInvDir(const Vector3 &dir);
unsigned int Shaders_GetBonesLocation();


#endif
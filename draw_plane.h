#pragma once
#include "ymath.h"
#include "vao_vbo.h"
#include "shaders.h"

class DrawPlane {
public:
	DrawPlane(){}
	void Create(int shader);
	void Draw(Vector3 pos, Vector3 scale);
	void Draw(Vector3 pos, Vector3 scale, Vector4 c[4]);
	void Draw(Vector3 pos, Vector2 scale, Vector4 c[4]) { Draw(pos, {scale.x, scale.y, 0}, c); }
	void Draw(Vector3 pos, Vector2 scale) { Draw(pos, {scale.x, scale.y, 0} ); }
	void DrawGlitched(Vector3 pos, Vector2 scale, float gAmount);
	Vao vao;
	Vbo posVbo;
	Vbo uvVbo;
	Vbo colorVbo;
	Vbo normVbo;
	int shader;
};
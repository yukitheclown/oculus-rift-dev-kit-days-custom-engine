#include "stdafx.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include "shaders.h"
#include "draw_plane.h"

void DrawPlane::Create(int shader){

	this->shader = shader;

	Shaders_UseProgram(shader);
	this->vao.Create();
	this->vao.Bind();
	this->posVbo.Create(VboType::Vertex);
	this->colorVbo.Create(VboType::Vertex);
	this->normVbo.Create(VboType::Vertex);
	this->uvVbo.Create(VboType::Vertex);

	unsigned int posAttrib = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_POSITION_ATTRIB);
	unsigned int colorAttrib = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_COLOR_ATTRIB);
	unsigned int uvAttrib = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_COORD_ATTRIB);
	unsigned int normalAttrib = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_NORM_ATTRIB);

	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(colorAttrib);
	glEnableVertexAttribArray(normalAttrib);
	glEnableVertexAttribArray(uvAttrib);

	this->posVbo.Bind();
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	this->colorVbo.Bind();
	glVertexAttribPointer(colorAttrib, 4, GL_FLOAT, GL_FALSE, 0, 0);

	this->uvVbo.Bind();
	glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	this->normVbo.Bind();
	glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

	Vector4 color = { 1, 1, 1, 1 };
	Vector4 colors[] = { color, color, color, color, color, color };

	this->colorVbo.Bind();
	this->colorVbo.SetData(sizeof(Vector4) * 6, &colors[0].x, GL_STATIC_DRAW);
}

void DrawPlane::Draw(Vector3 pos, Vector3 scale, Vector4 c[4]){

	Shaders_UseProgram(shader);
	vao.Bind();

	Vector4 colors[] = { c[0], c[1], c[2], c[2], c[3], c[0] };

	this->colorVbo.Bind();
	this->colorVbo.SetData(sizeof(Vector4) * 6, &colors[0].x, GL_STATIC_DRAW);
	Shaders_EnableColorAttrib();
	Draw(pos, scale);
	Shaders_DisableColorAttrib();
}

void DrawPlane::Draw(Vector3 pos, Vector3 scale){
	Shaders_UseProgram(shader);

	Shaders_UpdateViewMatrix();
	Shaders_UpdateProjectionMatrix();
	Shaders_UpdateModelMatrix();

	vao.Bind();

	Vector3 verts[] = {
		{ pos.x + (-0.5f*scale.x), pos.y + (-0.5f*scale.y), pos.z + (-0.5f*scale.z) },
		{ pos.x + (0.5f*scale.x), pos.y + (-0.5f*scale.y), pos.z + (-0.5f*scale.z) },
		{ pos.x + (0.5f*scale.x), pos.y + (0.5f*scale.y), pos.z + (0.5f*scale.z) },
		{ pos.x + (0.5f*scale.x), pos.y + (0.5f*scale.y), pos.z + (0.5f*scale.z) },
		{ pos.x + (-0.5f*scale.x), pos.y + (0.5f*scale.y), pos.z + (0.5f*scale.z) },
		{ pos.x + (-0.5f*scale.x), pos.y + (-0.5f*scale.y), pos.z + (-0.5f*scale.z) },
	};

	this->posVbo.Bind();
	this->posVbo.SetData(sizeof(verts), &verts[0].x, GL_STATIC_DRAW);

	Vector2 coords[] = {
		{ 1, 0 },
		{ 0, 0 },
		{ 0, 1 },
		{ 0, 1 },
		{ 1, 1 },
		{ 1, 0 },
	};

	this->uvVbo.Bind();
	this->uvVbo.SetData(sizeof(coords), &coords[0].x, GL_STATIC_DRAW);

	Vector3 norm = (verts[0] - verts[1]).cross((verts[0] - verts[2]))*-1;
	Vector3 norms[] = { norm, norm, norm, norm, norm, norm };

	this->normVbo.Bind();
	this->normVbo.SetData(sizeof(norms), &norms[0].x, GL_STATIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void AddGlitchRect(std::vector<Vector3> &verts, std::vector<Vector2> &coords, Vector2 tPos, Vector2 tSize,
	Vector3 vPos, Vector2 vSize){

	Vector3 v[] = {
		{ vPos.x + (-0.5f*vSize.x), vPos.y + (-0.5f*vSize.y), vPos.z + 0.5f },
		{ vPos.x + (0.5f*vSize.x), vPos.y + (-0.5f*vSize.y), vPos.z + 0.5f },
		{ vPos.x + (0.5f*vSize.x), vPos.y + (0.5f*vSize.y), vPos.z + 0.5f },
		{ vPos.x + (0.5f*vSize.x), vPos.y + (0.5f*vSize.y), vPos.z + 0.5f },
		{ vPos.x + (-0.5f*vSize.x), vPos.y + (0.5f*vSize.y), vPos.z + 0.5f },
		{ vPos.x + (-0.5f*vSize.x), vPos.y + (-0.5f*vSize.y), vPos.z + 0.5f },
	};


	Vector2 c[] = {
		{ tPos.x + tSize.x, tPos.y + 0 },
		{ tPos.x + 0, tPos.y + 0 },
		{ tPos.x + 0, tPos.y + tSize.y },
		{ tPos.x + 0, tPos.y + tSize.y },
		{ tPos.x + tSize.x, tPos.y + tSize.y },
		{ tPos.x + tSize.x, tPos.y + 0 },
	};

	for (int k = 0; k < 6; k++) { verts.push_back(v[k]); coords.push_back(c[k]); }
}
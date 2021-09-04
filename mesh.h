#pragma once
#include <vector>
#include <stdio.h>
#include "image_loader.h"
#include "bounding_box.h"
#include "shaders.h"
#include "object.h"
#include "ymath.h"
#include "vao_vbo.h"

class RiggedMesh;

typedef struct {
    float r;
    float g;
    float b;
} Color;

class Material {
public:
	Material():startingElementIndex(0), texIndex(0){}
	int startingElementIndex;
    // Color specular;
    // Color diffuse;
    // Color ambient;
    // float specularExponent;
    // float transparency;
    Image diffuseMapImage;
    // Image specularMapImage;
    // Image bumpMapImage;
    std::string name;
    int texIndex;
};

class Mesh {
public:
	Mesh() :
		verts(),
		norms(),
		texCoords(),
		weights(),
		boneIndices(),
		modelSize(0),
		modelElementsSize(0),
		materials(),
		img(),
		bb(),
		shader(0),
		drawingElements(true),
		drawingInitialized(false),
		vao(),
		posVbo(),
		uvVbo(),
		normVbo()
	{}

	int LoadObjFile(const char *fileName, bool loadUvs, bool loadNormals, bool loadElements, RiggedMesh *riggedMesh);
	int LoadObjFile(const char *fileName, bool loadUvs, bool loadNormals, bool loadElements)
	{ return LoadObjFile(fileName, loadUvs, loadNormals, loadElements, nullptr); }
	int LoadObjFile(const char *fileName, bool loadUvs, bool loadNormals){ return LoadObjFile(fileName, loadUvs, loadNormals, true, nullptr); }
	int LoadObjFile(const char *path, bool loadUvs){ return LoadObjFile(path, loadUvs, true, true, nullptr); }
	int LoadObjFile(const char *path){ return LoadObjFile(path, true, true, true, nullptr); }
	void InitializeDrawing(const char *texturePath, int shader);
	void InitializeDrawing(const char *texturePath){ InitializeDrawing(texturePath, TEXTURED_SHADER); this->shader = TEXTURED_SHADER; }
	void InitializeDrawing(int shader){ InitializeDrawing("",shader); this->shader = shader; }
	void InitializeDrawing(){ InitializeDrawing(""); }
	void Draw(std::vector<Material> *overrideMaterials);
	void Draw(){ Draw(nullptr); }
	void DrawElements(std::vector<Material> *overrideMaterials);
	void ReloadData();
	int GetShader() const { return shader; }
	unsigned int GetTexture() const { return img.GetTexture(); }
	BoundingBox GetBoundingBox(){ return bb; }
private:
	std::vector<Vector3> verts;
	std::vector<Vector3> norms;
	std::vector<Vector2> texCoords;
	std::vector<Vector3> weights;
	std::vector<Vector3> boneIndices;
	std::vector<int> elements;
	int modelSize; 
	int modelElementsSize;
	std::vector<Material> materials;
	Image img;
	BoundingBox bb;
	int shader;
	bool drawingElements, drawingInitialized;
	Vao vao;
	Vbo posVbo;
	Vbo uvVbo;
	Vbo normVbo;
	Vbo boneWeightVbo;
	Vbo boneIndexVbo;
	void LoadMTL(const std::string &fileName);
};

class MeshInstance : public Object {
public:
	Mesh *mesh = nullptr;
	int shader;
	MeshInstance(){}
	MeshInstance(Mesh *m){ mesh = m; bb = m->GetBoundingBox(); shader = mesh->GetShader(); }
	void Draw(std::vector<Material> *overrideMaterials);
	void Draw(){ Draw(nullptr); }
	int GetShader() override { return shader; }
	void SetShader(int s) { shader = s; }
	int GetTexture() override { if(!mesh) return -1; else return mesh->GetTexture(); }
    int CastsShadow() override { return 1; }
};
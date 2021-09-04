#pragma once
#include <vector>
#include "ymath.h"
#include "bounding_box.h"

class OctreeLeaf;

class Object {
public:
	Object();
	virtual ~Object() {}
	virtual void Draw() = 0;
	virtual int GetShader() = 0;
	virtual int GetTexture() = 0;
    virtual int CastsShadow() = 0;

	BoundingBox *GetBoundingBox() { return &bb; }
	Vector4 GetColor(){ return color; }
	void SetColor(const Vector4 &c){ this->color = c; }
	Matrix4 GetModelMatrix() { return modelMatrix; }
	void Scale(Vector3 s){ scale = s; }
	void Rotate(Vector3 r){ rotation = r; }
	void Translate(Vector3 t){ worldSpace = t; }
	void UpdateMatrix();
	Vector3 GetPosition() const { return worldSpace; }
	Vector3 GetRotation() const { return rotation; }
	Vector3 GetScale() const { return scale; }

	void SetInOctant(OctreeLeaf *o){ inOctant = o; }
	OctreeLeaf *InOctant() const { return inOctant; }
protected:
	OctreeLeaf *inOctant;
	Vector3 rotation, worldSpace;
	Vector3 scale;
	Vector4 color;
	BoundingBox bb;
	Matrix4 modelMatrix;
private:

};
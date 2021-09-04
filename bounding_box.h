#pragma once
#include <vector>
#include <string>
#include <stdio.h>
#include "ymath.h"

struct MTVOverlap {
	MTVOverlap() : sOverlap(FLT_MAX), bbPointIndex(0), inside(true) {}
    Vector3 overlap;
    float sOverlap;
    Vector3  sAxis;
    int bbPointIndex;
    Vector3 normal;
    bool inside;
};

class BoundingBox {
public:
	BoundingBox(BoundingBox *parent, float w, float h, float d, Vector3 ms):
		width (w),
		height (h),
		depth (d),
		modelSpace (ms),
		points(),
		normals(),
		name(),
		parent(parent),
		pointsNeedRecalculating(true),
		modelMatrix(),
		subBoundingBoxes()
	{
		RecalculatePoints();
	}

	BoundingBox(BoundingBox *parent){ BoundingBox(parent, 0, 0, 0, {0,0,0}); }
	BoundingBox(){BoundingBox(nullptr);}
	bool IsCompletelyInside(BoundingBox *b);
	void RecalculatePoints(){ RecalculatePointsMatrix(modelMatrix); }
	int CheckCollision(BoundingBox *b2, MTVOverlap *o);
	int CheckCollision(BoundingBox *b2, MTVOverlap *overlap, bool *isInside);
	int CheckCollision(BoundingBox *b2);
	int CheckCollisionRay(Ray r);
	void GetCollisionRay(Ray r, BoundingBox **bb, float *distanceFromEye);
	void SetName(const std::string &name){ this->name = name; }
	void SetModelMatrix(const Matrix4 &m){ modelMatrix = m; pointsNeedRecalculating = true; }
	float GetWidth() const {return width; }
	float GetHeight() const {return height; }
	float GetDepth() const {return depth; }
	Vector3 GetModelSpace() const {return modelSpace; }
	Vector4 (&Points())[8] { return points; }
	Vector3 (&Normals())[6] { return normals; }
	std::vector<BoundingBox> SubBoundingBoxes() {return subBoundingBoxes; };
private:
	float width;
	float height;
	float depth;
	Vector3 modelSpace;
	Vector4 points[8];
	Vector3 normals[6];
	std::string name;
	BoundingBox *parent;
	bool pointsNeedRecalculating;
	Matrix4 modelMatrix;
	std::vector<BoundingBox> subBoundingBoxes;
	void RecalculatePointsMatrix(Matrix4 m);
};
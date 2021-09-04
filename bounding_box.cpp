#include "stdafx.h"
#include "bounding_box.h"
#include <stdio.h>
#include "math.h"

void BoundingBox::RecalculatePointsMatrix(Matrix4 matrix){
	if(!pointsNeedRecalculating) return;

	Vector4 p[8] = {
		{modelSpace.x, modelSpace.y, modelSpace.z, 1},
		{modelSpace.x+width, modelSpace.y, modelSpace.z, 1},
		{modelSpace.x+width, modelSpace.y+height,modelSpace.z, 1},
		{modelSpace.x, modelSpace.y+height,modelSpace.z, 1},
		{modelSpace.x, modelSpace.y, modelSpace.z+depth, 1},
		{modelSpace.x+width, modelSpace.y, modelSpace.z+depth, 1},
		{modelSpace.x+width, modelSpace.y+height,modelSpace.z+depth, 1},
		{modelSpace.x, modelSpace.y+height,modelSpace.z+depth, 1}
	};

	for(int k = 0; k < 8; k++)
		this->points[k] = matrix * p[k];

	Vector4 vecs[] = {
		this->points[0] - this->points[1],
		this->points[0] - this->points[2],
		this->points[4] - this->points[6],
		this->points[4] - this->points[5],
		this->points[0] - this->points[3],
		this->points[0] - this->points[4],
		this->points[1] - this->points[5],
		this->points[1] - this->points[2],
		this->points[3] - this->points[2],
		this->points[3] - this->points[6],
		this->points[0] - this->points[4],
		this->points[0] - this->points[1],
	};

	for(int k = 0; k < 6; k++){
		Vector3 v1 = Vector3(vecs[k*2].x,vecs[k*2].y,vecs[k*2].z);
		Vector3 v2 = Vector3(vecs[(k*2)+1].x,vecs[(k*2)+1].y,vecs[(k*2)+1].z);
		normals[k] = v1.cross(v2);
		normals[k].normalize();
	}

	for(int k = 0; k < (int)subBoundingBoxes.size(); k++)
		subBoundingBoxes[k].RecalculatePointsMatrix(matrix);

	pointsNeedRecalculating = false;
}


static int CheckAxis(Vector4 *s1, int s1Sides, Vector4 *s2, int s2Sides, Vector3 a, MTVOverlap *overlap){
    
    struct {
    	float min;
    	float max;
    	int maxIndex;
    	int minIndex;
    	bool minSet;
    	bool maxSet;
    } minMax[2];

    memset(minMax, 0, sizeof(minMax));

    int m;
    for( m = 0; m < s1Sides; m++){
        Vector3 temp = Vector3(s1[m].x,s1[m].y,s1[m].z);
        float dotval = temp.dot(a);
        if(dotval < minMax[0].min || !minMax[0].minSet) { minMax[0].minIndex = m; minMax[0].min = dotval; minMax[0].minSet = true; }
        if(dotval > minMax[0].max || !minMax[0].maxSet) { minMax[0].maxIndex = m; minMax[0].max = dotval; minMax[0].maxSet = true; }
    }

    for(m = 0; m < s2Sides; m++){
        Vector3 temp = Vector3(s2[m].x,s2[m].y,s2[m].z);
        float dotval = temp.dot(a);
        if(dotval < minMax[1].min || !minMax[1].minSet) { minMax[1].minIndex = m; minMax[1].min = dotval; minMax[1].minSet = true; }
        if(dotval > minMax[1].max || !minMax[1].maxSet) { minMax[1].maxIndex = m; minMax[1].max = dotval; minMax[1].maxSet = true; }
    }

    if(minMax[0].min > minMax[1].max || minMax[1].min > minMax[0].max)
		return 1; 

    if(!(minMax[0].min > minMax[1].min || minMax[0].max < minMax[1].max))
    	overlap->inside = false;

    float option;
    float option1 = minMax[0].max - minMax[1].min;
    float option2 = minMax[1].max - minMax[0].min;
    float option3 = minMax[0].min - minMax[1].max;

    if(minMax[0].min < minMax[1].min)
        if(minMax[0].max < minMax[1].max)
			option = option1;
        else
            if(option1 < option2) option = option1; else option = -option2;
    else
        if(minMax[0].max > minMax[1].max)
			option = option3;            
        else
            if(option1 < option2) option = option1; else option = -option2;

    float absol = fabs(option);
    if(absol < overlap->sOverlap ) {
        overlap->sOverlap = absol;
        overlap->sAxis  = a;
        if(option == option1) overlap->bbPointIndex = minMax[0].maxIndex;
        if(option == option2) overlap->bbPointIndex = minMax[1].maxIndex;
        if(option == option3) overlap->bbPointIndex = minMax[0].minIndex;
        if(option < 0) {
            overlap->sAxis.x = -a.x;
            overlap->sAxis.y = -a.y;
            overlap->sAxis.z = -a.z;
        }
    }

    return 0;
}

int BoundingBox::CheckCollision(BoundingBox *b2, MTVOverlap *overlap, bool *isInside){

	RecalculatePoints();
	b2->RecalculatePoints();

    MTVOverlap mtv;

    int k;
    for(k = 0; k < 6; k++)
        if(CheckAxis(points, 8, b2->points, 8, b2->normals[k], &mtv)) return 0;
    
    if(mtv.inside && isInside)
    	*isInside = true;

    for(k = 0; k < 6; k++)
        if(CheckAxis(points, 8, b2->points, 8, normals[k], &mtv)) return 0;

    if(!overlap) return 1;

    mtv.overlap.x =  mtv.sAxis.x * mtv.sOverlap;   
    mtv.overlap.y =  mtv.sAxis.y * mtv.sOverlap;
    mtv.overlap.z =  mtv.sAxis.z * mtv.sOverlap;

    overlap->overlap += mtv.overlap;
    overlap->sAxis = mtv.sAxis;

    return 1;
}

bool BoundingBox::IsCompletelyInside(BoundingBox *b){
	bool inside = false;
	CheckCollision(b, 0, &inside);
	return inside;
}

int BoundingBox::CheckCollision(BoundingBox *b2, MTVOverlap *overlap){
	return CheckCollision(b2, overlap, nullptr);
}

int BoundingBox::CheckCollision(BoundingBox *b2){
	return CheckCollision(b2, nullptr);
}

static bool SameSide(Vector3 p, Vector3 a, Vector3 b, Vector3 c){
	Vector3 cp0 = (b-a).cross((p-a));
	Vector3 cp1 = (b-a).cross((c-a));
	if(cp0.dot(cp1) >= 0) return true;
	return false;
}

void BoundingBox::GetCollisionRay(Ray r, BoundingBox **b, float *distanceFromEye){

	*b = nullptr;

	RecalculatePoints();

	Vector4 planes[6][4] = {
		{points[0],points[1],points[2],points[3]},
		{points[4],points[5],points[1],points[0]},
		{points[3],points[2],points[6],points[7]},
		{points[5],points[4],points[7],points[6]},
		{points[1],points[5],points[6],points[2]},
		{points[4],points[0],points[3],points[7]},
	};

	int k;
	for(k = 0; k < 6; k++){

		Vector4 v0 = planes[k][0] - planes[k][1];
		Vector4 v1 = planes[k][0] - planes[k][2];
		Vector4 n = (v0.cross(v1)).normalize();

		float d = (planes[k][0].xyz() - r.pos).dot(n.xyz()) / (r.line.dot(n.xyz()));
		if(d < 0) continue;

		Vector3 collisionPoint = r.pos + (r.line*d);

		if( SameSide(collisionPoint, planes[k][1].xyz(), planes[k][2].xyz(), planes[k][3].xyz()) &&
			SameSide(collisionPoint, planes[k][2].xyz(), planes[k][3].xyz(), planes[k][0].xyz()) &&
			SameSide(collisionPoint, planes[k][3].xyz(), planes[k][0].xyz(), planes[k][1].xyz()) &&
			SameSide(collisionPoint, planes[k][0].xyz(), planes[k][1].xyz(), planes[k][2].xyz())){

			if((int)subBoundingBoxes.size()){
				for(int k = 0; k < (int)subBoundingBoxes.size(); k++){
					BoundingBox *bb = nullptr;
					float distance;
					subBoundingBoxes[k].GetCollisionRay(r, &bb, &distance);
					if(bb && (distance < *distanceFromEye || !*b)){
						*distanceFromEye = distance;
						*b = bb;
					}
				}

				return;
			} else {
				*b = this;
				*distanceFromEye = d;
				return;
			}
		}
	}

	return;
}

int BoundingBox::CheckCollisionRay(Ray r){
	BoundingBox *b = nullptr;
	float distanceFromEye;
	GetCollisionRay(r, &b, &distanceFromEye);
	if(!b) return 0;
	return 1;
}
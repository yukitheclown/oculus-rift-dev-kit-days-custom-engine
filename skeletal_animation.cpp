#define FRAME_RATE 30
#define GLEW_STATIC
#include <GL/glew.h>
#include "skeletal_animation.h"
#include "mesh.h"
#include "image_loader.h"
#include "shaders.h"
#include "window.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

Bone *RiggedMesh_FindBoneFromIndex(Bone *b, int index){

	if(!b) return nullptr;

	if(b->index == index)
		return b;

	int k;
	for(k = 0; k < (int)b->children.size(); k++){
		Bone *bone = nullptr;
		bone = RiggedMesh_FindBoneFromIndex(b->children[k].get(), index);
		if(bone) return bone;
	}

	return nullptr;
}

Bone *RiggedMesh_FindBoneFromName(Bone *b, const std::string &name){

	if(b->name == name)
		return b;

	int k;
	for(k = 0; k < (int)b->children.size(); k++){
		Bone *bone = nullptr;
		bone = RiggedMesh_FindBoneFromName(b->children[k].get(), name);
		if(bone) return bone;
	}

	return nullptr;
}

static int doesBoneMove(Bone *b, Animation *anim){
	int first = -1;
	for(int k = 0; k < (int)anim->keyframes.size(); k++){
		if(anim->keyframes[k].boneName == b->name){
			if(first == -1) first = k;
			else return 1;
		}
	}
	return 0;
}

static void GetFramePositions(Bone *b, Animation *anim, int *lastRotFrame, int *nextRotFrame, float timeInto){
	int k;
	for(k = 0; k < (int)anim->keyframes.size(); k++){
		if(anim->keyframes[k].boneName == b->name){
			if(anim->keyframes[k].pos*(1000/FRAME_RATE) <= timeInto){
				*lastRotFrame = k;
			} else {
				*nextRotFrame = k;
				break;
			}
		}
	}
}

// static Quaternion UpdateBoneDamping(Bone *b){

// 	const float springConst = 0.0025;
// 	const float dampingConst = 0.001;

// 	int dt = Window_GetTicks() - b->lastDampingTime;

// 	if(fabs(b->dampingVel) < 0.00000001 || isnan(b->dampingVel)){
// 		b->lastDampingTime = -1;
// 		return b->finalRotDamping;
// 	}

// 	b->dampingVel += (-(springConst*b->dampingX) - (dampingConst*b->dampingVel)) * dt;
// 	b->dampingX += b->dampingVel;

// 	b->lastDampingTime = Window_GetTicks();

// 	return Math_Slerp(b->startingRotDamping, b->finalRotDamping, 1 + b->dampingX);
// }

static Quaternion getCurrentBoneRotation(Bone *b, Animation *anim, PlayingAnimation *pAnim, int *frameChange, int *doSlerp){

	if(!b) return Quaternion();

	int timeInto = Window_GetTicks() - pAnim->timeStarted;
	int lastRotFrame = -1, nextRotFrame = -1;
	GetFramePositions(b, anim, &lastRotFrame, &nextRotFrame, timeInto);

	*doSlerp = 0;
	*frameChange = 1;

	if(lastRotFrame == -1){
		if(nextRotFrame == -1) return b->rot;
		return anim->keyframes[nextRotFrame].rotation;
	}

	int maxPos = 0;
	for(int k = 0; k < (int)anim->keyframes.size(); k++)
		if(anim->keyframes[k].pos > maxPos)
			maxPos = anim->keyframes[k].pos;

	if(anim->keyframes[lastRotFrame].pos >= maxPos){

		if(!pAnim->loop) {

			pAnim->ended = true;

		} else {

			pAnim->timeStarted = Window_GetTicks();
			nextRotFrame = lastRotFrame = -1;
			timeInto = *doSlerp = 0;
			GetFramePositions(b, anim, &lastRotFrame, &nextRotFrame, timeInto);
		}
	}

	if(lastRotFrame == -1){
		if(nextRotFrame == -1) return b->rot;
		return anim->keyframes[nextRotFrame].rotation;
	}

	if(nextRotFrame == -1){

		return anim->keyframes[lastRotFrame].rotation;
	}

	float animTime = abs(1000/FRAME_RATE)*(anim->keyframes[nextRotFrame].pos - anim->keyframes[lastRotFrame].pos);
	float t = fabs((float)timeInto - ((1000/FRAME_RATE)*anim->keyframes[lastRotFrame].pos)) / animTime;

	if(t > 1) t = 1;
	if(t < 0) t = 0;

	Quaternion qa = anim->keyframes[lastRotFrame].rotation;
	Quaternion qb = anim->keyframes[nextRotFrame].rotation;

	// if(b->finalRotDamping != qa){
	// 	b->startingRotDamping = b->finalRotDamping;
	// 	b->finalRotDamping = qa;
	// 	b->lastDampingTime = Window_GetTicks();

	//     float cosHalfTheta = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y + qa.z*qb.z;
	//     if(cosHalfTheta < 0) cosHalfTheta *= -1;

	//     float theta = (acos(cosHalfTheta) * 2.0);

	//     b->dampingVel = (animTime / theta);

	//     b->dampingX = 0;
	// }

	*frameChange = *doSlerp = 1;

	return Math_Slerp(qa, qb, t);
}

static Vector3 getCurrentBoneScale(Bone *b, Animation *anim, PlayingAnimation *pAnim){

	if(!b) return Vector3();

	int timeInto = Window_GetTicks() - pAnim->timeStarted;
	int lastScaleFrame = -1, nextScaleFrame = -1;
	GetFramePositions(b, anim, &lastScaleFrame, &nextScaleFrame, timeInto);

	if(lastScaleFrame == -1) return b->scale;
	if(nextScaleFrame == -1) return anim->keyframes[lastScaleFrame].scale;

	int maxPos = 0;
	for(int k = 0; k < (int)anim->keyframes.size(); k++)
		if(anim->keyframes[k].pos > maxPos)
			maxPos = anim->keyframes[k].pos;

	if(anim->keyframes[lastScaleFrame].pos >= maxPos){
		nextScaleFrame = 0;
		pAnim->timeStarted = Window_GetTicks();
		timeInto = 0;
		GetFramePositions(b, anim, &lastScaleFrame, &nextScaleFrame, timeInto);
	}

	float animTime = abs(1000/FRAME_RATE)*(anim->keyframes[nextScaleFrame].pos - anim->keyframes[lastScaleFrame].pos);
	float t = fabs((float)timeInto - ((1000/FRAME_RATE)*anim->keyframes[lastScaleFrame].pos)) / animTime;

	if(t > 1) t = 1;
	if(t < 0) t = 0;

	return Math_Lerp(anim->keyframes[lastScaleFrame].scale, anim->keyframes[nextScaleFrame].scale, t);
}

static Vector3 getCurrentBonePos(Bone *b, Animation *anim, PlayingAnimation *pAnim){

	if(!b) return Vector3();

	int timeInto = Window_GetTicks() - pAnim->timeStarted;
	int lastPosFrame = -1, nextPosFrame = -1;
	GetFramePositions(b, anim, &lastPosFrame, &nextPosFrame, timeInto);

	if(lastPosFrame == -1) return b->pos;
	if(nextPosFrame == -1) return anim->keyframes[lastPosFrame].position;

	int maxPos = 0;
	for(int k = 0; k < (int)anim->keyframes.size(); k++)
		if(anim->keyframes[k].pos > maxPos)
			maxPos = anim->keyframes[k].pos;

	if(anim->keyframes[lastPosFrame].pos >= maxPos){
		nextPosFrame = 0;
		pAnim->timeStarted = Window_GetTicks();
		timeInto = 0;
		GetFramePositions(b, anim, &lastPosFrame, &nextPosFrame, timeInto);
	}

	float animTime=abs(1000/FRAME_RATE)*(anim->keyframes[nextPosFrame].pos - anim->keyframes[lastPosFrame].pos);
	float t = fabs((float)timeInto - ((1000/FRAME_RATE)*anim->keyframes[lastPosFrame].pos)) / animTime;

	if(t > 1) t = 1;
	if(t < 0) t = 0;

	return Math_Lerp(anim->keyframes[lastPosFrame].position, anim->keyframes[nextPosFrame].position, t);
}

static int BoneUpdate(Bone *b, std::map<std::string, Animation> &anims,
	std::map<std::string, PlayingAnimation> &pAnims, std::vector<Matrix4> &shaderMatricies){

	if(!b) return 1;

	// bool rotSet = false;
	Quaternion rot = b->rot;

	if(b->startSlerpTime != -1){

		float t = (Window_GetTicks() - b->startSlerpTime) * b->slerpSpeed;

		if(t > 1.0)
			b->startSlerpTime = -1;
		else
			rot = Math_Slerp(b->currRot, b->rot, t);
	}

	Vector3 scale = b->scale;
	Vector3 pos = b->pos;

	for(std::map<std::string, PlayingAnimation>::iterator it = pAnims.begin(); it != pAnims.end(); ++it){

		// if(!anims[it->first].keyframes.size() || !doesBoneMove(b, &anims[it->first])) continue;
		if(!anims[it->first].keyframes.size()) continue;

		int rotTempFrameChange = 0, doSlerp = 0;

		rot *= getCurrentBoneRotation(b, &anims[it->first], &pAnims[it->first], &rotTempFrameChange, &doSlerp);

		// scale = getCurrentBoneScale(b, &anims[it->first], &pAnims[it->first]);
		// pos = getCurrentBonePos(b, &anims[it->first], &pAnims[it->first]);

		// if(rotTempFrameChange) {

		// 	if(doSlerp && rotSet)
		// 		rot = Math_Slerp(rot, rot2, it->second.weight);
		// 	else
		// 		rot = rot2;

		// 	rotSet = true;
		// }
	}

	float rotMatrix[16];
	float finalMatrix[16];
	float transMatrix[16] = {
		scale.x,0,0,pos.x,
		0,scale.y,0,pos.y,
		0,0,scale.z,pos.z,
		0,0,0,1
	};

	if(b->overrideRotation){
		Math_CopyMatrix(rotMatrix ,b->rotMatrix);
	} else {
		Math_MatrixFromQuat(rot, rotMatrix);
		b->currRot = rot;
	}

	Math_MatrixMatrixMult(b->matrix, transMatrix, rotMatrix);

	if(b->parent){
		Math_MatrixMatrixMult(b->absMatrix,  b->parent->absMatrix, b->matrix);
		Math_MatrixMatrixMult(b->absRotationMatrix,  b->parent->absRotationMatrix, rotMatrix);
	} else {
		Math_CopyMatrix(b->absMatrix, b->matrix);
		Math_CopyMatrix(b->absRotationMatrix, rotMatrix);
	}

	if(b->parent && b->overrideRotation){

		float invRot[16];
		Math_CopyMatrix(invRot, b->absRotationMatrix);
		Math_InverseMatrix(invRot);
		Math_MatrixMatrixMult(b->absMatrix, b->absMatrix, invRot);
		Math_MatrixMatrixMult(b->absMatrix, b->absMatrix, rotMatrix);
		Math_CopyMatrix(b->absRotationMatrix, rotMatrix);
	}

	Math_MatrixMatrixMult(finalMatrix, b->absMatrix, b->invBindMatrix);

	Math_CopyMatrix(&shaderMatricies[b->index].m[0], finalMatrix);

	for(int j = 0; j < (int)b->children.size(); j++)
		BoneUpdate(b->children[j].get(), anims, pAnims, shaderMatricies);

	return 0;
}

void RiggedMesh::LoadMesh(const std::string &meshPath){
    if(!this->skeletonLoaded) { printf("Skeleton not yet loaded for %s.\n", meshPath.c_str()); return; }

	this->mesh.LoadObjFile(meshPath.c_str(), 1, 1, 1, this);
	this->mesh.InitializeDrawing(SKELETAL_ANIMATION_SHADER);
}

void RiggedMesh::LoadAnim(const std::string &animPath, const std::string &name){
	std::vector<Keyframe> *keyframes = &this->animations[name].keyframes;
	this->animations[name].name = name;

	FILE *fp;
	fopen_s(&fp, animPath.c_str(), "rb");
    if(!fp) { printf("Error loading animation: %s\n", animPath.c_str()); return; }


    while(!feof(fp)){

    	char type;
    	fscanf_s(fp, "%c ", &type);

    	if(type == 'a'){

	    	Keyframe tmp;

	    	char boneName[64];

	    	fscanf_s(fp, "%i ", &tmp.pos);
	    	fscanf_s(fp, "%s ", boneName);
	    	fscanf_s(fp, "%f ", &tmp.position.x);
	    	fscanf_s(fp, "%f ", &tmp.position.y);
	    	fscanf_s(fp, "%f ", &tmp.position.z);
	    	fscanf_s(fp, "%f ", &tmp.rotation.x);
	    	fscanf_s(fp, "%f ", &tmp.rotation.y);
	    	fscanf_s(fp, "%f ", &tmp.rotation.z);
	    	fscanf_s(fp, "%f ", &tmp.rotation.w);
	    	fscanf_s(fp, "%f ", &tmp.scale.x);
	    	fscanf_s(fp, "%f ", &tmp.scale.y);
	    	fscanf_s(fp, "%f ", &tmp.scale.z);

	    	tmp.boneName = boneName;

	    	keyframes->push_back(tmp);
    	}

    	else if(type == 't'){

    		int index;
	    	char path[512];
			fscanf_s(fp, "%i", &index);
			fscanf_s(fp, "%s\n", path);

		    int k;
		    for(k = (int)animPath.size(); k > 0; k --)
		        if(animPath.c_str()[k] == '/') break;

		    char fullPath[512] = {};

		    memcpy(fullPath, animPath.c_str(), k+1);
		    strcat_s(fullPath, path);

		    Material mat;

		    mat.texIndex = index;

		    mat.diffuseMapImage.LoadPNG(std::string(fullPath), false);
		    mat.diffuseMapImage.SetParameters(GL_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT);

    		this->animations[name].overrideMaterials.push_back(mat);
    	}
    }

    fclose(fp);
}

int RiggedMesh::GetBoneIndexFromName(const std::string &name){
	Bone *b = RiggedMesh_FindBoneFromName(this->rootBone.get(), name);
	if(b == nullptr) return -1;
	return b->index;
}

Bone *RiggedMesh::GetBoneFromName(const std::string &name){
	return RiggedMesh_FindBoneFromName(this->rootBone.get(), name);
}

int RiggedMeshInstance::GetBoneIndexFromName(const std::string &name){
	Bone *b = RiggedMesh_FindBoneFromName(this->rootBone.get(), name);
	if(b == nullptr) return -1;
	return b->index;
}

Bone *RiggedMeshInstance::GetBoneFromName(const std::string &name){
	return RiggedMesh_FindBoneFromName(this->rootBone.get(), name);
}

void RiggedMeshInstance::Draw(){
	if(!playingAnims.size() || !shaderMatricies.size()) return;
	glUniformMatrix4fv(Shaders_GetBonesLocation(), this->shaderMatricies.size(), GL_TRUE, &this->shaderMatricies[0].m[0]);
	Shaders_SetModelMatrix(&GetModelMatrix().m[0]);
	Shaders_UpdateModelMatrix();
	rMesh->Draw(overrideMaterialAnim);
}

void RiggedMesh::AnimateMatricies(std::map<std::string, PlayingAnimation> &playingAnims,
	std::vector<Matrix4> &shaderMatricies, std::shared_ptr<Bone> root){

	BoneUpdate(root.get(), animations, playingAnims, shaderMatricies);
}

int RiggedMeshInstance::Update(){
	if(!this->shaderMatricies.size() || !this->playingAnims.size()) return 1;
	rMesh->AnimateMatricies(playingAnims, shaderMatricies, rootBone);
	return 0;
}

static void DrawBone(Bone *b, std::vector<Vector3> &lines){

	float matrix[16];
	Math_MatrixMatrixMult(matrix, b->absMatrix, b->invBindMatrix);

	lines.push_back(Math_MatrixMult({0,0,0}, matrix));

	if(b->children.size()){
		Math_MatrixMatrixMult(matrix, b->children[0]->absMatrix, b->children[0]->invBindMatrix);
		lines.push_back(Math_MatrixMult({0,0,0}, matrix));
	} else {

		lines.push_back(Math_MatrixMult({0,1,0}, matrix));
	}

	for(int j = 0; j < (int)b->children.size(); j++)
		DrawBone(b->children[j].get(), lines);
}

void RiggedMeshInstance::GetSkeletonLines(std::vector<Vector3> &lines){
	DrawBone(rootBone.get(), lines);
}

RiggedMeshInstance::RiggedMeshInstance(RiggedMesh *m):
	rMesh (m),
	shaderMatricies(),
	playingAnims()
{
	bb = m->GetBoundingBox();
	shaderMatricies.resize(m->GetNumOfBones());
	Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);
	this->rootBone = rMesh->CopyBones();
}

void RiggedMeshInstance::StartAnimation(const std::string &name, float weight, bool loop){
	playingAnims.insert(std::pair<std::string, PlayingAnimation>(name, PlayingAnimation()));
	playingAnims[name].timeStarted = Window_GetTicks();
	playingAnims[name].weight = weight;
	playingAnims[name].loop = loop;
	playingAnims[name].ended = false;

}

void RiggedMeshInstance::StopAnimation(const std::string &name){
	if(playingAnims.find(name) != playingAnims.end())
		playingAnims.erase(playingAnims.find(name));
}

void RiggedMeshInstance::StopAllAnimations(){
	playingAnims.clear();
}

void RiggedMesh::GetBonesStartingPositions(Bone *b){

	if(!b) return;

	float rotMatrix[16] = {0};
	float finalMatrix[16] = {0};
	float transMatrix[16] = {
		b->scale.x,0,0,b->pos.x,
		0,b->scale.y,0,b->pos.y,
		0,0,b->scale.z,b->pos.z,
		0,0,0,1
	};

	Math_MatrixFromQuat(b->startingRot, rotMatrix);

	Math_MatrixMatrixMult(b->matrix, transMatrix, rotMatrix);

	if(b->parent)
		Math_MatrixMatrixMult(b->absMatrix,  b->parent->absMatrix, b->matrix);
	else
		Math_CopyMatrix(b->absMatrix, b->matrix);

	Math_MatrixMatrixMult(finalMatrix, b->absMatrix, b->invBindMatrix);

	Vector3 point = {0,0,0};
	b->startingPos = Math_MatrixMult(point, b->absMatrix);

	for(int j = 0; j < (int)b->children.size(); j++)
		GetBonesStartingPositions(b->children[j].get());
}

static std::shared_ptr<Bone> CopyBone(std::shared_ptr<Bone> bone, std::shared_ptr<Bone> parent){

	std::shared_ptr<Bone> ret = std::shared_ptr<Bone>(new Bone(*bone));
	ret->parent = parent.get();

	ret->children.clear();

	for(int k = 0; k < (int)bone->children.size(); k++)
		ret->children.push_back(CopyBone(bone->children[k], ret));

	return ret;
}

std::shared_ptr<Bone> RiggedMesh::CopyBones(){

	return CopyBone(rootBone, nullptr);
}

void RiggedMesh::LoadSkel(const std::string &skelPath){

	FILE *fp;
	fopen_s(&fp, skelPath.c_str(), "r");
	if(!fp) { printf("Error loading skeleton %s\n", skelPath.c_str()); return; }

	int index = 0;
	while(!feof(fp)){

		this->numOfBones++;

	    int parent_index;

	    std::shared_ptr<Bone> temp(new Bone());

	    char name[64];

		int res = fscanf_s(fp, "%s ",name);
		res += fscanf_s(fp, "%i ", &parent_index);
		res += fscanf_s(fp, "%f ", &temp->pos.x);
		res += fscanf_s(fp, "%f ", &temp->pos.y);
		res += fscanf_s(fp, "%f ", &temp->pos.z);
		res += fscanf_s(fp, "%f ", &temp->rot.x);
		res += fscanf_s(fp, "%f ", &temp->rot.y);
		res += fscanf_s(fp, "%f ", &temp->rot.z);
		res += fscanf_s(fp, "%f ", &temp->rot.w);
		res += fscanf_s(fp, "%f ", &temp->scale.x);
		res += fscanf_s(fp, "%f ", &temp->scale.y);
		res += fscanf_s(fp, "%f\n", &temp->scale.z);

		temp->startingRot = temp->rot;
		temp->name = name;

		if(res){

	    	temp->index = index;

	    	if(parent_index == -1){

	    		this->rootBone = temp;

	    	} else {

	    		if(this->rootBone != nullptr){

			    	Bone *parent = nullptr;
			    	parent = RiggedMesh_FindBoneFromIndex(this->rootBone.get(), parent_index);
			    	if(parent){
				    	temp->parent = parent;
				    	parent->children.push_back(temp);
			    	}
			    }
	    	}

	    	index++;

	    } else {

			while(fgetc(fp) != '\n' && !feof(fp)){}
			continue;
	    }

		float transMatrix[16] = {
			temp->scale.x,0,0,temp->pos.x,
			0,temp->scale.y,0,temp->pos.y,
			0,0,temp->scale.z,temp->pos.z,
			0,0,0,1
		};

		float rotMatrix[16];
		Math_MatrixFromQuat(temp->rot, rotMatrix );
		Math_MatrixMatrixMult(temp->invBindMatrix, transMatrix, rotMatrix);

		if(temp->parent){
			Math_MatrixMatrixMult(temp->absMatrix,  temp->parent->absMatrix, temp->invBindMatrix);
			temp->startingAbsRot = temp->parent->startingAbsRot * Math_MatrixToQuat(rotMatrix);
		} else {
			Math_CopyMatrix(temp->absMatrix, temp->invBindMatrix);
			temp->startingAbsRot = Math_MatrixToQuat(rotMatrix);
		}

		Math_CopyMatrix(temp->invBindMatrix, temp->absMatrix);
		Math_InverseMatrix(temp->invBindMatrix);
	}

	fclose(fp);
	skeletonLoaded = true;

	GetBonesStartingPositions(rootBone.get());
}

void RiggedMesh::Draw(const std::string &overrideMaterialAnimation){
	if(animations.find(overrideMaterialAnimation) != animations.end())
		this->mesh.Draw(&animations[overrideMaterialAnimation].overrideMaterials);
	else
		this->mesh.Draw(nullptr);
}
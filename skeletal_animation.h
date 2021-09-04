#pragma once
#include "ymath.h"
#include <vector>
#include <map>
#include "vao_vbo.h"
#include "shaders.h"
#include "image_loader.h"
#include "mesh.h"

class Bone {
public:
	Bone():
		parent(nullptr),
		index(-1),
		isHidden(false),
		overrideRotation(false),
		dampingVel(0),
		dampingX(0),
		lastDampingTime(0),
		slerpSpeed(-1),
		startSlerpTime(-1)
		{}

	Vector3 pos;
	Vector3 startingPos;
	Quaternion rot;
	Quaternion startingRot;
	Quaternion currRot;
	Vector3 scale;
	std::string name;
	Bone *parent;
	std::vector<std::shared_ptr<Bone>> children;
	float invBindMatrix[16];
	float matrix[16];
	float absMatrix[16];
	float absRotationMatrix[16];
	Quaternion startingAbsRot;
	float rotMatrix[16];
	int index;
	char isHidden;
	bool overrideRotation;
	float dampingVel;
	float dampingX;
	float lastDampingTime;
	Quaternion finalRotDamping;
	Quaternion startingRotDamping;
	float slerpSpeed;
	int startSlerpTime;
};

typedef struct {
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	int pos;
	std::string boneName;
} Keyframe;

typedef struct {
	std::vector<Keyframe> keyframes;
	std::string name;
	std::vector<Material> overrideMaterials;
} Animation;

class PlayingAnimation {
public:
	PlayingAnimation():timeStarted(0), weight(0), loop(false), ended(false){}
	int timeStarted;
	float weight;
	bool loop;
	bool ended;

};


class RiggedMesh {

public:
	RiggedMesh() :
		numOfBones(0),
		skeletonLoaded(false)
	{}

	void LoadMesh(const std::string &path);
	void LoadSkel(const std::string &path);
	void LoadAnim(const std::string &path, const std::string &name);
	void Draw(const std::string &overrideMaterialAnimation = "");
	int GetTexture(){return img.GetTexture();}
	int GetBoneIndexFromName(const std::string &name);
	Bone *GetBoneFromName(const std::string &name);
	int GetNumOfBones() const { return numOfBones; }
	BoundingBox GetBoundingBox() { return mesh.GetBoundingBox(); }
	void AnimateMatricies(std::map<std::string, PlayingAnimation> &pa, std::vector<Matrix4> &sm, std::shared_ptr<Bone> r);
	std::shared_ptr<Bone> CopyBones();
	std::shared_ptr<Bone> rootBone;
private:
	void GetBonesStartingPositions(Bone *b);
	Mesh mesh;
	Image img;
	int numOfBones;
	std::map<std::string, Animation> animations;
	bool skeletonLoaded;
};

class RiggedMeshInstance : public Object {
public:
	RiggedMeshInstance() : rMesh(0) {}
	RiggedMeshInstance(RiggedMesh *rm);
	int GetBoneIndexFromName(const std::string &name);
	Bone *GetBoneFromName(const std::string &name);
	int GetNumOfBones() const { return rMesh->GetNumOfBones(); }
	void Draw();
	void GetSkeletonLines(std::vector<Vector3> &lines);
	int GetShader() { return SKELETAL_ANIMATION_SHADER; }
	int GetTexture() { return rMesh->GetTexture(); }
    int CastsShadow(){ return 1; }
	int  Update();
	bool AnimationEnded(const std::string &name){
		if(playingAnims.find(name) != playingAnims.end())
			return playingAnims[name].ended;
		return false;
	}
	void SetWeight(const std::string &name, float weight){
		if(playingAnims.find(name) != playingAnims.end())
			playingAnims[name].weight = weight;
	}
	void StartAnimation(const std::string &name, float weight, bool loop = false);
	void StopAnimation(const std::string &name);
	void StopAllAnimations();
	void SetOverrideMaterialAnim(const std::string &anim){ overrideMaterialAnim = anim; }
private:
	RiggedMesh *rMesh;
	std::vector<Matrix4> shaderMatricies;
	std::map<std::string, PlayingAnimation> playingAnims;
	std::string overrideMaterialAnim;
	std::shared_ptr<Bone> rootBone;
};
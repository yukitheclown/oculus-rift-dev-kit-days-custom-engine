#ifndef WAIFU_DEF
#define WAIFU_DEF

#include "skeletal_animation.h"
#include "math.h"
#include "oculus.h"
#include <string>

class LookAtMotion {
public:
	LookAtMotion(const Vector3 &u, const Quaternion &sRot, const Quaternion &fRot, Bone *b, float dmp, float spr, int lTime = -1):
		up(u),
		startingRot(sRot),
		finalRot(fRot),
		currRot(sRot),
		bone(b),
		dampingConst(dmp),
		springConst(spr),
		dampingX(0),
		dampingVelInitial(0),
		dampingVel(0),
		lastTime(lTime),
		timeInto(0),
		theta(0),
		returnAfterReached(false){
			Reset();
		}

	Vector3 up;
	int Update();
	void Reset();
	Quaternion startingRot;
	Quaternion finalRot;
	Quaternion currRot;
	Bone *bone;
	float dampingConst;
	float springConst;
	float dampingX;
	float dampingVelInitial;
	float dampingVel;
	int lastTime;
	float timeInto;
	float theta;
	bool returnAfterReached;
private:
	LookAtMotion();
};

class Waifu {
public:
	Waifu():
		eyeCloseDistance(0.5),
		isBeingLookedAt(true),
		lookingAtPlayerStartTime(0),
		sleeping(false),
		lookingAtPlayer(false),
		nextSleepTime(0),
		nextLookAtTime(0),
		sleepPlayCount(0){}

	virtual ~Waifu(){}
	Vector3 GetBonePos(const std::string &boneName);
	void UpdateIK();

	void LookAt(const std::string &boneName, Quaternion &qa, Quaternion &qb, Vector3 up, float dmp, float spr, Quaternion &msrq, float ma);
	void LookAt(const std::string &boneName, const Vector3 &target, Vector3 up, float d, float s, Quaternion &msrq, float ma);
	virtual void Init(OculusHandler *oHandler) = 0;
	virtual void Update() = 0;
	virtual void Draw(const Vector3 &lightDir, bool useShadows) = 0;
	void PlayAnimation(const std::string &name, bool loop = false);
	void CheckIfBeingLookedAt();
	void LookAtPlayer();
	LookAtMotion *GetLookAt(const std::string &boneName);
	void SetNextSleepTime();
	void LookAwayFromPlayer();
	void RemoveLookAt(const std::string &boneName, float speed = 0.00005);
	float eyeCloseDistance;
	Vector3 waifuPos;
	Vector3 eulerAngles;
	Vector4 color;
	OculusHandler *oHandler;
	RiggedMeshInstance meshInstance;
	bool isBeingLookedAt;
protected:
	void UpdateLookAts();
	RiggedMesh mesh;
	std::vector<LookAtMotion> lookAts;
	std::string playingAnim;
	int lookingAtPlayerStartTime;
	bool sleeping;
	bool lookingAtPlayer;
	int nextSleepTime;
	int nextLookAtTime;
	int sleepPlayCount;
};

class NeptuneWaifu : public Waifu {
public:
	NeptuneWaifu(){}
	void Init(OculusHandler *oHandler);
	void Update();
	void Draw(const Vector3 &lightDir, bool useShadows);
private:
};


#endif